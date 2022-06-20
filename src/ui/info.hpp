#pragma once

#include <utility>

#include "../ui/string.hpp"

namespace md { class ROM; }
typedef std::tuple<uint8_t, uint8_t, uint8_t, uint8_t> UiSelectionMapping;


class UiInfo {
private:
    std::vector<UiString> _strings;
    std::vector<UiSelectionMapping> _selection_mappings;

    uint32_t _preinit_function_addr = 0;

    uint32_t _on_down_pressed_addr = 0;
    uint32_t _on_up_pressed_addr = 0;
    uint32_t _on_left_pressed_addr = 0;
    uint32_t _on_right_pressed_addr = 0;
    uint32_t _on_a_pressed_addr = 0;
    uint32_t _on_b_pressed_addr = 0;
    uint32_t _on_c_pressed_addr = 0;
    uint32_t _on_start_pressed_addr = 0;

    uint32_t _main_table_addr = 0;

public:
    static constexpr uint32_t STRINGS_OFFSET = 0x4 * 0;
    static constexpr uint32_t STRING_POSITIONS_OFFSET = 0x4 * 1;
    static constexpr uint32_t SELECTION_MAPPINGS_OFFSET = 0x4 * 2;
    static constexpr uint32_t PREINIT_FUNC_OFFSET = 0x4 * 3;
    static constexpr uint32_t CONTROLLER_EVENTS_OFFSET = 0x4 * 4;

    UiInfo(std::vector<UiString> strings, std::vector<UiSelectionMapping> selection_mappings) :
        _strings            (std::move(strings)),
        _selection_mappings (std::move(selection_mappings))
    {}

    uint32_t inject_data(md::ROM& rom);

    [[nodiscard]] const std::vector<UiString>& strings() const { return _strings; }
    [[nodiscard]] const std::vector<UiSelectionMapping>& selection_mappings() const { return _selection_mappings; }
    [[nodiscard]] uint8_t max_selection() const;

    [[nodiscard]] uint32_t preinit_function_addr() const { return _preinit_function_addr; }
    void preinit_function_addr(uint32_t addr) { _preinit_function_addr = addr; }

    [[nodiscard]] uint32_t on_down_pressed() const { return _on_down_pressed_addr; }
    void on_down_pressed(uint32_t addr) { _on_down_pressed_addr = addr; }

    [[nodiscard]] uint32_t on_up_pressed() const { return _on_up_pressed_addr; }
    void on_up_pressed(uint32_t addr) { _on_up_pressed_addr = addr; }

    [[nodiscard]] uint32_t on_left_pressed() const { return _on_left_pressed_addr; }
    void on_left_pressed(uint32_t addr) { _on_left_pressed_addr = addr; }

    [[nodiscard]] uint32_t on_right_pressed() const { return _on_right_pressed_addr; }
    void on_right_pressed(uint32_t addr) { _on_right_pressed_addr = addr; }

    [[nodiscard]] uint32_t on_a_pressed() const { return _on_a_pressed_addr; }
    void on_a_pressed(uint32_t addr) { _on_a_pressed_addr = addr; }

    [[nodiscard]] uint32_t on_b_pressed() const { return _on_b_pressed_addr; }
    void on_b_pressed(uint32_t addr) { _on_b_pressed_addr = addr; }

    [[nodiscard]] uint32_t on_c_pressed() const { return _on_c_pressed_addr; }
    void on_c_pressed(uint32_t addr) { _on_c_pressed_addr = addr; }

    [[nodiscard]] uint32_t on_start_pressed() const { return _on_start_pressed_addr; }
    void on_start_pressed(uint32_t addr) { _on_start_pressed_addr = addr; }

    [[nodiscard]] uint32_t main_table_address() const {
        if(!_main_table_addr)
            throw std::exception();
        return _main_table_addr;
    }

private:
    [[nodiscard]] ByteArray build_string_bytes() const;
    [[nodiscard]] ByteArray build_string_position_bytes() const;
    [[nodiscard]] ByteArray build_selection_mapping_bytes() const;
};
