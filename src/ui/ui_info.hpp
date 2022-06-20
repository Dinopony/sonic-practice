#pragma once

#include <utility>

#include "../ui/ui_string.hpp"

typedef std::tuple<uint8_t, uint8_t, uint8_t, uint8_t> UiSelectionMapping;

class UiInfo {
private:
    std::vector<UiString> _strings;
    std::vector<UiSelectionMapping> _selection_mappings;

public:
    UiInfo(std::vector<UiString> strings, std::vector<UiSelectionMapping> selection_mappings) :
        _strings            (std::move(strings)),
        _selection_mappings (std::move(selection_mappings))
    {}

    [[nodiscard]] const std::vector<UiString>& strings() const { return _strings; }
    [[nodiscard]] const std::vector<UiSelectionMapping>& selection_mappings() const { return _selection_mappings; }
    [[nodiscard]] uint8_t max_selection() const
    {
        uint8_t highest_sel = 0;
        for(const auto& [sel,_2,_3,_4] : _selection_mappings)
            if(sel > highest_sel)
                highest_sel = sel;
        return highest_sel;
    }
};
