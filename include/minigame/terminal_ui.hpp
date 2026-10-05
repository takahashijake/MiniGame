#pragma once

#include <string>
#include <string_view>
#include <vector>

namespace minigame {

class Campaign;
class Enemy;

class TerminalUI {
public:
    TerminalUI();

    void clear() const;
    void banner() const;
    void world(const Campaign& campaign) const;
    void inventory(const Campaign& campaign) const;
    void battle(const Campaign& campaign, const Enemy& enemy,
                const std::vector<std::string>& events) const;
    void message(std::string_view text) const;

    std::string prompt(std::string_view text) const;
    char command(std::string_view text) const;
    void pause() const;

private:
    std::string color(std::string_view code, std::string_view text) const;
    std::string healthBar(int current, int maximum, int width = 24) const;

    bool colorEnabled_{true};
    bool clearEnabled_{true};
};

}  // namespace minigame
