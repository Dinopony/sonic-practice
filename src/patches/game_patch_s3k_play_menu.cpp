#include "game_patch_s3k.hpp"
#include "../ui/vertical_menu.hpp"

uint32_t GamePatchS3K::inject_play_menu_press_a_handler(md::ROM& rom)
{
    md::Code func;

    func.clrl(reg_D0);
    func.movew(0x101, reg_D0);

    func.andiw(0x3FFF, reg_D0);
    func.movew(reg_D0, addrw_(CURRENT_ZONE_AND_ACT));
    func.movew(reg_D0, addrw_(APPARENT_ZONE_AND_ACT));
    func.movew(reg_D0, addrw_(SAVED_ZONE_AND_ACT));
    func.movew(reg_D0, addrw_(SAVED_APPARENT_ZONE_AND_ACT));

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

    func.jsr(_engine->func_schedule_ui_exit());
    func.rts();

    return rom.inject_code(func);
}

uint32_t GamePatchS3K::inject_play_menu(md::ROM& rom)
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

    mdui::VerticalMenu play_menu(rom, *_engine);
    play_menu.add_string(13, 1, "* PLAY LEVEL *");
    play_menu.add_string(0, 2, "________________________________________");
    play_menu.add_selectable_option(1, 4,  "CHARACTER", CHARACTERS_LIST);
    play_menu.add_selectable_option(1, 6,  "EMERALDS", EMERALDS_LIST);
    play_menu.add_selectable_option(1, 8, "SHIELD", SHIELDS_LIST);
//    play_menu.add_selectable_option(1, 12, "MUSIC", { "ON", "OFF" });
//    play_menu.add_selectable_option(1, 14, "LOWER BOSS HITCOUNT", { "NO", "YES" });
//    play_menu.add_selectable_option(1, 16, "TIMER DURING PAUSE", { "OFF", "ON" });
    play_menu.add_selectable_option(1, 20, "ZONE", ZONES_LIST);
    play_menu.add_selectable_option(1, 22, "SPAWN", { "ACT 1", "ACT 2"});
    play_menu.add_string(0, 25, "________________________________________");
    play_menu.add_string(20, 26, "a/c PLAY   b BACK");

    uint32_t press_a_handler = inject_play_menu_press_a_handler(rom);
    play_menu.on_a_pressed(press_a_handler);

    return play_menu.inject(rom);
}

