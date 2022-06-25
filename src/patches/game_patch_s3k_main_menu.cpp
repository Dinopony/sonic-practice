#include "game_patch_s3k.hpp"
#include "../ui/vertical_menu.hpp"

uint32_t GamePatchS3K::inject_main_menu_press_a_handler(md::ROM& rom)
{
    uint32_t play_menu_addr = inject_play_menu(rom);
    std::cout << "Play Menu is at " << std::hex << play_menu_addr << std::dec << std::endl;

    md::Code func;
    func.movem_to_stack({}, { reg_A0 });

    func.lea(addr_(play_menu_addr), reg_A0);
    func.jsr(_engine->func_schedule_ui_change());

    func.movem_from_stack({}, { reg_A0 });
    func.rts();

    return rom.inject_code(func);
}

uint32_t GamePatchS3K::inject_main_menu(md::ROM& rom)
{
    const std::vector<std::string> CHARACTERS_LIST = {
            "SONIC + TAILS", "SONIC", "TAILS", "KNUCKLES"
    };
    const std::vector<std::string> SHIELDS_LIST = {
            "NONE", "LIGHTNING", "FIRE", "WATER"
    };
    const std::vector<std::string> EMERALDS_LIST = {
            "0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "11", "12", "13", "14"
    };
    const std::vector<std::string> ZONES_LIST = {
            "ANGEL ISLAND", "HYDROCITY", "MARBLE GARDEN", "CARNIVAL NIGHT", "ICECAP", "LAUNCH BASE", "MUSHROOM HILL",
            "FLYING BATTERY", "SANDOPOLIS", "LAVA REEF", "HIDDEN PALACE", "SKY SANCTUARY", "DEATH EGG", "THE DOOMSDAY"
    };

    mdui::VerticalMenu main_menu(rom, *_engine);
    main_menu.add_string(1, 2, "     *** SONIC 3 AND KNUCKLES ***     ");
    main_menu.add_string(1, 4, "             PRACTICE ROM             ");
    main_menu.add_string(0, 6, "________________________________________");
    main_menu.add_selectable_option(1, 9,  "r PLAY LEVEL");
    main_menu.add_selectable_option(1, 12, "r PLAY BLUE SPHERES");
    main_menu.add_selectable_option(1, 15, "r SETTINGS");
    main_menu.add_selectable_option(1, 18, "r HELP AND SHORTCUTS");
    main_menu.add_string(0, 24, "________________________________________");
    main_menu.add_string(27, 26, "a/c CONFIRM");

    uint32_t press_a_handler = inject_main_menu_press_a_handler(rom);
    main_menu.on_a_pressed(press_a_handler);

    return main_menu.inject(rom);
}

