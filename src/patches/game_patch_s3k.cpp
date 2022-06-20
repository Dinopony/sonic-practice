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
    std::vector<UiString> strings = {
            UiString("CHARACTER %%%%%%%%%%%%%%%%%%%%%%%%%%%%", {1,1}    ),
            UiString("EMERALDS %%%%%%%%%%%%%%%%%%%%%%%%%%%%%", {1,3}    ),
            UiString("SHIELD %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%", {1,5}    ),
            UiString("MUSIC %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%", {1,7}    ),
            UiString("LOWER BOSS HITCOUNT %%%%%%%%%%%%%%%%%%", {1,9}    ),
            UiString("TIMER DURING PAUSE %%%%%%%%%%%%%%%%%%%", {1,11}   ),
            UiString("______________________________________", {1,13}   ),
            UiString("ZONE %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%", {1,15}   ),
            UiString("SPAWN %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%", {1,17}   ),
            UiString("______________________________________", {1,19}   ),
            UiString("              * START *               ", {1,21}   )
    };

    std::vector<UiSelectionMapping> selection_mappings = {
            { 0, 1, 1,  38 },
            { 1, 1, 3,  38 },
            { 2, 1, 5,  38 },
            { 3, 1, 7,  38 },
            { 4, 1, 9,  38 },
            { 5, 1, 11, 38 },
            { 6, 1, 15, 38 },
            { 7, 1, 17, 38 },
            { 8, 1, 21, 38 }
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
     */
    UiInfo settings_ui(strings, selection_mappings);

    rom.set_long(0x4C6, inject_gui(rom, settings_ui));

    // Can store property values in $F664-$F67F
}