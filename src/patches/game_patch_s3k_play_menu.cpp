#include "game_patch_s3k.hpp"
#include "../ui/vertical_menu.hpp"

static uint32_t inject_zone_id_matching_table(md::ROM& rom)
{
    ByteArray zone_id_matching_table;

    zone_id_matching_table.add_word(0x0000); // AI
    zone_id_matching_table.add_word(0x0100); // HC
    zone_id_matching_table.add_word(0x0200); // MG
    zone_id_matching_table.add_word(0x0300); // CN
    zone_id_matching_table.add_word(0x0500); // IC
    zone_id_matching_table.add_word(0x0600); // LB
    zone_id_matching_table.add_word(0x0700); // MH
    zone_id_matching_table.add_word(0x0400); // FB
    zone_id_matching_table.add_word(0x0800); // SP
    zone_id_matching_table.add_word(0x0900); // LR
    zone_id_matching_table.add_word(0x1601); // HP
    zone_id_matching_table.add_word(0x0A00); // SS
    zone_id_matching_table.add_word(0x0B00); // DE
    zone_id_matching_table.add_word(0x1700); // TD

    return rom.inject_bytes(zone_id_matching_table);
}

uint32_t GamePatchS3K::inject_play_menu_press_a_handler(md::ROM& rom)
{
    uint32_t zone_id_matching_table = inject_zone_id_matching_table(rom);

    md::Code func;
    func.movem_to_stack({ reg_D0 }, { reg_A0, reg_A1 });

    func.lea(addrw_(_engine->option_values_start_ram_addr()), reg_A1);

    // Selected character
    func.moveb(addr_postinc_(reg_A1), addrw_(CHARACTER_PLAYED+1));

    // Emeralds count
    func.clrl(reg_D0);
    func.moveb(addr_postinc_(reg_A1), reg_D0);
    func.moveb(0, addrw_(SUPER_EMERALDS_COUNT));
    func.cmpib(0x7, reg_D0);
    func.ble("no_super_emerald");
    func.subqb(0x7, reg_D0);
    func.moveb(reg_D0, addrw_(SUPER_EMERALDS_COUNT));
    func.label("no_super_emerald");
    func.moveb(reg_D0, addrw_(EMERALDS_COUNT));

    // Shield
    func.moveb(addr_postinc_(reg_A1), reg_D0); // Contains 0x1, 0x2 or 0x3
    func.cmpib(0x3, reg_D0);
    func.bne("not_water_shield");
    func.moveb(0x4, reg_D0); // Set 0x4 instead of 0x3 for water shield, since it will be more convenient for bitshifting afterwards
    func.label("not_water_shield");
    func.lslb(4, reg_D0); // Make 0x1, 0x2 or 0x4 become 0x10, 0x20 or 0x40
//    func.moveb(reg_D0, addrw_(0xFE47));
//    func.moveb(reg_D0, addrw_(0xFE96));
//    func.moveb(reg_D0, addrw_(0xB02B));
//    func.movem_to_stack({ reg_D0 }, { reg_A1 });
//    func.jsr(0x69E0);
//    func.movem_from_stack({ reg_D0 }, { reg_A1 });

    // Selected zone and act
    func.clrl(reg_D0);
    func.moveb(addr_postinc_(reg_A1), reg_D0);  // Selected zone ID --> D0
    func.lea(addr_(zone_id_matching_table), reg_A0);
    func.lslw(1, reg_D0);
    func.adda(reg_D0, reg_A0);
    func.movew(addr_(reg_A0), reg_D0);          // Real in-game zone ID --> D0
    func.addb(addr_postinc_(reg_A1), reg_D0);   // Add act ID to D0
    func.movew(reg_D0, addrw_(CURRENT_ZONE_AND_ACT));
    func.movew(reg_D0, addrw_(APPARENT_ZONE_AND_ACT));
    func.movew(reg_D0, addrw_(SAVED_ZONE_AND_ACT));
    func.movew(reg_D0, addrw_(SAVED_APPARENT_ZONE_AND_ACT));

    func.clrl(reg_D0);
    func.movew(reg_D0, addrw_(RING_COUNT));
    func.movel(reg_D0, addrw_(TIMER));
    func.movel(reg_D0, addrw_(SCORE));
    func.moveb(reg_D0, addrw_(CONTINUE_COUNT));
    func.movew(reg_D0, addrw_(DEMO_MODE_FLAG));
    func.movew(reg_D0, addrw_(COMPETITION_SETTINGS));
    func.movew(reg_D0, addrw_(COMPETITION_MODE));
    func.moveb(reg_D0, addrw_(LEVEL_STARTED_FLAG));
    func.movel(reg_D0, addrw_(CAMERA_X_POS));
    func.movel(reg_D0, addrw_(CAMERA_Y_POS));
    func.movel(reg_D0, addrw_(SAVE_POINTER));
    func.movel(reg_D0, addrw_(COLLECTED_BIG_RINGS_ARRAY));
    func.moveb(reg_D0, addrw_(LAST_STARPOLE_HIT));
    func.movew(reg_D0, addrw_(0xF7F0)); // Anim Counters
    func.moveb(reg_D0, addrw_(0xFE48)); // Special bonus entry flag
    func.moveb(reg_D0, addrw_(0xFFD4)); // Blue spheres stage flag (0 = regular, 1 = standalone BS game)
    func.movew(reg_D0, addrw_(0xFFE4)); // Level_select_cheat_counter
    func.movew(reg_D0, addrw_(0xFFE6)); // Debug_mode_cheat_counter

    func.moveb(0xC, addrw_(GAME_MODE));
    func.moveb(0, addrw_(LIFE_COUNT));
    func.movew(0x1000, addrw_(SAVED_CAMERA_MAX_Y_POS));
    func.movew(0x1000, addrw_(SAVED2_CAMERA_MAX_Y_POS));
    func.movel(5000, addrw_(NEXT_EXTRA_LIFE_SCORE));

    // func.movew(reg_D0, addrw_(RING_COUNT_P2));
    // func.movel(reg_D0, addrw_(TIMER_P2));
    // func.movel(reg_D0, addrw_(SCORE_P2));
    // func.moveb(1, addrw_(LIFE_COUNT_P2));
    // func.movel(5000, addrw_(NEXT_EXTRA_LIFE_SCORE_P2));
    // func.movew(0x707, addrw_(0xF614)); // Demo timer
    // func.movew(reg_D0, addrw_(0xFF02)); // Results screen 2P

    func.jsr(_engine->func_schedule_ui_exit());
    func.movem_from_stack({ reg_D0 }, { reg_A0, reg_A1 });
    func.rts();

    return rom.inject_code(func);
}

uint32_t GamePatchS3K::inject_play_menu(md::ROM& rom)
{
    const std::vector<std::string> CHARACTERS_LIST = {
            "SONIC + TAILS", "SONIC", "TAILS", "KNUCKLES"
    };
    const std::vector<std::string> SHIELDS_LIST = {
            "NONE", "FIRE", "LIGHTNING", "WATER"
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

