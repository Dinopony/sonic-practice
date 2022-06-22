#pragma once

#include <utility>
#include <array>

#include "text.hpp"

namespace md { class ROM; }

namespace mdui {

/// <r, g, b>
typedef std::tuple<uint8_t, uint8_t, uint8_t> Color;

/// <option, x, y, size>
typedef std::tuple<uint8_t, uint8_t, uint8_t, uint8_t> SelectionMapping;

class Info {
private:
    std::vector<Text> _strings;
    std::vector<SelectionMapping> _selection_mappings;

    Color _background_color = {0x2,0x2,0x2};
    std::pair<Color, Color> _text_color_palette_neutral = { {0x8,0x8,0x8}, {0xA,0xA,0xA} };
    std::pair<Color, Color> _text_color_palette_selected = { {0xF,0xA,0x0}, {0xF,0xC,0x4} };

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
    static constexpr uint32_t STRINGS_OFFSET = 0;
    static constexpr uint32_t STRING_POSITIONS_OFFSET = STRINGS_OFFSET + 0x4;
    static constexpr uint32_t SELECTION_MAPPINGS_OFFSET = STRING_POSITIONS_OFFSET + 0x4;
    static constexpr uint32_t PREINIT_FUNC_OFFSET = SELECTION_MAPPINGS_OFFSET + 0x4;
    static constexpr uint32_t CONTROLLER_EVENTS_OFFSET = PREINIT_FUNC_OFFSET + 0x4;
    static constexpr uint32_t COLOR_PALETTES_OFFSET = CONTROLLER_EVENTS_OFFSET + (0x4 * 8);
    static constexpr uint8_t LAST_OPTION_ID_OFFSET = COLOR_PALETTES_OFFSET + (0x2 * 5);
    static constexpr uint32_t INFO_END_OFFSET = LAST_OPTION_ID_OFFSET + 1;

    Info() = default;

    virtual uint32_t inject(md::ROM& rom);

    [[nodiscard]] const std::vector<Text>& strings() const { return _strings; }
    [[nodiscard]] const std::vector<SelectionMapping>& selection_mappings() const { return _selection_mappings; }
    void add_string(uint8_t x, uint8_t y, const std::string& str);
    void add_option(uint8_t x, uint8_t y, const std::string& str, uint8_t option_id);

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

    [[nodiscard]] const Color& background_color() const { return _background_color; }
    void background_color(const Color& color) { _background_color = color; }

    [[nodiscard]] const std::pair<Color, Color>& text_color_palette_neutral() const { return _text_color_palette_neutral; }
    void text_color_palette_neutral(const std::pair<Color, Color>& colors) { _text_color_palette_neutral = colors; }

    [[nodiscard]] const std::pair<Color, Color>& text_color_palette_selected() const { return _text_color_palette_selected; }
    void text_color_palette_selected(const std::pair<Color, Color>& colors) { _text_color_palette_selected = colors; }

    [[nodiscard]] uint8_t last_option_id() const;

    [[nodiscard]] uint32_t main_table_address() const {
        if(!_main_table_addr)
            throw std::exception();
        return _main_table_addr;
    }

private:
    [[nodiscard]] ByteArray build_string_bytes() const;
    [[nodiscard]] ByteArray build_string_position_bytes() const;
    [[nodiscard]] ByteArray build_selection_mapping_bytes() const;
    [[nodiscard]] ByteArray build_palette_bytes() const;

    virtual void extend_data_table(md::ROM& rom, ByteArray& data_table) const {}
};

} // namespace end