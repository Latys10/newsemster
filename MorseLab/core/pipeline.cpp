#include "pipeline.h"

#include "morse.h"

namespace morse {

Pipeline::Pipeline(ChannelMode mode, double panicProbability, double lossProbability) {
    switch (mode) {
    case ChannelMode::PanicOnly:
        chain_.push_back(std::make_unique<PanicDistortion>(panicProbability));
        break;
    case ChannelMode::LossOnly:
        chain_.push_back(std::make_unique<LossDistortion>(lossProbability));
        break;
    case ChannelMode::PanicThenLoss:
        chain_.push_back(std::make_unique<PanicDistortion>(panicProbability));
        chain_.push_back(std::make_unique<LossDistortion>(lossProbability));
        break;
    case ChannelMode::LossThenPanic:
        chain_.push_back(std::make_unique<LossDistortion>(lossProbability));
        chain_.push_back(std::make_unique<PanicDistortion>(panicProbability));
        break;
    }
}

std::string Pipeline::transmit(const std::string& code, std::mt19937& rng) const {
    std::string current = code;
    for (const auto& distortion : chain_) current = distortion->apply(current, rng);
    return current;
}

RestoreReport Pipeline::restore(const std::string& received, std::size_t beam) const {
    // languages[i] - допустимые токены на входе i-го нарушения (до его наложения).
    std::vector<TokenPredicate> languages;
    languages.push_back(isValidToken);
    for (std::size_t i = 0; i + 1 < chain_.size(); ++i)
        languages.push_back(chain_[i]->widen(languages.back()));

    RestoreReport report;
    std::vector<Candidate> current{{received, 0}};
    for (std::size_t i = chain_.size(); i-- > 0;) {  // с конца цепочки к началу
        StepResult step = chain_[i]->restore(current, languages[i], beam);
        report.steps.push_back({chain_[i]->name(), current.size(), step.candidates.size(),
                                step.truncated});
        current = std::move(step.candidates);
    }
    report.variants = std::move(current);
    return report;
}

std::string Pipeline::describe() const {
    std::string forward, backward;
    for (std::size_t i = 0; i < chain_.size(); ++i) {
        if (i) forward += " -> ";
        forward += chain_[i]->name();
    }
    for (std::size_t i = chain_.size(); i-- > 0;) {
        if (i + 1 != chain_.size()) backward += " -> ";
        backward += chain_[i]->name();
    }
    return "наложение: " + forward + "; восстановление: " + backward;
}

}  // namespace morse
