#pragma once

#include <utility>
#include "info.hpp"
#include <iostream>

namespace mdui {

class Engine;

class VerticalMenu : public Info {
private:
    std::vector<std::vector<std::string>> _option_values;

public:
    explicit VerticalMenu(md::ROM& rom, Engine& engine) :
        Info()
    {
        this->on_down_pressed(this->inject_down_press_handler(rom, engine));
        this->on_up_pressed(this->inject_up_press_handler(rom, engine));
        std::cout << "Down press handler is at " << std::hex << this->on_down_pressed() << std::dec << std::endl;
    }

private:
    uint32_t inject_down_press_handler(md::ROM& rom, Engine& engine) const;
    uint32_t inject_up_press_handler(md::ROM& rom, Engine& engine) const;
};


} // namespace mdui