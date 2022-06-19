#pragma once

#include "../md_tools/md_tools.hpp"
#include <string>

class GamePatch {
public:
    GamePatch() = default;

    virtual std::string game_code() = 0;
    virtual std::string game_name() = 0;
    virtual std::string rom_default_filename() = 0;

    /// Marks chunks in the ROM containing empty data that can be erased in order to inject new data/code if needed
    virtual void mark_empty_chunks(md::ROM& rom) = 0;

    /// Makes the game load straight up in level select instead of displaying the SEGA logo and title screen
    virtual void skip_title_screen_to_level_select(md::ROM& rom) = 0;

    /// Removes the "remove 1 life on death" routine, effectively removing any game over possibility
    virtual void give_infinite_lives(md::ROM& rom) = 0;

    /// Add support for in-game savestates, using SRAM to dump & restore game state at any given point
    virtual void add_savestate_handling(md::ROM& rom) = 0;

    /// Makes the game display additionnal information when paused (such as position, last spindash speed...)
    virtual void display_additionnal_hud_on_pause(md::ROM& rom) = 0;

    /// Overridable function containing patches that are specific to this game and would not make sense in any other game
    virtual void game_specific_patches(md::ROM& rom) {}

    void patch_rom(md::ROM& rom)
    {
        this->mark_empty_chunks(rom);

        this->skip_title_screen_to_level_select(rom);
        this->give_infinite_lives(rom);
        this->add_savestate_handling(rom);
        this->display_additionnal_hud_on_pause(rom);

        this->game_specific_patches(rom);
    }
};


