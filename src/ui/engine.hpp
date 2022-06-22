#pragma once

#include "../md_tools/md_tools.hpp"

namespace mdui {

class Engine {
private:
    md::ROM& _rom;

    uint32_t _preinit_function_addr = 0x0;

    uint32_t _ui_mode_ram_addr        = 0xFFFFFF80; ///< the RAM address where the "UI Mode" state is stored (byte)
    uint32_t _input_bits_ram_addr     = 0xFFFFFF81; ///< the RAM address where the currently pressed input bits are stored (byte)
    uint32_t _input_repeat_ram_addr   = 0xFFFFFF82; ///< the RAM address where the input repeat frame counter is located (byte)
    uint32_t _current_option_ram_addr = 0xFFFFFF83; ///< the RAM address where the currently selected option is stored (byte)

    uint32_t _func_set_selected_option = 0x0;
    uint32_t _func_coords_to_plane_map_address = 0x0;
    uint32_t _func_set_palette = 0x0;
    uint32_t _func_apply_selection_mapping = 0x0;
    uint32_t _func_nemesis_decomp = 0x0;
    uint32_t _func_wait_vsync = 0x0;
    uint32_t _func_copy_plane_map_to_vram = 0x0;
    uint32_t _func_draw_string = 0x0;
    uint32_t _func_poll_controller = 0x0;
    uint32_t _func_v_int = 0x0;
    uint32_t _func_init_ui = 0x0;
    uint32_t _func_build_text_plane = 0x0;
    uint32_t _func_handle_ui_controls = 0x0;
    uint32_t _func_ui_main_loop = 0x0;
    uint32_t _func_boot_ui = 0x0;

public:
    Engine(md::ROM& rom, uint32_t preinit_function_addr = 0x0) :
        _rom                    (rom),
        _preinit_function_addr  (preinit_function_addr)
    {
        rom.set_long(0x78, func_v_int());
    }

    [[nodiscard]] uint32_t ui_mode_ram_addr() const { return _ui_mode_ram_addr; }
    void ui_mode_ram_addr(uint32_t addr) { _ui_mode_ram_addr = addr; }

    [[nodiscard]] uint32_t input_bits_ram_addr() const { return _input_bits_ram_addr; }
    void input_bits_ram_addr(uint32_t addr) { _input_bits_ram_addr = addr; }

    [[nodiscard]] uint32_t input_repeat_ram_addr() const { return _input_repeat_ram_addr; }
    void input_repeat_ram_addr(uint32_t addr) { _input_repeat_ram_addr = addr; }

    [[nodiscard]] uint32_t current_option_ram_addr() const { return _current_option_ram_addr; }
    void current_option_ram_addr(uint32_t addr) { _current_option_ram_addr = addr; }

    uint32_t func_set_selected_option();
//    uint32_t func_coords_to_plane_map_address();
    uint32_t func_set_palette();
    uint32_t func_apply_selection_mapping();

    uint32_t func_nemesis_decomp();
    uint32_t func_wait_vsync();
    uint32_t func_copy_plane_map_to_vram();
    uint32_t func_draw_string();
    uint32_t func_poll_controller();
    uint32_t func_v_int();
    uint32_t func_init_ui();
    uint32_t func_build_initial_text_plane();
    uint32_t func_handle_ui_controls();
    uint32_t func_ui_main_loop();
    uint32_t func_boot_ui();
};

} // namespace end