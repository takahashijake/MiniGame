#ifndef MINIGAME_RANDOM_H
#define MINIGAME_RANDOM_H

#include <cstdint>
#include <random>

namespace minigame {

class RandomSource {
public:
    virtual ~RandomSource() = default;
    virtual int between(int minimum, int maximum) = 0;

    bool chance(int percent);
};

class RandomGenerator final : public RandomSource {
public:
    RandomGenerator();
    explicit RandomGenerator(std::uint32_t seed);

    int between(int minimum, int maximum) override;

private:
    std::mt19937 engine_;
};

}  // namespace minigame

#endif
