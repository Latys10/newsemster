#pragma once
// Цепочка нарушений. Хранит порядок наложения и сама разворачивает его при восстановлении:
// если наложили 1, затем 2, то отменяются 2, затем 1.

#include <cstddef>
#include <memory>
#include <random>
#include <string>
#include <vector>

#include "distortion.h"

namespace morse {

enum class ChannelMode {
    PanicOnly = 0,      // только вариант 3
    LossOnly = 1,       // только вариант 4
    PanicThenLoss = 2,  // сначала 3, затем 4 (восстановление: 4, затем 3)
    LossThenPanic = 3   // сначала 4, затем 3 (восстановление: 3, затем 4)
};

struct RestoreStepInfo {
    std::string name;          // какое нарушение отменялось на этой итерации
    std::size_t inputCount;    // кандидатов на входе итерации
    std::size_t outputCount;   // кандидатов на выходе
    bool truncated;            // список был урезан
};

struct RestoreReport {
    std::vector<Candidate> variants;      // итоговые варианты, лучшие первыми
    std::vector<RestoreStepInfo> steps;   // журнал итераций
};

class Pipeline {
public:
    static constexpr std::size_t kDefaultBeam = 3000;

    Pipeline(ChannelMode mode, double panicProbability, double lossProbability);

    // Накладывает нарушения в порядке цепочки.
    std::string transmit(const std::string& code, std::mt19937& rng) const;

    // Отменяет нарушения в обратном порядке.
    RestoreReport restore(const std::string& received, std::size_t beam = kDefaultBeam) const;

    std::string describe() const;  // "Паника -> Потеря связи; восстановление: ..."

private:
    std::vector<std::unique_ptr<Distortion>> chain_;  // в порядке наложения
};

}  // namespace morse
