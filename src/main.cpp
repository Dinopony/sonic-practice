//////////////////////////////////////////////////////////////////////////////////////////
//
//     SONIC PRACTICE ROM GENERATOR
//
// ---------------------------------------------------------------------------------------
//
//     Developed by: Dinopony (@DinoponyRuns)
//
// ---------------------------------------------------------------------------------------
//
//     Thanks to the whole Classic Sonic Speedrunning community for being supportive and helpful
// 
//////////////////////////////////////////////////////////////////////////////////////////

#include <iostream>
#include <vector>

#include "md_tools/md_tools.hpp"
#include "tools/argument_dictionary.hpp"
#include "patches/game_patch_s3k.hpp"

int pause_and_exit(const ArgumentDictionary& args, int exit_code)
{
    if(args.get_boolean("pause", true))
    {
        std::cout << "\nPress any key to exit.";
        std::string dummy;
        std::getline(std::cin, dummy);
    }

    return exit_code;
}

int main(int argc, char* argv[])
{
    const std::vector<GamePatch*> GAME_PATCHES = {
            new GamePatchS3K()
    };

    ArgumentDictionary args(argc, argv);

    std::cout << "======== Sonic Practice ROM builder v" << RELEASE << " ========\n\n";

    std::string selected_game_code = args.get_string("game", "");
    GamePatch* selected_game_patch = nullptr;
    for(GamePatch* game_patch : GAME_PATCHES)
    {
        if(game_patch->game_code() == selected_game_code)
        {
            selected_game_patch = game_patch;
            break;
        }
    }

    if(!selected_game_patch)
    {
        std::cerr << "Please specify a valid game code using '--game=<CODE>'.\nValid game codes are:\n";
        for(GamePatch* game_patch : GAME_PATCHES)
            std::cerr << "\t- " << game_patch->game_code() << ": " << game_patch->game_name() << std::endl;
        return pause_and_exit(args, EXIT_FAILURE);
    }

    std::string input_rom_path = "./" + selected_game_patch->rom_default_filename();
    md::ROM rom(input_rom_path);
    if(!rom.is_valid())
    {
        std::cerr << "Could not open ROM at path '" << input_rom_path << "'." << std::endl;
        return pause_and_exit(args, EXIT_FAILURE);
    }

    std::cout << "Patching ROM at '" << input_rom_path << "' as '" << selected_game_code << "'..." << std::endl;
    selected_game_patch->patch_rom(rom);

    std::string output_rom_path = "./" + selected_game_patch->game_code() + "_practice.md";
    std::ofstream output_rom_file(output_rom_path, std::ios::binary);
    if(!output_rom_file)
    {
        std::cerr << "Could not write ROM at path '" << output_rom_path << "'." << std::endl;
        return pause_and_exit(args, EXIT_FAILURE);
    }

    rom.write_to_file(output_rom_file);
    std::cout << "Done." << std::endl;

    return pause_and_exit(args, EXIT_SUCCESS);
}
