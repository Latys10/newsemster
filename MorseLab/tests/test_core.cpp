// Консольные тесты ядра (без Qt).
// Сборка: g++ -std=c++17 -I. tests/test_core.cpp morse.cpp distortion.cpp pipeline.cpp -o test_core && ./test_core
#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <random>
#include <set>

#include "morse.h"
#include "pipeline.h"

using namespace morse;

static int failures = 0;
#define CHECK(cond)                                                              \
    do {                                                                         \
        if (!(cond)) {                                                           \
            std::cerr << "FAIL line " << __LINE__ << ": " #cond << "\n";         \
            ++failures;                                                          \
        }                                                                        \
    } while (0)

// u32string -> utf8 для вывода в консоль
static std::string utf8(const Text& t) {
    std::string out;
    for (char32_t c : t) {
        if (c < 0x80) out += static_cast<char>(c);
        else if (c < 0x800) { out += char(0xC0 | (c >> 6)); out += char(0x80 | (c & 0x3F)); }
        else { out += char(0xE0 | (c >> 12)); out += char(0x80 | ((c >> 6) & 0x3F)); out += char(0x80 | (c & 0x3F)); }
    }
    return out;
}

static bool hasText(const std::vector<Candidate>& v, const Text& t) {
    return std::any_of(v.begin(), v.end(), [&](const Candidate& c) { return decodeCode(c.code).text == t; });
}

int main() {
    // 1. Таблица
    CHECK(encodeTable().size() == 32);
    CHECK(decodeTable().size() == 32);  // коды уникальны

    // 2. Кодирование / декодирование
    CHECK(encodeText(U"ТОР") == "-/---/*-*/");
    CHECK(encodeText(U"тор тор") == "-/---/*-*//-/---/*-*/");
    CHECK(decodeCode(encodeText(U"Ёлка и пень")).text == U"ЕЛКА И ПЕНЬ");
    std::size_t skipped = 0;
    CHECK(encodeText(U"А1Б?", &skipped) == "*-/-***/" && skipped == 2);
    CHECK(decodeCode("--**/").text == U"З");
    CHECK(decodeCode("*-*-*-*-/").hasErrors);

    // 3. Пример паники из задания: Ч = ---*  ->  --**  (читается как З)
    {
        Pipeline p(ChannelMode::PanicOnly, 0, 0);
        RestoreReport r = p.restore("--**/");
        CHECK(hasText(r.variants, U"З"));
        CHECK(hasText(r.variants, U"Ч"));
        CHECK(r.variants.front().cost == 0);  // без поправок - первым
    }

    // 4. Пример потери из задания: Б = -***  ->  -??*  ; варианты МЕ, ТИ ...
    {
        Pipeline p(ChannelMode::LossOnly, 0, 0);
        RestoreReport r = p.restore("-??*");
        CHECK(hasText(r.variants, U"Б"));
        CHECK(hasText(r.variants, U"МЕ"));
        CHECK(hasText(r.variants, U"ТИ"));
        std::cout << "Пример потери '- ? ? *': вариантов " << r.variants.size() << ": ";
        for (auto& c : r.variants) std::cout << utf8(decodeCode(c.code).text) << ' ';
        std::cout << "\n";
        // модель: между потерями нужен '/', длина <= 5
        CHECK(p.restore("-?*?*").variants.empty());
        CHECK(p.restore(std::string(6, '?') + "/").variants.empty());
    }

    // 5. Ограничения генераторов нарушений
    {
        std::mt19937 rng(1);
        PanicDistortion panic(0.9);
        LossDistortion loss(0.3);
        for (int it = 0; it < 500; ++it) {
            std::u32string txt;
            for (int i = 0; i < 12; ++i) txt += char32_t(U'А' + rng() % 32);
            Code clean = encodeText(txt);
            // паника: <=1 изменения на букву, только '-'->'*', не в первой позиции
            Code bad = panic.apply(clean, rng);
            auto a = splitTokens(clean), b = splitTokens(bad);
            CHECK(a.size() == b.size());
            for (size_t i = 0; i < a.size(); ++i) {
                int diff = 0;
                for (size_t j = 0; j < a[i].size(); ++j)
                    if (a[i][j] != b[i][j]) { ++diff; CHECK(j > 0 && a[i][j] == '-' && b[i][j] == '*'); }
                CHECK(diff <= 1);
            }
            // потеря: длина серии 1..5, между сериями есть '/', длина потока сохраняется
            Code lost = loss.apply(clean, rng);
            CHECK(lost.size() == clean.size());
            size_t run = 0; bool slashSince = true;
            for (char ch : lost) {
                if (ch == '?') { ++run; CHECK(run <= 5); if (run == 1) CHECK(slashSince); slashSince = false; }
                else { run = 0; if (ch == '/') slashSince = true; }
            }
        }
    }

    // 6. Главное: две итерации. Паника -> Потеря, восстановление Потеря -> Паника.
    {
        std::mt19937 rng(2024);
        const int trials = 300;
        int found = 0, truncatedCnt = 0, twoSteps = 0;
        double avgVariants = 0, avgRank = 0;
        size_t maxVariants = 0;
        for (int t = 0; t < trials; ++t) {
            std::u32string txt;
            const int len = 4 + rng() % 5;  // 4..8 букв
            for (int i = 0; i < len; ++i) txt += char32_t(U'А' + rng() % 32);
            Pipeline p(ChannelMode::PanicThenLoss, 0.4, 0.06);
            const Code clean = encodeText(txt);
            const Code received = p.transmit(clean, rng);
            RestoreReport r = p.restore(received);
            if (r.steps.size() == 2) ++twoSteps;
            if (r.steps[0].name == "Потеря связи" && r.steps[1].name == "Радист в панике") {}
            else CHECK(false);
            if (r.steps[0].truncated || r.steps[1].truncated) ++truncatedCnt;
            auto it = std::find_if(r.variants.begin(), r.variants.end(), [&](const Candidate& c) { return c.code == clean; });
            if (it != r.variants.end()) {
                ++found;
                avgRank += (it - r.variants.begin()) + 1;
            } else if (!(r.steps[0].truncated || r.steps[1].truncated)) {
                CHECK(false);  // без усечения исходный код обязан быть среди вариантов
                std::cerr << "нет исходного: " << utf8(txt) << " принято " << received << "\n";
            }
            avgVariants += r.variants.size();
            maxVariants = std::max(maxVariants, r.variants.size());
        }
        CHECK(twoSteps == trials);
        std::cout << "Цепочка 3->4: тестов " << trials << ", две итерации во всех: " << (twoSteps == trials)
                  << ", исходный найден: " << found << ", усечений: " << truncatedCnt
                  << ", ср. число вариантов: " << avgVariants / trials
                  << ", макс.: " << maxVariants
                  << ", ср. место исходного: " << (found ? avgRank / found : 0) << "\n";
    }

    // 6б. Обратный порядок: Потеря -> Паника, восстановление Паника -> Потеря.
    {
        std::mt19937 rng(99);
        const int trials = 300;
        int found = 0, ok = 0, truncatedCnt = 0;
        for (int t = 0; t < trials; ++t) {
            std::u32string txt;
            const int len = 4 + rng() % 4;
            for (int i = 0; i < len; ++i) txt += char32_t(U'А' + rng() % 32);
            Pipeline p(ChannelMode::LossThenPanic, 0.3, 0.03);
            const Code clean = encodeText(txt);
            RestoreReport r = p.restore(p.transmit(clean, rng));
            CHECK(r.steps.size() == 2);
            if (r.steps.size() == 2 && r.steps[0].name == "Радист в панике" && r.steps[1].name == "Потеря связи") ++ok;
            const bool tr = r.steps[0].truncated || r.steps[1].truncated;
            truncatedCnt += tr;
            const bool has = std::any_of(r.variants.begin(), r.variants.end(), [&](const Candidate& c) { return c.code == clean; });
            found += has;
            if (!has && !tr) { CHECK(false); std::cerr << "нет исходного: " << utf8(txt) << "\n"; }
        }
        CHECK(ok == trials);
        std::cout << "Цепочка 4->3: порядок итераций верный в " << ok << "/" << trials
                  << ", исходный найден: " << found << ", усечений: " << truncatedCnt << "\n";
    }

    // 7. Демонстрация
    {
        std::mt19937 rng(7);
        Pipeline p(ChannelMode::PanicThenLoss, 0.5, 0.08);
        Code clean = encodeText(U"СВЯЗЬ");
        Code received = p.transmit(clean, rng);
        RestoreReport r = p.restore(received);
        std::cout << "Исходный: " << formatCode(clean) << "\nПринятый: " << formatCode(received) << "\n";
        for (auto& s : r.steps) std::cout << " итерация: " << s.name << " " << s.inputCount << " -> " << s.outputCount << "\n";
        for (size_t i = 0; i < std::min<size_t>(8, r.variants.size()); ++i)
            std::cout << "  #" << i + 1 << " " << utf8(decodeCode(r.variants[i].code).text) << " (правок " << r.variants[i].cost << ")\n";
    }

    std::cout << (failures ? "ЕСТЬ ОШИБКИ\n" : "ВСЕ ТЕСТЫ ПРОЙДЕНЫ\n");
    return failures ? 1 : 0;
}
