#include "engine.hpp"
#include "info.hpp"
#include "../../assets/nem_decomp.bin.hxx"

namespace mdui {

// TODO: It will be needed at some point to have some kind of "UiEngine" class that:
//          - injects the UI core
//          - handles control engine using the appropriate RAM space for the game
//        Info then becomes a structure that contains a specific menu descriptor, but no redundant core code

// TODO: Override Vint with a custom one that does whatever the current good behavior is,
//       and then jumps to the regular Vint if UI is not enabled
//       S3K executes VInt at FFF0 in RAM, leading to 0x584 (VInt function)
//       - Change ROM header to execute a custom VInt
//       - Custom VInt tests (UI_Mode).b.
//              * If true, execute current minimum viable behavior (Poll inputs + VSync VDP stuff?)
//              * If false, JMP to old address in VInt table
//       - This should work for absolutely any game

// Global RAM offsets that are always valid
constexpr uint32_t RAM_start = 0xFFFF0000;
constexpr uint32_t VDP_data_port = 0xC00000;
constexpr uint32_t VDP_control_port = 0xC00004;
constexpr uint32_t Controller_1_data_port = 0xA10002;

constexpr uint8_t UI_MODE_DISABLED = 0x0;
constexpr uint8_t UI_MODE_ENABLED = 0x1;
constexpr uint8_t UI_MODE_V_INT_OCCURRED = 0x2;

constexpr uint32_t V_int_routine = 0xFFFFF62A;

#include "../../assets/gui_tileset.bin.hxx"

uint32_t Engine::inject(md::ROM& rom)
{
    // Replace base game's VInt by a custom VInt that only triggers when UI is displayed
    _func_v_int = inject_func_v_int(rom, rom.get_long(0x78));
    rom.set_long(0x78, _func_v_int);

    _func_nemesis_decomp = inject_func_nemesis_decomp(rom);
    _func_init_ui = inject_func_init_ui(rom, _func_nemesis_decomp);

    _func_copy_plane_map_to_vram = inject_func_copy_plane_map_to_vram(rom);
    _func_draw_string = inject_func_draw_string(rom);
    _func_build_text_plane = inject_func_build_text_plane(rom, _func_copy_plane_map_to_vram, _func_draw_string);

    _func_poll_controller = inject_func_poll_controller(rom);
    _func_handle_ui_controls = inject_func_handle_ui_controls(rom, _func_poll_controller);

    _func_mark_fields = inject_func_mark_fields(rom);
    _func_wait_vsync = inject_func_wait_vsync(rom);
    _func_ui_main_loop = inject_func_ui_main_loop(rom, _func_handle_ui_controls, _func_mark_fields, _func_wait_vsync);

    _func_boot_ui = inject_func_boot_ui(rom, _func_init_ui, _func_build_text_plane, _func_ui_main_loop);
    return _func_boot_ui;
}

uint32_t Engine::inject_func_nemesis_decomp(md::ROM& rom)
{
    return rom.inject_bytes(NEM_DECOMP, NEM_DECOMP_SIZE);
}

uint32_t Engine::inject_func_v_int(md::ROM& rom, uint32_t current_v_int_handler)
{
    md::Code func_v_int;

    func_v_int.nop();
    func_v_int.tstb(addr_(_ui_mode_ram_addr));
    func_v_int.beq("fallback_to_old_vint"); // If UI Mode == 0, jump to the usual VInt for this game

    func_v_int.label("wait_for_vertical_blanking");
    func_v_int.movew(addr_(VDP_control_port), reg_D0);
    func_v_int.andiw(0x8, reg_D0);
    func_v_int.beq("wait_for_vertical_blanking");

    func_v_int.moveb(UI_MODE_V_INT_OCCURRED, addr_(_ui_mode_ram_addr)); // Set GUI Mode to "VInt occurred"
    func_v_int.jmp(current_v_int_handler);
    func_v_int.rte();

    func_v_int.label("fallback_to_old_vint");
    func_v_int.jmp(current_v_int_handler);

    return rom.inject_code(func_v_int);
//    move.l	#vdpComm($0000,VSRAM,WRITE),(VDP_control_port).l
//    move.l	(V_scroll_value).w,(VDP_data_port).l

//    btst	    #6, (Graphics_flags).w
//    beq.s	+	; branch if it's not a PAL system
//    move.w	#$700,d0
//    -
//    dbf d0,-	; otherwise, waste a bit of time here
//    +

//    dma68kToVDP Normal_palette,$0000,$80,CRAM
//    dma68kToVDP Sprite_table_buffer,$F800,$280,VRAM
//    dma68kToVDP H_scroll_buffer,$F000,$380,VRAM

//    bsr.w	Process_DMA_Queue
//            startZ80
//    bsr.w	Process_Nem_Queue
}

uint32_t Engine::inject_func_wait_vsync(md::ROM& rom)
{
    md::Code func_wait_vsync;

    func_wait_vsync.move_to_sr(0x2300);
    func_wait_vsync.label("loop");
    func_wait_vsync.tstb(addr_(V_int_routine));
    func_wait_vsync.bne("loop"); // Wait until V-int's run
    func_wait_vsync.rts();

    return rom.inject_code(func_wait_vsync);
}

/*
{
    md::Code func_wait_vsync;

    func_wait_vsync.move_to_sr(0x2300); // Enable VInts

    func_wait_vsync.label("wait_for_v_int");
    func_wait_vsync.cmpib(UI_MODE_V_INT_OCCURRED, addr_(_ui_mode_ram_addr));
    func_wait_vsync.bne("wait_for_v_int");

    func_wait_vsync.moveb(UI_MODE_ENABLED, addr_(_ui_mode_ram_addr));

    func_wait_vsync.move_to_sr(0x2700); // Disable VInts
    func_wait_vsync.rts();

    return rom.inject_code(func_wait_vsync);
}
 */

uint32_t Engine::inject_func_copy_plane_map_to_vram(md::ROM& rom)
{
    // Params:
    //      - D0.w: VDP control word
    //      - D1.b: tiles per row
    //      - D2.b: number of rows to copy
    md::Code func_copy_plane_map_to_vram;

    func_copy_plane_map_to_vram.lea(addr_(RAM_start), reg_A1);
    func_copy_plane_map_to_vram.movel(0x40000003, reg_D0);
    func_copy_plane_map_to_vram.moveq(0x27, reg_D1);
    func_copy_plane_map_to_vram.moveq(0x1B, reg_D2);

    func_copy_plane_map_to_vram.lea(addr_(VDP_data_port), reg_A6);

    func_copy_plane_map_to_vram.label("loop_row");
    func_copy_plane_map_to_vram.movel(reg_D0, addr_(VDP_control_port));
    func_copy_plane_map_to_vram.movew(reg_D1, reg_D3);

    func_copy_plane_map_to_vram.label("loop_tile");
    func_copy_plane_map_to_vram.movew(addr_postinc_(reg_A1), addr_(reg_A6));
    func_copy_plane_map_to_vram.dbra(reg_D3, "loop_tile");   // copy one row

    func_copy_plane_map_to_vram.addil(0x800000, reg_D0); // move onto next row
    func_copy_plane_map_to_vram.dbra(reg_D2, "loop_row");
    func_copy_plane_map_to_vram.rts();

    return rom.inject_code(func_copy_plane_map_to_vram, "copy_plane_map_to_vram");
}

uint32_t Engine::inject_func_draw_string(md::ROM& rom)
{
    // Params:
    //      - A1: string starting address
    //      - A3: RAM graphics buffer start
    //      - D3.w: string position
    md::Code func_draw_string;

    // Puts the exact RAM address where to write in A2
    func_draw_string.lea(addrw_(reg_A3, reg_D3), reg_A2);
    // Store the string size (first byte of the string) in D2
    func_draw_string.moveq(0, reg_D2);
    func_draw_string.moveb(addr_postinc_(reg_A1), reg_D2);

    // Iterate D2 times to write each letter tile of the string into the RAM
    func_draw_string.label("loop_write_letter");
    func_draw_string.moveb(addr_postinc_(reg_A1), reg_D0);
    func_draw_string.movew(reg_D0, addr_postinc_(reg_A2));
    func_draw_string.dbra(reg_D2, "loop_write_letter");
    func_draw_string.rts();

    return rom.inject_code(func_draw_string);
}

uint32_t Engine::inject_func_poll_controller(md::ROM& rom)
{
    md::Code func_poll_controller;
    func_poll_controller.movem_to_stack({ reg_D1 }, { reg_A0 });

    // Might be necessary to ensure this works in any game:
    // moveq	#$40,d0
    // move.b	d0,(HW_Port_1_Control).l
    func_poll_controller.lea(addr_(Controller_1_data_port), reg_A0);

    func_poll_controller.moveb(0x00, addr_(reg_A0)); // Poll controller data port
    func_poll_controller.nop(2);
    func_poll_controller.moveb(addr_(reg_A0), reg_D0); // Get controller port data (start/A)
    func_poll_controller.lslb(0x2, reg_D0);
    func_poll_controller.andib(0xC0, reg_D0);

    func_poll_controller.moveb(0x40, addr_(reg_A0)); // Poll controller data port again
    func_poll_controller.nop(2);
    func_poll_controller.moveb(addr_(reg_A0), reg_D1); // Get controller port data (B/C/Dpad)
    func_poll_controller.andib(0x3F, reg_D1);
    func_poll_controller.orb(reg_D1, reg_D0); // Fuse together into one controller bit array
    func_poll_controller.notb(reg_D0);

    func_poll_controller.movem_from_stack({ reg_D1 }, { reg_A0 });
    func_poll_controller.rts();

    return rom.inject_code(func_poll_controller);
}

uint32_t Engine::inject_func_build_text_plane(md::ROM& rom, uint32_t copy_plane_map_to_vram, uint32_t draw_string)
{
    md::Code func_build_text_plane;

    func_build_text_plane.lea(addr_(RAM_start), reg_A3);
    func_build_text_plane.movel(addr_(reg_A4, Info::STRINGS_OFFSET), reg_A1);
    func_build_text_plane.movel(addr_(reg_A4, Info::STRING_POSITIONS_OFFSET), reg_A5);
    func_build_text_plane.moveq(0, reg_D0);

    // Double loop to write each letter of each line of text
    func_build_text_plane.label("loop_write_string");
    func_build_text_plane.movew(addr_postinc_(reg_A5), reg_D3);
    func_build_text_plane.cmpiw(0xFFFF, reg_D3);
    func_build_text_plane.beq("complete");
    func_build_text_plane.jsr(draw_string);
    func_build_text_plane.bra("loop_write_string");

    // Send our built plane map to VRAM
    func_build_text_plane.label("complete");
    func_build_text_plane.jsr(copy_plane_map_to_vram);

    func_build_text_plane.rts();

    return rom.inject_code(func_build_text_plane);
}

uint32_t Engine::inject_func_handle_ui_controls(md::ROM& rom, uint32_t poll_controller)
{
    auto HANDLE_BUTTON_PRESS = [this](md::Code& func, const std::string& button_name, uint8_t button_bit)
    {
        std::string end_label = button_name + "_end";

        // If callback address is 0x0, it means this button is not handled, so no need to check it
        func.cmpil(0x0, addr_(reg_A1));
        func.beq(end_label);

        // There is a valid callback, test the actual bit for this button
        func.btst(button_bit, reg_D0);
        func.beq(end_label);

        // Button is pressed, call the callback function and set the repeat counter to prevent
        // high-speed consecutive inputs
        func.movel(addr_(reg_A1), reg_A2);
        func.jsr(addr_(reg_A2));
        func.moveb(0x9, addr_(_input_repeat_ram_addr));

        func.label(end_label);
        func.addql(0x4, reg_A1);
    };

    md::Code func_handle_gui_controls;

    // First of all, check if we didn't already press a button recently. If so, just leave.
    func_handle_gui_controls.tstb(addr_(_input_repeat_ram_addr));
    func_handle_gui_controls.beq("can_process_inputs");
    func_handle_gui_controls.subib(1, addr_(_input_repeat_ram_addr));
    func_handle_gui_controls.rts();

    func_handle_gui_controls.label("can_process_inputs");
    // Make D0 contain the currently pressed button bits
//    func_handle_gui_controls.jsr(poll_controller); // TODO: Our own poll mechanism is broken for pretty much any button beyond up and down
    func_handle_gui_controls.moveb(addr_(0xFFFFF604), reg_D0);
    // Make A1 point on the first controller event callback address
    func_handle_gui_controls.lea(addr_(reg_A4, Info::CONTROLLER_EVENTS_OFFSET), reg_A1);

    // Test consecutively each button, and call the appropriate callback if pressed
    HANDLE_BUTTON_PRESS(func_handle_gui_controls, "up",     0);
    HANDLE_BUTTON_PRESS(func_handle_gui_controls, "down",   1);
    HANDLE_BUTTON_PRESS(func_handle_gui_controls, "left",   2);
    HANDLE_BUTTON_PRESS(func_handle_gui_controls, "right",  3);
    HANDLE_BUTTON_PRESS(func_handle_gui_controls, "b",      4);
    HANDLE_BUTTON_PRESS(func_handle_gui_controls, "c",      5);
    HANDLE_BUTTON_PRESS(func_handle_gui_controls, "a",      6);
    HANDLE_BUTTON_PRESS(func_handle_gui_controls, "start",  7);

    func_handle_gui_controls.rts();

    return rom.inject_code(func_handle_gui_controls);
}

uint32_t Engine::inject_func_mark_fields(md::ROM& rom)
{
    md::Code func_change_palette;

    // Read X position beforehand, double it and store it in D2 to re-use it later
    func_change_palette.moveq(0, reg_D2);
    func_change_palette.moveb(addr_(reg_A5), reg_D2);
    func_change_palette.lslb(1, reg_D2);

    // Read info from A5 to compose D1 (VDP control word)
    func_change_palette.moveq(0, reg_D1);
    func_change_palette.moveb(addr_(reg_A5, 0x1), reg_D1);
    func_change_palette.beq("return");
    func_change_palette.lslw(7, reg_D1);
    func_change_palette.addb(reg_D2, reg_D1);
    func_change_palette.addiw(0xC000, reg_D1); // VRAM_Plane_A_Name_Table
    func_change_palette.lsll(2, reg_D1);
    func_change_palette.lsrw(2, reg_D1);
    func_change_palette.oriw(0x4000, reg_D1); // #vdpComm($0000,VRAM,WRITE) >> 16
    func_change_palette.swap(reg_D1);
    func_change_palette.movel(reg_D1, addr_(VDP_control_port));

    // Read the same info from A5 to deduce A1 (base address where to alter used palette)
    func_change_palette.moveq(0, reg_D0);
    func_change_palette.moveb(addr_(reg_A5, 0x1), reg_D0);
    func_change_palette.mulu(0x50, reg_D0);
    func_change_palette.addw(reg_D2, reg_D0);
    func_change_palette.lea(addrw_(reg_A2, reg_D0), reg_A1);

    // Send the tile (A1) with palette ID (D3) on the VDP Data port (A6)
    func_change_palette.moveq(0, reg_D1);
    func_change_palette.moveb(addr_(reg_A5, 0x2), reg_D1);
    func_change_palette.label("loop");
    func_change_palette.movew(addr_postinc_(reg_A1), reg_D0);
    func_change_palette.addw(reg_D3, reg_D0);
    func_change_palette.movew(reg_D0, addr_(reg_A6));
    func_change_palette.dbra(reg_D1, "loop");

    func_change_palette.label("return");
    func_change_palette.rts();
    uint32_t func_change_palette_addr = rom.inject_code(func_change_palette);

    ////////////////

    md::Code func_mark_fields;
    func_mark_fields.lea(addr_(RAM_start), reg_A2);

    func_mark_fields.movel(addr_(reg_A4, Info::CURRENT_OPTION_ADDR_OFFSET), reg_A5);
    func_mark_fields.moveq(0, reg_D0);
    func_mark_fields.movew(addr_(reg_A5), reg_D0);

    func_mark_fields.movel(addr_(reg_A4, Info::SELECTION_MAPPINGS_OFFSET), reg_A5);
    func_mark_fields.lea(addr_(VDP_data_port), reg_A6);

    func_mark_fields.label("loop");
    func_mark_fields.cmpb(addr_postinc_(reg_A5), reg_D0);
    func_mark_fields.bmi("return");
    func_mark_fields.bne("next_mapping");
    func_mark_fields.jsr(func_change_palette_addr);
    func_mark_fields.label("next_mapping");
    func_mark_fields.addqw(0x3, reg_A5);
    func_mark_fields.bra("loop");

    func_mark_fields.label("return");
    func_mark_fields.rts();

    return rom.inject_code(func_mark_fields);
}

uint32_t Engine::inject_func_ui_main_loop(md::ROM& rom, uint32_t handle_ui_controls, uint32_t mark_fields, uint32_t wait_vsync)
{
    md::Code func_gui_main_loop;
    func_gui_main_loop.label("begin"); // routine running during level select

    // Wait for the frame to be drawn by the VDP before processing another one
    func_gui_main_loop.move_to_sr(0x2300); // Enable VInts
    func_gui_main_loop.moveb(0x16, addr_(V_int_routine));
    func_gui_main_loop.jsr(wait_vsync);
    func_gui_main_loop.move_to_sr(0x2700); // Disable VInts

    func_gui_main_loop.moveq(0, reg_D3); // Palette line 0
    func_gui_main_loop.jsr(mark_fields); // 0x7F62

    func_gui_main_loop.jsr(handle_ui_controls);

    func_gui_main_loop.movew(0x2000, reg_D3); // Palette line 1
    func_gui_main_loop.jsr(mark_fields); // 0x7F62

//    func_settings_menu.moveb(addr_(0xF605), reg_D0); // CTRL_1_PRESSED
//    func_settings_menu.orb(addr_(0xF607), reg_D0); // CTRL_2_PRESSED
//    func_settings_menu.andib(0x80, reg_D0);
//    func_settings_menu.bne(LevelSelect_PressStart);
    func_gui_main_loop.bra("begin");

    return rom.inject_code(func_gui_main_loop);
}

uint32_t Engine::inject_func_init_ui(md::ROM& rom, uint32_t nemesis_decomp)
{
    uint32_t gui_tileset_addr = rom.inject_bytes(GUI_TILESET, GUI_TILESET_SIZE);

    md::Code func_init_gui;
    func_init_gui.move_to_sr(0x2700); // Disable VInts
//    func_init_gui.movew(addrw_(VDP_reg_1_command), reg_D0);  // 0xFFFFF60E
//    func_init_gui.andib(0xBF, reg_D0);
//    func_init_gui.movew(reg_D0, addr_(VDP_control_port));
//    func_init_gui.jsr(Clear_DisplayData);     // 0x11CA
    func_init_gui.lea(addr_(VDP_control_port), reg_A6);
    func_init_gui.movew(0x8004, addr_(reg_A6));
    func_init_gui.movew(0x8230, addr_(reg_A6));
    func_init_gui.movew(0x8407, addr_(reg_A6));
    func_init_gui.movew(0x8230, addr_(reg_A6));
    func_init_gui.movew(0x8700, addr_(reg_A6));
    func_init_gui.movew(0x8C81, addr_(reg_A6));
    func_init_gui.movew(0x9001, addr_(reg_A6));
    func_init_gui.movew(0x8B00, addr_(reg_A6));

//    func_init_gui.clrw(addrw_(DMA_queue));  // 0xFFFFFB00
//    func_init_gui.movel(DMA_queue, addrw_(DMA_queue_slot)); // 0xFFFFFBFC
    func_init_gui.movel(0x42000000, addr_(VDP_control_port));

    // Decompress GUI tileset
    func_init_gui.lea(addr_(gui_tileset_addr), reg_A0);
    func_init_gui.jsr(nemesis_decomp);

    func_init_gui.moveb(UI_MODE_ENABLED, addr_(_ui_mode_ram_addr));

    func_init_gui.rts();

    return rom.inject_code(func_init_gui);
}

uint32_t Engine::inject_func_boot_ui(md::ROM& rom, uint32_t init_ui, uint32_t build_text_plane, uint32_t ui_main_loop)
{
    md::Code func_boot_gui;

    // Call the pre-init function if there is one
    func_boot_gui.movel(addr_(reg_A4, Info::PREINIT_FUNC_OFFSET), reg_A1);
    func_boot_gui.cmpa(lval_(0x0), reg_A1);
    func_boot_gui.beq("after_preinit");
    func_boot_gui.jsr(addr_(reg_A1));

    func_boot_gui.label("after_preinit");
    func_boot_gui.jsr(init_ui); // func_settings_menu.jmp(0x7B34);
    func_boot_gui.jsr(build_text_plane); // func_settings_menu.jmp(0x7BB2);

    // Init palette
    constexpr uint16_t PALETTE_0_ADDR = 0xFC00;
    func_boot_gui.lea(addr_(reg_A4, Info::COLOR_PALETTES_OFFSET), reg_A1);
    func_boot_gui.movew(addr_postinc_(reg_A1), addrw_(PALETTE_0_ADDR));
    func_boot_gui.movew(addr_postinc_(reg_A1), addrw_(PALETTE_0_ADDR + 0xC));
    func_boot_gui.movew(addr_postinc_(reg_A1), addrw_(PALETTE_0_ADDR + 0xE));
    constexpr uint16_t PALETTE_1_ADDR = PALETTE_0_ADDR + 0x20;
    func_boot_gui.movew(addr_postinc_(reg_A1), addrw_(PALETTE_1_ADDR + 0xC));
    func_boot_gui.movew(addr_postinc_(reg_A1), addrw_(PALETTE_1_ADDR + 0xE));

//    func_settings_menu.moveb(0x16, addr_(V_int_routine));
//    func_settings_menu.jsr(Wait_VSync);
    func_boot_gui.movew(0x8134/*addr_(VDP_reg_1_command)*/, reg_D0);
    func_boot_gui.orib(0x40, reg_D0);
    func_boot_gui.movew(reg_D0, addr_(VDP_control_port));

    func_boot_gui.jsr(ui_main_loop);
    func_boot_gui.rts();

    return rom.inject_code(func_boot_gui);
}

} // namespace end