#include "distortion.h"

#include <algorithm>
#include <map>

#include "morse.h"

namespace morse {

namespace {

// Убирает дубликаты (оставляя минимальную цену), сортирует по цене, урезает до beam.
void finalize(StepResult& result, std::size_t beam) {
    std::map<std::string, int> best;
    for (const Candidate& c : result.candidates) {
        auto it = best.find(c.code);
        if (it == best.end()) best.emplace(c.code, c.cost);
        else it->second = std::min(it->second, c.cost);
    }
    result.candidates.clear();
    for (const auto& [code, cost] : best) result.candidates.push_back({code, cost});
    std::stable_sort(result.candidates.begin(), result.candidates.end(),
                     [](const Candidate& a, const Candidate& b) { return a.cost < b.cost; });
    if (result.candidates.size() > beam) {
        result.candidates.resize(beam);
        result.truncated = true;
    }
}

// ---------- Паника ----------

// Все токены-прообразы для токена t: сам t (цена 0), если он допустим, и t с одной
// заменой '*' -> '-' в позициях 1.. (цена 1), если результат допустим.
std::vector<std::pair<std::string, int>> panicPreimages(const std::string& token,
                                                         const TokenPredicate& language) {
    std::vector<std::pair<std::string, int>> result;
    if (language(token)) result.emplace_back(token, 0);
    for (std::size_t i = 1; i < token.size(); ++i) {
        if (token[i] != '*') continue;
        std::string original = token;
        original[i] = '-';
        if (language(original)) result.emplace_back(original, 1);
    }
    return result;
}

// ---------- Потеря связи ----------

// Все заполнения потери длины k: символы '*'/'-' и не более одного '/'.
// Количество: 2^k + k * 2^(k-1).
std::vector<std::string> makeFills(std::size_t k) {
    std::vector<std::string> fills;
    for (int slash = -1; slash < static_cast<int>(k); ++slash) {
        const std::size_t freeSlots = (slash < 0) ? k : k - 1;
        for (unsigned mask = 0; mask < (1u << freeSlots); ++mask) {
            std::string fill;
            std::size_t bit = 0;
            for (std::size_t i = 0; i < k; ++i) {
                if (static_cast<int>(i) == slash) fill += '/';
                else fill += ((mask >> bit++) & 1u) ? '-' : '*';
            }
            fills.push_back(fill);
        }
    }
    return fills;
}

const std::vector<std::string>& fillOptions(std::size_t k) {
    static const std::vector<std::vector<std::string>> cache = [] {
        std::vector<std::vector<std::string>> all(LossDistortion::kMaxLoss + 1);
        for (std::size_t k = 1; k <= LossDistortion::kMaxLoss; ++k) all[k] = makeFills(k);
        return all;
    }();
    return cache[k];
}

// Проверяет все ЗАКРЫТЫЕ (завершённые '/') токены строки начиная с позиции checked.
// Двигает checked вперёд. false - найден токен, невозможный в модели.
bool checkClosed(const std::string& s, std::size_t& checked, const TokenPredicate& language) {
    for (std::size_t i = checked; i < s.size(); ++i) {
        if (s[i] != '/') continue;
        if (!language(s.substr(checked, i - checked))) return false;
        checked = i + 1;
    }
    return true;
}

// Эвристика для урезания: сколько токенов не являются буквами (каждый потребует
// исправления паникой). Чем меньше, тем правдоподобнее.
std::size_t invalidTokens(const std::string& text) {
    std::size_t n = 0;
    for (const std::string& token : splitTokens(text))
        if (!token.empty() && !isLetterCode(token)) ++n;
    return n;
}

// Все способы заполнить '?' в одном потоке так, чтобы каждый токен был допустим.
std::vector<std::string> fillLosses(const std::string& code, const TokenPredicate& language,
                                    std::size_t beam, bool& truncated) {
    // 1. Разбор: parts - целые куски, runs - длины серий '?' между ними.
    std::vector<std::string> parts;
    std::vector<std::size_t> runs;
    std::string current;
    for (std::size_t i = 0; i < code.size();) {
        if (code[i] != '?') {
            current += code[i++];
            continue;
        }
        std::size_t j = i;
        while (j < code.size() && code[j] == '?') ++j;
        parts.push_back(current);
        current.clear();
        runs.push_back(j - i);
        i = j;
    }
    parts.push_back(current);

    // 2. Проверка ограничений модели.
    for (std::size_t len : runs)
        if (len > LossDistortion::kMaxLoss) return {};
    for (std::size_t i = 1; i + 1 < parts.size(); ++i)  // между потерями нужен '/'
        if (parts[i].find('/') == std::string::npos) return {};

    // 3. Поиск по сериям с отсечением (beam search).
    struct Partial {
        std::string text;
        std::size_t checked = 0;
        std::size_t key = 0;
    };
    std::vector<Partial> partials(1);
    partials[0].text = parts[0];
    if (!checkClosed(partials[0].text, partials[0].checked, language)) return {};

    for (std::size_t r = 0; r < runs.size(); ++r) {
        std::vector<Partial> next;
        for (const Partial& p : partials) {
            for (const std::string& fill : fillOptions(runs[r])) {
                Partial q{p.text + fill + parts[r + 1], p.checked, 0};
                if (checkClosed(q.text, q.checked, language)) next.push_back(std::move(q));
            }
        }
        partials = std::move(next);
        if (partials.empty()) return {};
        if (partials.size() > beam) {
            for (Partial& p : partials) p.key = invalidTokens(p.text);
            std::stable_sort(partials.begin(), partials.end(),
                             [](const Partial& a, const Partial& b) { return a.key < b.key; });
            partials.resize(beam);
            truncated = true;
        }
    }

    // 4. Хвост без завершающего '/' тоже должен быть допустимым токеном.
    std::vector<std::string> result;
    for (const Partial& p : partials) {
        const std::string tail = p.text.substr(p.checked);
        if (!tail.empty() && !language(tail)) continue;
        result.push_back(joinTokens(splitTokens(p.text)));
    }
    return result;
}

}  // namespace

// ================= PanicDistortion =================

std::string PanicDistortion::name() const { return "Радист в панике"; }

std::string PanicDistortion::apply(const std::string& code, std::mt19937& rng) const {
    std::uniform_real_distribution<double> chance(0.0, 1.0);
    std::string out = code;  // меняем символы на месте: структура потока не должна меняться
    std::size_t start = 0;   // начало текущего токена
    for (std::size_t i = 0; i <= out.size(); ++i) {
        if (i < out.size() && out[i] != '/') continue;
        const std::size_t length = i - start;  // токен занимает [start, i)
        if (length >= 2 && chance(rng) < p_) {
            std::vector<std::size_t> dashes;  // позиции '-' кроме первой
            for (std::size_t j = start + 1; j < i; ++j)
                if (out[j] == '-') dashes.push_back(j);
            if (!dashes.empty()) {
                std::uniform_int_distribution<std::size_t> pick(0, dashes.size() - 1);
                out[dashes[pick(rng)]] = '*';  // одна ошибка на букву
            }
        }
        start = i + 1;
    }
    return out;
}

TokenPredicate PanicDistortion::widen(const TokenPredicate& in) const {
    return [in](const std::string& token) { return !panicPreimages(token, in).empty(); };
}

StepResult PanicDistortion::restore(const std::vector<Candidate>& in,
                                    const TokenPredicate& languageIn, std::size_t beam) const {
    StepResult result;
    for (const Candidate& cand : in) {
        std::vector<Candidate> partial{{"", cand.cost}};
        bool feasible = true;
        const std::vector<std::string> tokens = splitTokens(cand.code);
        // Если поток не заканчивался на '/', лишний '/' в конец не дописываем.
        const bool terminated = cand.code.empty() || cand.code.back() == '/';
        for (std::size_t k = 0; k < tokens.size(); ++k) {
            const std::string& token = tokens[k];
            const bool isLast = (k + 1 == tokens.size());
            const char* suffix = (isLast && !terminated) ? "" : "/";
            const auto options = panicPreimages(token, languageIn);
            if (options.empty()) {  // токен не объясняется паникой - поток невозможен
                feasible = false;
                break;
            }
            std::vector<Candidate> next;
            next.reserve(partial.size() * options.size());
            for (const Candidate& p : partial)
                for (const auto& [original, extra] : options)
                    next.push_back({p.code + original + suffix, p.cost + extra});
            partial = std::move(next);
            if (partial.size() > beam) {
                std::stable_sort(partial.begin(), partial.end(),
                                 [](const Candidate& a, const Candidate& b) { return a.cost < b.cost; });
                partial.resize(beam);
                result.truncated = true;
            }
        }
        if (feasible)
            result.candidates.insert(result.candidates.end(), partial.begin(), partial.end());
    }
    finalize(result, beam);
    return result;
}

// ================= LossDistortion =================

std::string LossDistortion::name() const { return "Потеря связи"; }

std::string LossDistortion::apply(const std::string& code, std::mt19937& rng) const {
    std::uniform_real_distribution<double> chance(0.0, 1.0);
    std::uniform_int_distribution<std::size_t> lengthDist(1, kMaxLoss);
    std::string out = code;
    bool mayStart = true;  // после потери нужен хотя бы один целый '/'
    for (std::size_t i = 0; i < out.size();) {
        if (mayStart && chance(rng) < p_) {
            std::size_t len = std::min(lengthDist(rng), out.size() - i);
            while (len > 1 && std::count(out.begin() + i, out.begin() + i + len, '/') > 1) --len;
            std::fill(out.begin() + i, out.begin() + i + len, '?');
            i += len;
            mayStart = false;
        } else {
            if (out[i] == '/') mayStart = true;
            ++i;
        }
    }
    return out;
}

TokenPredicate LossDistortion::widen(const TokenPredicate& in) const {
    // После потери допустим либо обычный токен, либо токен с '?' (что именно потеряно,
    // заранее неизвестно: потеря может задеть и '/', поэтому проверка "мягкая").
    return [in](const std::string& token) {
        return token.find('?') != std::string::npos || in(token);
    };
}

StepResult LossDistortion::restore(const std::vector<Candidate>& in,
                                   const TokenPredicate& languageIn, std::size_t beam) const {
    StepResult result;
    for (const Candidate& cand : in) {
        for (std::string& code : fillLosses(cand.code, languageIn, beam, result.truncated))
            result.candidates.push_back({std::move(code), cand.cost});
    }
    finalize(result, beam);
    return result;
}

}  // namespace morse
