#pragma once

#include "../md_tools/md_tools.hpp"

namespace mdui {

class Engine {
private:
    md::ROM& _rom;

    uint32_t _preinit_function_addr = 0x0;
    uint8_t _alignment_helper_character = 0x3A; ///< Character used to align option labels and their values (defaults to '%')

    uint32_t _ui_mode_ram_addr            = 0xFFFFFF80; ///< the RAM address where the "UI Mode" state is stored (byte)
    uint32_t _input_bits_ram_addr         = 0xFFFFFF81; ///< the RAM address where the currently pressed input bits are stored (byte)
    uint32_t _input_repeat_ram_addr       = 0xFFFFFF82; ///< the RAM address where the input repeat frame counter is located (byte)
    uint32_t _current_selectable_ram_addr = 0xFFFFFF83; ///< the RAM address where the currently selected option is stored (byte)
    uint32_t _next_ui_addr                = 0xFFFFFF84; ///< the RAM address where a pointer on the next UI to display is stored (long)

    uint32_t _func_get_selectable_addr = 0x0;
    uint32_t _func_get_selectable_value_text_addr = 0x0;
    uint32_t _func_draw_all_selectable_values = 0x0;
    uint32_t _func_get_selectable_value = 0x0;
    uint32_t _func_set_option_value = 0x0;
    uint32_t _func_get_selectable_maximum_value = 0x0;
    uint32_t _func_set_current_selectable = 0x0;
    uint32_t _func_set_palette = 0x0;
    uint32_t _func_apply_selection_mapping = 0x0;
    uint32_t _func_nemesis_decomp = 0x0;
    uint32_t _func_wait_vsync = 0x0;
    uint32_t _func_copy_plane_map_to_vram = 0x0;
    uint32_t _func_erase_text = 0x0;
    uint32_t _func_draw_alignment_helper_line = 0x0;
    uint32_t _func_draw_text = 0x0;
    uint32_t _func_poll_controller = 0x0;
    uint32_t _func_v_int = 0x0;

    uint32_t _func_schedule_ui_change = 0x0;
    uint32_t _func_schedule_ui_exit = 0x0;
    uint32_t _func_init_engine = 0x0;
    uint32_t _func_init_ui = 0x0;
    uint32_t _func_build_initial_plane_map = 0x0;
    uint32_t _func_handle_ui_controls = 0x0;
    uint32_t _func_boot_ui = 0x0;

public:
    static constexpr uint8_t UI_MODE_DISABLED = 0x0;
    static constexpr uint8_t UI_MODE_ENABLED = 0x1;
    static constexpr uint8_t UI_MODE_V_INT_OCCURRED = 0x2;
    static constexpr uint8_t UI_MODE_EXIT = 0x2;
    static constexpr uint8_t UI_MODE_CHANGE = 0x3;

    explicit Engine(md::ROM& rom, uint32_t preinit_function_addr = 0x0) :
        _rom                    (rom),
        _preinit_function_addr  (preinit_function_addr)
    {
        rom.set_long(0x78, func_v_int());
    }

    [[nodiscard]] uint8_t alignment_helper_character() const { return _alignment_helper_character; }
    void alignment_helper_character(uint8_t character) { _alignment_helper_character = character; }

    [[nodiscard]] uint32_t ui_mode_ram_addr() const { return _ui_mode_ram_addr; }
    void ui_mode_ram_addr(uint32_t addr) { _ui_mode_ram_addr = addr; }

    [[nodiscard]] uint32_t input_bits_ram_addr() const { return _input_bits_ram_addr; }
    void input_bits_ram_addr(uint32_t addr) { _input_bits_ram_addr = addr; }

    [[nodiscard]] uint32_t input_repeat_ram_addr() const { return _input_repeat_ram_addr; }
    void input_repeat_ram_addr(uint32_t addr) { _input_repeat_ram_addr = addr; }

    [[nodiscard]] uint32_t current_selectable_addr() const { return _current_selectable_ram_addr; }
    void current_selectable_addr(uint32_t addr) { _current_selectable_ram_addr = addr; }

    uint32_t func_get_selectable_addr();
    uint32_t func_get_selectable_value_text_addr();
    uint32_t func_draw_all_selectable_values();
    uint32_t func_get_selectable_value();
    uint32_t func_set_option_value();
    uint32_t func_get_selectable_maximum_value();

    uint32_t func_set_current_selectable();
    uint32_t func_set_palette();
    uint32_t func_apply_selection_mapping();

    uint32_t func_nemesis_decomp();
    uint32_t func_wait_vsync();
    uint32_t func_copy_plane_map_to_vram();
    uint32_t func_erase_text();
    uint32_t func_draw_alignment_helper_line();
    uint32_t func_draw_text();
    uint32_t func_poll_controller();
    uint32_t func_v_int();

    uint32_t func_schedule_ui_change();
    uint32_t func_schedule_ui_exit();
    uint32_t func_init_engine();
    uint32_t func_init_ui();
    uint32_t func_build_initial_plane_map();
    uint32_t func_handle_ui_controls();
    uint32_t func_boot_ui();
};

} // namespace end