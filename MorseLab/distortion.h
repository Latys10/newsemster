#pragma once
// Нарушения канала связи и алгоритмы их отмены.
//   Вариант 3 - PanicDistortion: "радист в панике" ('-' превращается в '*').
//   Вариант 4 - LossDistortion:  "потеря связи" (символы заменяются на '?').

#include <cstddef>
#include <functional>
#include <random>
#include <string>
#include <vector>

namespace morse {

// Один вариант восстановленного потока и его "цена" (число сделанных поправок).
struct Candidate {
    std::string code;
    int cost = 0;
};

// Предикат "токен допустим": какие токены может содержать поток на данном этапе.
using TokenPredicate = std::function<bool(const std::string&)>;

struct StepResult {
    std::vector<Candidate> candidates;
    bool truncated = false;  // список урезан ограничением beam
};

class Distortion {
public:
    virtual ~Distortion() = default;

    virtual std::string name() const = 0;

    // Накладывает нарушение на поток.
    virtual std::string apply(const std::string& code, std::mt19937& rng) const = 0;

    // Какие токены возможны ПОСЛЕ этого нарушения, если до него допустимыми были токены `in`.
    virtual TokenPredicate widen(const TokenPredicate& in) const = 0;

    // Отменяет нарушение: из набора кандидатов делает набор кандидатов "до нарушения".
    // languageIn - допустимые токены на входе нарушения (то, что должно получиться).
    virtual StepResult restore(const std::vector<Candidate>& in,
                               const TokenPredicate& languageIn,
                               std::size_t beam) const = 0;
};

// Вариант 3. Не больше одной ошибки на букву, позиция любая, кроме первой.
// Ошибка: длинный сигнал '-' превращается в короткий '*'.
class PanicDistortion : public Distortion {
public:
    explicit PanicDistortion(double probability) : p_(probability) {}
    std::string name() const override;
    std::string apply(const std::string& code, std::mt19937& rng) const override;
    TokenPredicate widen(const TokenPredicate& in) const override;
    StepResult restore(const std::vector<Candidate>& in, const TokenPredicate& languageIn,
                       std::size_t beam) const override;

private:
    double p_;  // вероятность ошибки в каждой букве
};

// Вариант 4. Потеря 1..5 подряд идущих символов (они заменяются на '?'),
// внутри потери не больше одного '/', между потерями есть хотя бы один '/'.
class LossDistortion : public Distortion {
public:
    static constexpr std::size_t kMaxLoss = 5;

    explicit LossDistortion(double probability) : p_(probability) {}
    std::string name() const override;
    std::string apply(const std::string& code, std::mt19937& rng) const override;
    TokenPredicate widen(const TokenPredicate& in) const override;
    StepResult restore(const std::vector<Candidate>& in, const TokenPredicate& languageIn,
                       std::size_t beam) const override;

private:
    double p_;  // вероятность начала потери в каждой позиции
};

}  // namespace morse
