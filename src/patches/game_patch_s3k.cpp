#include "game_patch_s3k.hpp"
#include "common_gui.h"

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

void GamePatchS3K::add_settings_menu(md::ROM& rom)
{
    const std::vector<std::string> SETTINGS_ENTRIES = {
        "CHARACTER",
        "EMERALDS",
        "SHIELD",
        "MUSIC",
        "REDUCE BOSSES HITCOUNT",
        "TIMER DURING PAUSE"
    };

    const std::vector<std::vector<std::string>> SETTINGS_OPTIONS = {
        { "SONIC AND TAILS", "SONIC", "TAILS", "KNUCKLES" },
        { "0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "11", "12", "13", "14" },
        { "NONE", "LIGHTNING", "FIRE", "WATER" },
        { "ENABLED", "DISABLED" },
        { "YES", "NO" },
        { "ENABLED", "DISABLED" },
    };

    rom.set_long(0x4C6, inject_gui(rom));

    // Can store property values in $F664-$F67F
}