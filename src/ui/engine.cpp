#include "engine.hpp"

// TODO: It will be needed at some point to have some kind of "UiEngine" class that:
//          - injects the UI core
//          - handles control engine using the appropriate RAM space for the game
//        UiInfo then becomes a structure that contains a specific menu descriptor, but no redundant core code

// ROM offsets that are only valid in S3K (i.e. functions that need to be ported over)
constexpr uint32_t Wait_VSync = 0x1D18;
constexpr uint32_t Plane_Map_To_VRAM = 0x14E6;
constexpr uint32_t Nem_Decomp = 0x15BA;
constexpr uint32_t Clear_DisplayData = 0x11CA;

// Global RAM offsets that are always valid
constexpr uint32_t RAM_start = 0xFFFF0000;
constexpr uint32_t VDP_data_port = 0xC00000;
constexpr uint32_t VDP_control_port = 0xC00004;

// RAM offsets that are most likely only valid in S3K
constexpr uint32_t VDP_reg_1_command = 0xFFFFF60E;
constexpr uint32_t DMA_queue = 0xFFFFFB00;
constexpr uint32_t V_int_routine = 0xFFFFF62A;
constexpr uint32_t DMA_queue_slot = 0xFFFFFBFC;
constexpr uint32_t Ctrl_1 = 0xFFFFF604;
constexpr uint32_t Ctrl_1_pressed = 0xFFFFF605;
constexpr uint32_t Level_select_repeat = 0xFFFFFF80;
constexpr uint32_t Level_select_option = 0xFFFFFF82;

#include "../../assets/gui_tileset.bin.hxx"

//////////////////////////////////////////////////////
///     DATA INJECTION FOR WINDOW DESCRIPTORS
//////////////////////////////////////////////////////

uint32_t inject_text_strings(md::ROM& rom, const UiInfo& ui_info)
{
    ByteArray bytes;
    for(const UiString& str : ui_info.strings())
        bytes.add_bytes(str.text_bytes());

    bytes.add_byte(0xFF);
    return rom.inject_bytes(bytes);
}

uint32_t inject_text_mappings(md::ROM& rom, const UiInfo& ui_info)
{
    ByteArray bytes;
    for(const UiString& str : ui_info.strings())
        bytes.add_word(str.position_bytes());

    bytes.add_word(0xFFFF);
    return rom.inject_bytes(bytes);
}

uint32_t inject_selection_mappings(md::ROM& rom, const UiInfo& ui_info)
{
    ByteArray bytes;
    for(const auto& [selection_id, x, y, size] : ui_info.selection_mappings())
    {
        bytes.add_byte(selection_id);
        bytes.add_byte(x);
        bytes.add_byte(y);
        bytes.add_byte(size);
    }

    bytes.add_byte(0xFF);
    return rom.inject_bytes(bytes);
}

//////////////////////////////////////////////////////
///     CODE INJECTION FOR UI ENGINE
//////////////////////////////////////////////////////

uint32_t inject_func_init_gui(md::ROM& rom)
{
    uint32_t gui_tileset_addr = rom.inject_bytes(GUI_TILESET, GUI_TILESET_SIZE);

    md::Code func_init_gui;
    func_init_gui.move_to_sr(0x2700);
    func_init_gui.movew(addrw_(VDP_reg_1_command), reg_D0);
    func_init_gui.andib(0xBF, reg_D0);
    func_init_gui.movew(reg_D0, addr_(VDP_control_port));
    func_init_gui.jsr(Clear_DisplayData);
    func_init_gui.lea(addr_(VDP_control_port), reg_A6);
    func_init_gui.movew(0x8004, addr_(reg_A6));
    func_init_gui.movew(0x8230, addr_(reg_A6));
    func_init_gui.movew(0x8407, addr_(reg_A6));
    func_init_gui.movew(0x8230, addr_(reg_A6));
    func_init_gui.movew(0x8700, addr_(reg_A6));
    func_init_gui.movew(0x8C81, addr_(reg_A6));
    func_init_gui.movew(0x9001, addr_(reg_A6));
    func_init_gui.movew(0x8B00, addr_(reg_A6));

    func_init_gui.clrw(addrw_(DMA_queue));
    func_init_gui.movel(DMA_queue, addrw_(DMA_queue_slot));
    func_init_gui.movel(0x42000000, addr_(VDP_control_port));

    // Decompress GUI tileset
    func_init_gui.lea(addr_(gui_tileset_addr), reg_A0);
    func_init_gui.jsr(Nem_Decomp);

    func_init_gui.rts();

    return rom.inject_code(func_init_gui);
}

uint32_t inject_func_build_text_plane(md::ROM& rom, const UiInfo& ui_info)
{
    uint32_t text_strings_addr = inject_text_strings(rom, ui_info);
    uint32_t text_mappings_addr = inject_text_mappings(rom, ui_info);

    md::Code func_build_text_plane;

    func_build_text_plane.lea(addr_(RAM_start), reg_A3);
    func_build_text_plane.lea(addr_(text_strings_addr), reg_A1);
    func_build_text_plane.lea(addr_(text_mappings_addr), reg_A5);
    func_build_text_plane.moveq(0, reg_D0);

    // Double loop to write each letter of each line of text
    func_build_text_plane.label("loop_write_line");
    func_build_text_plane.movew(addr_postinc_(reg_A5), reg_D3);
    func_build_text_plane.cmpiw(0xFFFF, reg_D3);
    func_build_text_plane.beq("complete");
    func_build_text_plane.lea(addrw_(reg_A3, reg_D3), reg_A2);
    func_build_text_plane.moveq(0, reg_D2);
    func_build_text_plane.moveb(addr_postinc_(reg_A1), reg_D2);
    func_build_text_plane.movew(reg_D2, reg_D3);

    func_build_text_plane.label("loop_write_letter");
    func_build_text_plane.moveb(addr_postinc_(reg_A1), reg_D0);
    func_build_text_plane.movew(reg_D0, addr_postinc_(reg_A2));
    func_build_text_plane.dbra(reg_D2, "loop_write_letter");
    func_build_text_plane.bra("loop_write_line");

    // Send our built plane map to VRAM
    func_build_text_plane.label("complete");
    func_build_text_plane.lea(addr_(RAM_start), reg_A1);
    func_build_text_plane.movel(0x40000003, reg_D0);
    func_build_text_plane.moveq(0x27, reg_D1);
    func_build_text_plane.moveq(0x1B, reg_D2);
    func_build_text_plane.jsr(Plane_Map_To_VRAM);

    func_build_text_plane.rts();

    return rom.inject_code(func_build_text_plane);
}

uint32_t inject_func_handle_gui_controls(md::ROM& rom, const UiInfo& ui_info)
{
    auto HANDLE_BUTTON_PRESS = [](md::Code& func, const std::string& button_name, uint8_t button_bit, uint32_t callback_addr)
    {
        if(callback_addr)
        {
            func.btst(button_bit, reg_D1);
            func.beq(button_name + "_not_pressed");
            func.jsr(callback_addr);
            func.movew(0x9, addrw_(Level_select_repeat));
            func.rts();
        }
        func.label(button_name + "_not_pressed");
    };

    md::Code func_handle_gui_controls;

    func_handle_gui_controls.movew(addrw_(Level_select_repeat), reg_D1);
    func_handle_gui_controls.beq("can_process_inputs");
    func_handle_gui_controls.subqw(1, reg_D1);
    func_handle_gui_controls.movew(reg_D1, addrw_(Level_select_repeat));
    func_handle_gui_controls.rts();

    func_handle_gui_controls.label("can_process_inputs");
    func_handle_gui_controls.moveb(addrw_(Ctrl_1), reg_D1);
    HANDLE_BUTTON_PRESS(func_handle_gui_controls, "up",     0, ui_info.on_up_pressed());
    HANDLE_BUTTON_PRESS(func_handle_gui_controls, "down",   1, ui_info.on_down_pressed());
    HANDLE_BUTTON_PRESS(func_handle_gui_controls, "left",   2, ui_info.on_left_pressed());
    HANDLE_BUTTON_PRESS(func_handle_gui_controls, "right",  3, ui_info.on_right_pressed());
    HANDLE_BUTTON_PRESS(func_handle_gui_controls, "b",      4, ui_info.on_b_pressed());
    HANDLE_BUTTON_PRESS(func_handle_gui_controls, "c",      5, ui_info.on_c_pressed());
    HANDLE_BUTTON_PRESS(func_handle_gui_controls, "a",      6, ui_info.on_a_pressed());
    HANDLE_BUTTON_PRESS(func_handle_gui_controls, "start",  7, ui_info.on_start_pressed());
    func_handle_gui_controls.rts();

    return rom.inject_code(func_handle_gui_controls);
}

uint32_t inject_func_mark_fields(md::ROM& rom, const UiInfo& ui_info)
{
    uint32_t selection_mappings_addr = inject_selection_mappings(rom, ui_info);

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
    func_change_palette.lea(addrw_(reg_A4, reg_D0), reg_A1);

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
    func_mark_fields.lea(addr_(RAM_start), reg_A4);
    func_mark_fields.lea(addr_(selection_mappings_addr), reg_A5);
    func_mark_fields.lea(addr_(VDP_data_port), reg_A6);
    func_mark_fields.moveq(0, reg_D0);
    func_mark_fields.movew(addrw_(Level_select_option), reg_D0);

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

uint32_t inject_func_gui_main_loop(md::ROM& rom, const UiInfo& ui_info)
{
    uint32_t func_handle_gui_controls = inject_func_handle_gui_controls(rom, ui_info);
    uint32_t func_mark_fields = inject_func_mark_fields(rom, ui_info);

    md::Code func_gui_main_loop;
    func_gui_main_loop.label("begin"); // routine running during level select

    // Wait for the frame to be drawn by the VDP before processing another one
    func_gui_main_loop.moveb(0x16, addr_(V_int_routine));
    func_gui_main_loop.jsr(Wait_VSync);
    func_gui_main_loop.move_to_sr(0x2700);

    func_gui_main_loop.moveq(0, reg_D3); // Palette line 0
    func_gui_main_loop.jsr(func_mark_fields); // 0x7F62

    func_gui_main_loop.jsr(func_handle_gui_controls);

    func_gui_main_loop.movew(0x2000, reg_D3); // Palette line 1
    func_gui_main_loop.jsr(func_mark_fields); // 0x7F62

    func_gui_main_loop.move_to_sr(0x2300);
//    func_settings_menu.moveb(addr_(0xF605), reg_D0); // CTRL_1_PRESSED
//    func_settings_menu.orb(addr_(0xF607), reg_D0); // CTRL_2_PRESSED
//    func_settings_menu.andib(0x80, reg_D0);
//    func_settings_menu.bne(LevelSelect_PressStart);
    func_gui_main_loop.bra("begin");

    return rom.inject_code(func_gui_main_loop);

}

uint32_t inject_gui(md::ROM& rom, const UiInfo& ui_info)
{
    uint32_t func_init_gui = inject_func_init_gui(rom);
    uint32_t func_build_text_plane = inject_func_build_text_plane(rom, ui_info);
    uint32_t func_gui_main_loop = inject_func_gui_main_loop(rom, ui_info);

    md::Code func_settings_menu;

    // Call the pre-init function if there is one
    if(ui_info.preinit_function_addr())
        func_settings_menu.jsr(ui_info.preinit_function_addr());

    func_settings_menu.jsr(func_init_gui); // func_settings_menu.jmp(0x7B34);
    func_settings_menu.jsr(func_build_text_plane); // func_settings_menu.jmp(0x7BB2);

    // Setup custom palette
    func_settings_menu.movew(0x0222, addrw_(0xFC00));   // Background color
    func_settings_menu.movew(0x0888, addrw_(0xFC0C));   // Neutral text color
    func_settings_menu.movew(0x0AAA, addrw_(0xFC0E));   // Neutral text color highlight
    func_settings_menu.movew(0x00AF, addrw_(0xFC2C));   // Selected text color
    func_settings_menu.movew(0x04CF, addrw_(0xFC2E));   // Selected text color highlight

//    func_settings_menu.moveb(0x16, addr_(V_int_routine));
//    func_settings_menu.jsr(Wait_VSync);
    func_settings_menu.movew(addr_(VDP_reg_1_command), reg_D0);
    func_settings_menu.orib(0x40, reg_D0);
    func_settings_menu.movew(reg_D0, addr_(VDP_control_port));

    func_settings_menu.jsr(func_gui_main_loop);
    func_settings_menu.rts();

    return rom.inject_code(func_settings_menu);
}
