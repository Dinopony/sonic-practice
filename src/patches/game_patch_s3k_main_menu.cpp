#include "game_patch_s3k.hpp"
#include "../ui/vertical_menu.hpp"

uint32_t GamePatchS3K::inject_main_menu_press_a_handler(md::ROM& rom)
{
    md::Code func;

    func.clrl(reg_D0);
    func.movew(0x101, reg_D0);

    func.andiw(0x3FFF, reg_D0);
    func.movew(reg_D0, addrw_((CURRENT_ZONE_AND_ACT)));
    func.movew(reg_D0, addrw_((APPARENT_ZONE_AND_ACT)));
    func.movew(reg_D0, addrw_((SAVED_ZONE_AND_ACT)));
    func.movew(reg_D0, addrw_((SAVED_APPARENT_ZONE_AND_ACT)));

    func.moveq(0, reg_D0);
    func.movew(reg_D0, addrw_(RING_COUNT));
    func.movel(reg_D0, addrw_(TIMER));
    func.movel(reg_D0, addrw_(SCORE));
    func.movew(reg_D0, addrw_(RING_COUNT_P2));
    func.movel(reg_D0, addrw_(TIMER_P2));
    func.movel(reg_D0, addrw_(SCORE_P2));
    func.moveb(reg_D0, addrw_(CONTINUE_COUNT));
    func.movew(reg_D0, addrw_(DEMO_MODE_FLAG));
    func.movew(reg_D0, addrw_(COMPETITION_SETTINGS));
    func.movew(reg_D0, addrw_(COMPETITION_MODE));

    func.moveb(0xC, addrw_(GAME_MODE));
    func.moveb(1, addrw_(LIFE_COUNT));
    func.moveb(1, addrw_(LIFE_COUNT_P2));   // TODO: Try 0
    func.movew(0x1000, addrw_(SAVED_CAMERA_MAX_Y_POS));
    func.movew(0x1000, addrw_(SAVED2_CAMERA_MAX_Y_POS));
    func.movel(5000, addrw_(NEXT_EXTRA_LIFE_SCORE));
    func.movel(5000, addrw_(NEXT_EXTRA_LIFE_SCORE_P2));

    // Fadeout music
//    func.moveq(0xE1, reg_D0);
//    func.jsr(0x1358); // PlaySfx is 1380

    func.moveb(mdui::Engine::UI_MODE_EXIT, addrw_(_engine->ui_mode_ram_addr()));
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
    main_menu.add_string(1, 1, "       * SONIC 3 AND KNUCKLES *       ");
    main_menu.add_string(1, 3, "             PRACTICE ROM             ");
    main_menu.add_string(1, 4, "______________________________________");
    main_menu.add_selectable_option(1, 6,  "CHARACTER", CHARACTERS_LIST);
    main_menu.add_selectable_option(1, 8,  "EMERALDS", EMERALDS_LIST);
    main_menu.add_selectable_option(1, 10, "SHIELD", SHIELDS_LIST);
    main_menu.add_selectable_option(1, 12, "MUSIC", { "ON", "OFF" });
    main_menu.add_selectable_option(1, 14, "LOWER BOSS HITCOUNT", { "NO", "YES" });
    main_menu.add_selectable_option(1, 16, "TIMER DURING PAUSE", { "OFF", "ON" });
    main_menu.add_selectable_option(1, 20, "ZONE", ZONES_LIST);
    main_menu.add_selectable_option(1, 22, "SPAWN", { "ACT 1", "ACT 2"});
    main_menu.add_string(1, 25, "______________________________________");
    main_menu.add_string(22, 26, "a PLAY    b BACK");

    uint32_t press_a_handler = inject_main_menu_press_a_handler(rom);
    main_menu.on_a_pressed(press_a_handler);

    return main_menu.inject(rom);
}

