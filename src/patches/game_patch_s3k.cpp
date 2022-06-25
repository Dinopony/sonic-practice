#include "game_patch_s3k.hpp"


void GamePatchS3K::mark_empty_chunks(md::ROM& rom)
{
    // rom.mark_empty_chunk(0x95590, 0x96000);
    // rom.mark_empty_chunk(0xDE690, 0xDEA00);
    rom.mark_empty_chunk(0x7A74, 0x81AF); // Old level select code & assets
    rom.mark_empty_chunk(0x1FF420, 0x200000);
    // rom.mark_empty_chunk(0x25CFF0, 0x25E000);
    rom.mark_empty_chunk(0x3FFF20, 0x400000);
}

void GamePatchS3K::skip_title_screen_to_level_select(md::ROM& rom)
{
    uint32_t main_menu_addr = inject_main_menu(rom);
    std::cout << "Main Menu is at " << std::hex << main_menu_addr << std::dec << std::endl;

    md::Code proc_boot_ui;
    proc_boot_ui.lea(addr_(main_menu_addr), reg_A4);
    proc_boot_ui.jmp(_engine->func_boot_ui());
    uint32_t boot_ui_addr = rom.inject_code(proc_boot_ui);

    // Make "Sega Screen" gamemode point to the practice mod UI bootup procedure
    rom.set_long(0x4C6, boot_ui_addr);
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

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///     UI STUFF
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

uint32_t GamePatchS3K::inject_func_preinit_s3k(md::ROM& rom)
{
    // constexpr uint32_t Pal_FadeToBlack = 0x3BE4;
    // constexpr uint32_t Pal_FadeFromBlack = 0x3AF0;
    constexpr uint32_t Sprite_table_input = 0xFFFFAC00;
    constexpr uint32_t Object_RAM_Start = 0xFFFFB000;

    md::Code func_s3k_preinit;
    // func_s3k_preinit.jsr(Pal_FadeToBlack);

    // Clear sprite table
    func_s3k_preinit.lea(addrw_(Sprite_table_input), reg_A1);
    func_s3k_preinit.moveq(0x0, reg_D0);
    func_s3k_preinit.movew(0x00FF, reg_D1);
    func_s3k_preinit.label("loop_clear_sprite_table");
    func_s3k_preinit.movel(reg_D0, addr_postinc_(reg_A1));
    func_s3k_preinit.dbra(reg_D1, "loop_clear_sprite_table");

    // Clear object RAM
    func_s3k_preinit.lea(addrw_(Object_RAM_Start), reg_A1);
    func_s3k_preinit.moveq(0, reg_D0);
    func_s3k_preinit.movew(0x7FF, reg_D1);
    func_s3k_preinit.label("loop_clear_object_ram");
    func_s3k_preinit.movel(reg_D0, addr_postinc_(reg_A1));
    func_s3k_preinit.dbra(reg_D1, "loop_clear_object_ram");

    func_s3k_preinit.moveq(0x23, reg_D0);
    func_s3k_preinit.jsr(0x1358); // PlayMusic

    func_s3k_preinit.rts();

    return rom.inject_code(func_s3k_preinit);
}