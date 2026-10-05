#pragma once

#include <cstdint>
#include <optional>
#include <random>

namespace minigame {

class Random {
public:
    explicit Random(std::optional<std::uint32_t> seed = std::nullopt);

    int between(int minimum, int maximum);
    bool chance(int percentage);

private:
    std::mt19937 engine_;
};

}  // namespace minigame
