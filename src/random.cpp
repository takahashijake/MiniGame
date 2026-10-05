#include "minigame/random.h"

#include <sstream>
#include <stdexcept>

namespace minigame {

std::string RandomSource::serializeState() const {
    return {};
}

bool RandomSource::restoreState(const std::string& state) {
    return state.empty();
}

bool RandomSource::chance(int percent) {
    if (percent <= 0) {
        return false;
    }
    if (percent >= 100) {
        return true;
    }
    return between(1, 100) <= percent;
}

RandomGenerator::RandomGenerator() : engine_(std::random_device{}()) {}

RandomGenerator::RandomGenerator(std::uint32_t seed) : engine_(seed) {}

int RandomGenerator::between(int minimum, int maximum) {
    if (minimum > maximum) {
        throw std::invalid_argument("minimum cannot exceed maximum");
    }

    std::uniform_int_distribution<int> distribution(minimum, maximum);
    return distribution(engine_);
}

std::string RandomGenerator::serializeState() const {
    std::ostringstream output;
    output << engine_;
    return output.str();
}

bool RandomGenerator::restoreState(const std::string& state) {
    std::istringstream input(state);
    input >> engine_;
    return !input.fail();
}

}  // namespace minigame
