#include "minigame/random.h"

#include <stdexcept>

namespace minigame {

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

}  // namespace minigame
