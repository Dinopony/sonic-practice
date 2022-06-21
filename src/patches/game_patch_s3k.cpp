#include "game_patch_s3k.hpp"
#include "../ui/engine.hpp"
#include "../ui/vertical_menu.hpp"
#include "../ui/text.hpp"

void GamePatchS3K::mark_empty_chunks(md::ROM& rom)
{
    // rom.mark_empty_chunk(0x95590, 0x96000);
    // rom.mark_empty_chunk(0xDE690, 0xDEA00);
    rom.mark_empty_chunk(0x1FF420, 0x200000);
    // rom.mark_empty_chunk(0x25CFF0, 0x25E000);
    rom.mark_empty_chunk(0x3FFF20, 0x400000);
}

void GamePatchS3K::skip_title_screen_to_level_select(md::ROM& rom)
{
    // Make "Sega Screen" gamemode point to the address of level select function
    rom.set_long(0x4C6, 0x00007A74);
}

void GamePatchS3K::give_infinite_lives(md::ROM& rom)
{
    // Replace life subtraction on death by two NOP instructions
    rom.set_code(0x12448, md::Code().nop(2));
}

void GamePatchS3K::add_savestate_handling(md::ROM& rom)
{
    // TODO
}

void GamePatchS3K::display_additionnal_hud_on_pause(md::ROM& rom)
{
    // TODO: Display subpixels
    // TODO: Display last spindash speed
}

static uint32_t inject_func_preinit_s3k(md::ROM& rom)
{
    constexpr uint32_t Sprite_table_input = 0xFFFFAC00;
    constexpr uint32_t Object_RAM_Start = 0xFFFFB000;

    md::Code func_s3k_preinit;
    // func_s3k_preinit.jsr(Pal_FadeToBlack);

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

    func_s3k_preinit.clrw(addr_(0xFFE4)); // Level_select_cheat_counter
    func_s3k_preinit.clrw(addr_(0xFFE6)); // Debug_mode_cheat_counter

    func_s3k_preinit.moveq(0x23, reg_D0);
    func_s3k_preinit.jsr(0x1358); // PlayMusic

    func_s3k_preinit.rts();

    return rom.inject_code(func_s3k_preinit);
}

void GamePatchS3K::add_settings_menu(md::ROM& rom)
{
    std::vector<mdui::Text> strings = {
            mdui::Text("CHARACTER %%%%%%%%%%%%%%%%%%%%%%%%%%%%", {1,1}    ),
            mdui::Text("EMERALDS %%%%%%%%%%%%%%%%%%%%%%%%%%%%%", {1,3}    ),
            mdui::Text("SHIELD %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%", {1,5}    ),
            mdui::Text("MUSIC %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%", {1,7}    ),
            mdui::Text("LOWER BOSS HITCOUNT %%%%%%%%%%%%%%%%%%", {1,9}    ),
            mdui::Text("TIMER DURING PAUSE %%%%%%%%%%%%%%%%%%%", {1,11}   ),
            mdui::Text("ZONE %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%", {1,15}   ),
            mdui::Text("SPAWN %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%", {1,17}   ),
            mdui::Text("______________________________________", {1,25}   ),
            mdui::Text("                     a PLAY    b BACK ", {1,26}   )
    };

    std::vector<mdui::SelectionMapping> selection_mappings = {
            { 0, 1, 1,  38 },
            { 1, 1, 3,  38 },
            { 2, 1, 5,  38 },
            { 3, 1, 7,  38 },
            { 4, 1, 9,  38 },
            { 5, 1, 11, 38 },
            { 6, 1, 15, 38 },
            { 7, 1, 17, 38 }
    };

    /*
    const std::vector<std::vector<std::string>> SELECTION_OPTIONS = {
        { "SONIC AND TAILS", "SONIC", "TAILS", "KNUCKLES" },
        { "0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "11", "12", "13", "14" },
        { "NONE", "LIGHTNING", "FIRE", "WATER" },
        { "ENABLED", "DISABLED" },
        { "YES", "NO" },
        { "ENABLED", "DISABLED" },
        { "ANGEL ISLAND", "HYDROCITY", "MARBLE GARDEN", "CARNIVAL NIGHT", "ICECAP", "LAUNCH BASE", "MUSHROOM HILL",
          "FLYING BATTERY", "SANDOPOLIS", "LAVA REEF", "HIDDEN PALACE", "SKY SANCTUARY", "DEATH EGG", "THE DOOMSDAY" },
    };

    constexpr uint32_t Pal_FadeToBlack = 0x3BE4;
    constexpr uint32_t Pal_FadeFromBlack = 0x3AF0;

    // Can store property values in $F664-$F67F
     */
    constexpr uint32_t Level_select_option = 0xFFFFFF82;

    mdui::VerticalMenu settings_ui(strings, selection_mappings);
    settings_ui.preinit_function_addr(inject_func_preinit_s3k(rom));
    settings_ui.current_option_ram_addr(Level_select_option);

    uint32_t gui_info_addr = settings_ui.inject(rom);

    mdui::Engine ui_engine(0xFFFFCFCC, 0xFFFFFF80);
    uint32_t func_boot_gui_addr = ui_engine.inject(rom);

    md::Code proc_launch_gui;
    proc_launch_gui.lea(addr_(gui_info_addr), reg_A4);
    proc_launch_gui.jmp(func_boot_gui_addr);
    rom.set_long(0x4C6, rom.inject_code(proc_launch_gui));
}