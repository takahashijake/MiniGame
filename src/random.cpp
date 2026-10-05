#include "minigame/random.hpp"

#include <algorithm>

namespace minigame {

Random::Random(std::optional<std::uint32_t> seed) {
    engine_.seed(seed.value_or(std::random_device{}()));
}

int Random::between(int minimum, int maximum) {
    if (minimum > maximum) {
        std::swap(minimum, maximum);
    }
    std::uniform_int_distribution<int> distribution(minimum, maximum);
    return distribution(engine_);
}

bool Random::chance(int percentage) {
    percentage = std::clamp(percentage, 0, 100);
    return between(1, 100) <= percentage;
}

}  // namespace minigame
