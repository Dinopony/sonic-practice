#pragma once

#include <utility>
#include "info.hpp"

namespace mdui {


class VerticalMenu : public Info {
private:
    std::vector<std::vector<std::string>> _options;

public:
    VerticalMenu(std::vector<Text> strings, std::vector<SelectionMapping> selection_mappings) :
        Info(std::move(strings), std::move(selection_mappings))
    {}

    uint32_t inject(md::ROM& rom) override;

private:
    uint32_t inject_down_press_handler(md::ROM& rom);
    uint32_t inject_up_press_handler(md::ROM& rom);
    // Up pressed: decrease selected option by one, looping to the end if needed


    // TODO: on plane constitution, add options to the right

    // TODO: on init, load options from SRAM
};


} // namespace mdui