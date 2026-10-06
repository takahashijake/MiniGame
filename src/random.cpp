#include "minigame/random.h"

#include <sstream>
#include <cstdint>
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

    // Fixed rejection mapping: standard-library distributions differ across platforms.
    const std::uint64_t range = static_cast<std::uint64_t>(
        static_cast<std::int64_t>(maximum) - minimum) + 1;
    constexpr std::uint64_t space = std::uint64_t{1} << 32;
    const std::uint64_t limit = space - space % range;
    std::uint64_t roll;
    do {
        roll = engine_();
    } while (roll >= limit);
    return static_cast<int>(static_cast<std::int64_t>(minimum) + static_cast<std::int64_t>(roll % range));
}

std::string RandomGenerator::serializeState() const {
    std::ostringstream output;
    output << engine_;
    return output.str();
}

bool RandomGenerator::restoreState(const std::string& state) {
    std::istringstream input(state);
    auto restored = engine_;
    input >> restored;
    if (input.fail()) return false;
    input >> std::ws;
    if (!input.eof()) return false;
    engine_ = restored;
    return true;
}

}  // namespace minigame
