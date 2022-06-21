#pragma once

#include "../md_tools/md_tools.hpp"

namespace mdui {

class Engine {
private:
    uint32_t _ui_mode_ram_addr;         ///< the RAM address where the "UI Mode" state is stored (byte)
    uint32_t _input_repeat_ram_addr;    ///< the RAM address where the input repeat frame counter is located (byte)

    uint32_t _func_nemesis_decomp = 0x0;
    uint32_t _func_wait_vsync = 0x0;
    uint32_t _func_copy_plane_map_to_vram = 0x0;
    uint32_t _func_draw_string = 0x0;
    uint32_t _func_poll_controller = 0x0;
    uint32_t _func_v_int = 0x0;
    uint32_t _func_init_ui = 0x0;
    uint32_t _func_build_text_plane = 0x0;
    uint32_t _func_handle_ui_controls = 0x0;
    uint32_t _func_mark_fields = 0x0;
    uint32_t _func_ui_main_loop = 0x0;
    uint32_t _func_boot_ui = 0x0;

public:
    Engine(uint32_t ui_mode_ram_addr, uint32_t input_repeat_ram_addr) :
        _ui_mode_ram_addr       (ui_mode_ram_addr),
        _input_repeat_ram_addr  (input_repeat_ram_addr)
    {}

    uint32_t inject(md::ROM& rom);

private:
    uint32_t inject_func_nemesis_decomp(md::ROM& rom);
    uint32_t inject_func_wait_vsync(md::ROM& rom);
    uint32_t inject_func_copy_plane_map_to_vram(md::ROM& rom);
    uint32_t inject_func_draw_string(md::ROM& rom);
    uint32_t inject_func_poll_controller(md::ROM& rom);
    uint32_t inject_func_v_int(md::ROM& rom, uint32_t current_v_int_handler);
    uint32_t inject_func_init_ui(md::ROM& rom, uint32_t nemesis_decomp);
    uint32_t inject_func_build_text_plane(md::ROM& rom, uint32_t copy_plane_map_to_vram, uint32_t draw_string);
    uint32_t inject_func_handle_ui_controls(md::ROM& rom, uint32_t poll_controller);
    uint32_t inject_func_mark_fields(md::ROM& rom);
    uint32_t inject_func_ui_main_loop(md::ROM& rom, uint32_t handle_ui_controls, uint32_t mark_fields, uint32_t wait_vsync);
    uint32_t inject_func_boot_ui(md::ROM& rom, uint32_t init_ui, uint32_t build_text_plane, uint32_t ui_main_loop);
};

} // namespace end