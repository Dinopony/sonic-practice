#include "common_gui.h"

// TODO:
// - Mark fields look at first byte to check if selected option == byte
// - Several mappings can concern the same option, to allow for primary / secondaries and even more
//      - Therefore, loop must not stop as soon as we found one
// - Also introduce a "size" to mention the number of consecutive tiles to mark
// TODO:
// - Draw text using a routine that contains "bars of text" with coordinates
//      Format: X,Y,TEXT_UNTIL_FF



// ROM offsets that are only valid in S3K
constexpr uint32_t Pal_FadeToBlack = 0x3BE4;
constexpr uint32_t Pal_FadeFromBlack = 0x3AF0;
constexpr uint32_t LoadPalette = 0x3DBE;
constexpr uint32_t Wait_VSync = 0x1D18;
constexpr uint32_t Plane_Map_To_VRAM = 0x14E6;
constexpr uint32_t Nem_Decomp = 0x15BA;
constexpr uint32_t Clear_DisplayData = 0x11CA;
constexpr uint32_t LEVEL_SELECT_TEXT = 0x80E6;
constexpr uint32_t LEVEL_SELECT_MAPPINGS = 0x80C4;

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
#include "../tools/byte_array.hpp"

const std::vector<std::string> TEXT_ENTRIES = {
        "CHARACTER %%%%%%%%%%%%%%%%%%%%%%%%%%%%",
        "EMERALDS %%%%%%%%%%%%%%%%%%%%%%%%%%%%%",
        "SHIELD %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%",
        "MUSIC %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%",
        "REDUCE BOSSES HITCOUNT %%%%%%%%%%%%%%%",
        "TIMER DURING PAUSE %%%%%%%%%%%%%%%%%%%",
        "______________________________________",
        "ZONE %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%",
        "SPAWN %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%",
        "______________________________________",
        "              * START *               "
};

const std::vector<std::pair<uint8_t, uint8_t>> TEXT_MAPPINGS = {
        { 1, 1 },
        { 1, 3 },
        { 1, 5 },
        { 1, 7 },
        { 1, 9 },
        { 1, 11 },
        { 1, 13 },
        { 1, 15 },
        { 1, 17 },
        { 1, 19 },
        { 1, 21 }
};

ByteArray convert_text_into_bytes(const std::string& string)
{
    const std::vector<char> CONVERSION_TABLE = {
            ' ',
            '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
            '*', '@', ':', '.',
            'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M',
            'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
            '+', '_', '%',
            'a', 'n', 'b', 'w', 'e', 'c', 's', 'd'
    };

    ByteArray output;

    for(char char_in_string : string)
    {
        for(size_t i=0 ; i<CONVERSION_TABLE.size() ; ++i)
        {
            if(CONVERSION_TABLE[i] == char_in_string)
            {
                output.add_byte(i + 0x0F);
                break;
            }
        }
    }

    uint8_t string_length = static_cast<uint8_t>(output.size()-1);
    output.insert(output.begin(), string_length);

    return output;
}

uint32_t inject_text_strings(md::ROM& rom)
{
    ByteArray bytes;
    for(const std::string& str : TEXT_ENTRIES)
    {
        ByteArray string_bytes = convert_text_into_bytes(str);
        bytes.add_bytes(string_bytes);
    }

    bytes.add_byte(0xFF);
    return rom.inject_bytes(bytes);
}

uint32_t inject_text_mappings(md::ROM& rom)
{
    ByteArray bytes;
    for(const auto& [x, y] : TEXT_MAPPINGS)
    {
        uint16_t position_word = (x * 2) + (0x50 * y);
        bytes.add_word(position_word);
    }

    bytes.add_word(0xFFFF);
    return rom.inject_bytes(bytes);
}

uint32_t inject_s3k_preinit(md::ROM& rom)
{
    constexpr uint32_t Sprite_table_input = 0xFFFFAC00;
    constexpr uint32_t Object_RAM_Start = 0xFFFFB000;

    md::Code func_s3k_preinit;

    // Clear sprite table
    func_s3k_preinit.lea(addrw_(Sprite_table_input), reg_A1);
    func_s3k_preinit.moveq(0x0, reg_D0);
    func_s3k_preinit.movew(0x00FF, reg_D1);
    func_s3k_preinit.label("loop1");
    func_s3k_preinit.movel(reg_D0, addr_postinc_(reg_A1));
    func_s3k_preinit.dbra(reg_D1, "loop1");

    // Clear object RAM
    func_s3k_preinit.lea(addrw_(Object_RAM_Start), reg_A1);
    func_s3k_preinit.moveq(0, reg_D0);
    func_s3k_preinit.movew(0x7FF, reg_D1);
    func_s3k_preinit.label("loop_clear_object_ram");
    func_s3k_preinit.movel(reg_D0, addr_postinc_(reg_A1));
    func_s3k_preinit.dbra(reg_D1, "loop_clear_object_ram");

    func_s3k_preinit.clrw(addr_(0xFF08)); // Player Mode
    func_s3k_preinit.clrw(addr_(0xFF02)); // Results screen 2P
    func_s3k_preinit.clrb(addr_(0xF711)); // Level Started Flag
    func_s3k_preinit.clrw(addr_(0xF7F0)); // Anim Counters
    func_s3k_preinit.movew(0x707, addr_(0xF614)); // Demo timer
    func_s3k_preinit.clrw(addr_(0xFFE8)); // Competition mode
    func_s3k_preinit.clrl(addr_(0xEE78)); // Camera X pos
    func_s3k_preinit.clrl(addr_(0xEE7C)); // Camera Y pos
    func_s3k_preinit.clrl(addr_(0xE660)); // Save pointer
    func_s3k_preinit.clrl(addr_(0xFF92)); // Collected special ring array
    func_s3k_preinit.clrb(addr_(0xFE2A)); // Last star post hit
    func_s3k_preinit.clrb(addr_(0xFE48)); // Special bonus entry flag
    func_s3k_preinit.clrb(addr_(0xFFD4)); // Blue spheres stage flag
//    func_s3k_preinit.nop(6);
    func_s3k_preinit.clrw(addr_(0xFFE4)); // Level_select_cheat_counter
    func_s3k_preinit.clrw(addr_(0xFFE6)); // Debug_mode_cheat_counter

    func_s3k_preinit.moveq(0x23, reg_D0);
    func_s3k_preinit.jsr(0x1358); // PlayMusic

    func_s3k_preinit.rts();

    return rom.inject_code(func_s3k_preinit);
}

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

uint32_t inject_func_build_text_plane(md::ROM& rom)
{
    uint32_t text_strings_addr = inject_text_strings(rom);
    uint32_t text_mappings_addr = inject_text_mappings(rom);

    md::Code func_build_text_plane;

    func_build_text_plane.lea(addr_(RAM_start), reg_A3);
    func_build_text_plane.lea(addr_(text_strings_addr), reg_A1);
    func_build_text_plane.lea(addr_(text_mappings_addr), reg_A5);
    func_build_text_plane.moveq(0, reg_D0);

    // A1: pointer on text descriptor list
    // A5: pointer on coordinates

    // Write line loop
    func_build_text_plane.label("loop_write_line");
    func_build_text_plane.movew(addr_postinc_(reg_A5), reg_D3);
    func_build_text_plane.cmpiw(0xFFFF, reg_D3);
    func_build_text_plane.beq("complete");
    func_build_text_plane.lea(addrw_(reg_A3, reg_D3), reg_A2);
    func_build_text_plane.moveq(0, reg_D2);
    func_build_text_plane.moveb(addr_postinc_(reg_A1), reg_D2);
    func_build_text_plane.movew(reg_D2, reg_D3);

    // Write letter loop
    func_build_text_plane.label("loop_write_letter");
    func_build_text_plane.moveb(addr_postinc_(reg_A1), reg_D0);
    func_build_text_plane.movew(reg_D0, addr_postinc_(reg_A2));
    func_build_text_plane.dbra(reg_D2, "loop_write_letter");

//    func_build_text_plane.movew(0xD, reg_D2); // Maximum length of string
//    func_build_text_plane.subw(reg_D3, reg_D2); // Get remaining space in string
//    func_build_text_plane.bcs("string_full");

//    func_build_text_plane.label("blank_loop");
//    func_build_text_plane.movew(0x0, addr_postinc_(reg_A2));
//    func_build_text_plane.dbra(reg_D2, "blank_loop");

//    func_build_text_plane.label("string_full");
//    func_build_text_plane.movew(0x11, addr_(reg_A2)); // #make_art_tile('1',0,0) ---> Write (act) '1'
//    func_build_text_plane.lea(addr_(reg_A2, 0x50), reg_A2); // Next line
//    func_build_text_plane.movew(0x12, addr_(reg_A2)); // #make_art_tile('2',0,0) ---> Write (act) '2'
    func_build_text_plane.bra("loop_write_line");

//    func_build_text_plane.movew(0x0, addr_(reg_A2)); // Get rid of (act) '2' on sound test
//    func_build_text_plane.lea(addr_(reg_A2, 0xFFFFFFB0), reg_A2); // Go back to act 1
//    func_build_text_plane.movew(0x1A, addr_(reg_A2));  // Replace act 1 by *
//    func_build_text_plane.movew(0x13, addr_(0xFFFF018A));  // Make Lava Reef duplicate acts 3 & 4
//    func_build_text_plane.movew(0x14, addr_(0xFFFF01DA));

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

uint32_t inject_func_handle_gui_controls(md::ROM& rom)
{
    md::Code func_handle_gui_controls;
    constexpr uint8_t NUM_OPTIONS = 0x20;

    func_handle_gui_controls.moveb(addrw_(Ctrl_1_pressed), reg_D1);
    func_handle_gui_controls.andib(3, reg_D1);
    func_handle_gui_controls.bne("vertical_direction_pressed"); // Up/Down pressed

    func_handle_gui_controls.subqw(1, addrw_(Level_select_repeat));
    func_handle_gui_controls.bpl("LevSelControls_CheckLR");

    func_handle_gui_controls.label("vertical_direction_pressed");
    func_handle_gui_controls.movew(0x9, addrw_(Level_select_repeat));
    func_handle_gui_controls.moveb(addrw_(Ctrl_1), reg_D1);
    func_handle_gui_controls.andib(3, reg_D1);
    func_handle_gui_controls.beq("LevSelControls_CheckLR"); // up/down not pressed, check for left & right

    func_handle_gui_controls.movew(addrw_(Level_select_option), reg_D0);

    // Test if up is pressed
    func_handle_gui_controls.btst(0, reg_D1);
    func_handle_gui_controls.beq("test_down");

    // UP pressed : decrease selected option by one, looping to the end if needed
    func_handle_gui_controls.subqw(1, reg_D0);
    func_handle_gui_controls.bcc("test_down");
    func_handle_gui_controls.moveq(NUM_OPTIONS, reg_D0);

    // Test if down is pressed
    func_handle_gui_controls.label("test_down");
    func_handle_gui_controls.btst(1, reg_D1);
    func_handle_gui_controls.beq("return");

    // DOWN pressed : increase selected option by one, looping back to the first option if needed
    func_handle_gui_controls.addqw(1, reg_D0); // yes, add 1
    func_handle_gui_controls.cmpiw(NUM_OPTIONS, reg_D0);
    func_handle_gui_controls.ble("return");
    func_handle_gui_controls.moveq(0, reg_D0); // if not, set to 0

    func_handle_gui_controls.label("return");
    func_handle_gui_controls.movew(reg_D0, addrw_(Level_select_option));
    func_handle_gui_controls.label("LevSelControls_CheckLR");
    func_handle_gui_controls.rts();

    return rom.inject_code(func_handle_gui_controls);
}

uint32_t inject_func_mark_fields(md::ROM& rom)
{
    constexpr uint32_t LevSel_MarkTable = 0x8040;

    md::Code func_change_palette;

    // Read info from A3 to compose D1 (VDP control word)
    func_change_palette.moveq(0, reg_D1);
    func_change_palette.moveb(addr_(reg_A3), reg_D1);
    func_change_palette.beq("return");
    func_change_palette.lslw(7, reg_D1);
    func_change_palette.addb(addr_(reg_A3, 0x1), reg_D1);
    func_change_palette.addiw(0xC000, reg_D1); // VRAM_Plane_A_Name_Table
    func_change_palette.lsll(2, reg_D1);
    func_change_palette.lsrw(2, reg_D1);
    func_change_palette.oriw(0x4000, reg_D1); // #vdpComm($0000,VRAM,WRITE) >> 16
    func_change_palette.swap(reg_D1);
    func_change_palette.movel(reg_D1, addr_(VDP_control_port));

    // Read the same info from A3 to deduce A1 (base address where to alter used palette)
    func_change_palette.moveq(0, reg_D0);
    func_change_palette.moveb(addr_(reg_A3), reg_D0);
    func_change_palette.mulu(0x50, reg_D0);
    func_change_palette.moveq(0, reg_D1);
    func_change_palette.moveb(addr_(reg_A3, 0x1), reg_D1);
    func_change_palette.addw(reg_D1, reg_D0);
    func_change_palette.lea(addrw_(reg_A4, reg_D0), reg_A1);

    // Send the tile (A1) with palette ID (D3) on the VDP Data port (A6)
    func_change_palette.label("loop");
    func_change_palette.movew(addr_postinc_(reg_A1), reg_D0);
    func_change_palette.addw(reg_D3, reg_D0);
    func_change_palette.movew(reg_D0, addr_(reg_A6));
    func_change_palette.dbra(reg_D2, "loop");

    func_change_palette.label("return");
    func_change_palette.rts();
    uint32_t func_change_palette_addr = rom.inject_code(func_change_palette);

    ////////////////

    md::Code func_mark_fields;
    func_mark_fields.lea(addr_(RAM_start), reg_A4);
    func_mark_fields.lea(addr_(LevSel_MarkTable), reg_A5);
    func_mark_fields.lea(addr_(VDP_data_port), reg_A6);
    func_mark_fields.moveq(0, reg_D0);
    func_mark_fields.movew(addrw_(Level_select_option), reg_D0);
    func_mark_fields.lslw(2, reg_D0);
    func_mark_fields.lea(addrw_(reg_A5, reg_D0), reg_A3);

    // Change palette for 0xF consecutive tiles, reading info from A3
    func_mark_fields.moveq(0xE, reg_D2);
    func_mark_fields.jsr(func_change_palette_addr);

    func_mark_fields.addqw(2, reg_A3);

    // Change palette for 1 tile, reading info from A3
    func_mark_fields.moveq(0, reg_D2);
    func_mark_fields.jsr(func_change_palette_addr);

    func_mark_fields.rts();

    return rom.inject_code(func_mark_fields);
}

uint32_t inject_func_gui_main_loop(md::ROM& rom)
{
    uint32_t func_handle_gui_controls = inject_func_handle_gui_controls(rom);
    uint32_t func_mark_fields = inject_func_mark_fields(rom);

    md::Code func_gui_main_loop;
    func_gui_main_loop.label("begin"); // routine running during level select
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

uint32_t inject_gui(md::ROM& rom)
{
    uint32_t func_s3k_preinit = inject_s3k_preinit(rom);
    uint32_t func_init_gui = inject_func_init_gui(rom);
    uint32_t func_build_text_plane = inject_func_build_text_plane(rom);
    uint32_t func_gui_main_loop = inject_func_gui_main_loop(rom);

    md::Code func_settings_menu;
//    func_settings_menu.jsr(Pal_FadeToBlack);

    func_settings_menu.jsr(func_s3k_preinit);
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

//    func_settings_menu.jsr(Pal_FadeFromBlack);

    func_settings_menu.jsr(func_gui_main_loop);
    func_settings_menu.rts();

    return rom.inject_code(func_settings_menu);
}
