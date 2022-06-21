#include "vertical_menu.hpp"
#include "../md_tools/md_tools.hpp"

namespace mdui {

uint32_t VerticalMenu::inject(md::ROM& rom)
{
    this->on_down_pressed(this->inject_down_press_handler(rom));
    this->on_up_pressed(this->inject_up_press_handler(rom));

    return Info::inject(rom);
}

uint32_t VerticalMenu::inject_down_press_handler(md::ROM& rom)
{
    // Up pressed: increased selected option by one, looping back to start if needed
    md::Code down_press_handler;

    down_press_handler.movem_to_stack({ reg_D0 }, {});
    down_press_handler.movew(addrw_(this->current_option_ram_addr()), reg_D0);
    down_press_handler.addqw(1, reg_D0);
    down_press_handler.cmpiw(this->max_selection(), reg_D0);
    down_press_handler.ble("commit_down");
    down_press_handler.moveq(0, reg_D0);
    down_press_handler.label("commit_down");
    down_press_handler.movew(reg_D0, addrw_(this->current_option_ram_addr()));
    if(this->on_down_pressed())
        down_press_handler.jsr(this->on_down_pressed());
    down_press_handler.movem_from_stack({ reg_D0 }, {});
    down_press_handler.rts();

    return rom.inject_code(down_press_handler);
}

uint32_t VerticalMenu::inject_up_press_handler(md::ROM& rom)
{
    // Up pressed: decrease selected option by one, looping to the end if needed
    md::Code up_press_handler;

    up_press_handler.movem_to_stack({ reg_D0 }, {});
    up_press_handler.movew(addrw_(this->current_option_ram_addr()), reg_D0);
    up_press_handler.subqw(1, reg_D0);
    up_press_handler.bcc("commit_up");
    up_press_handler.moveq(this->max_selection(), reg_D0);
    up_press_handler.label("commit_up");
    up_press_handler.movew(reg_D0, addrw_(this->current_option_ram_addr()));
    if(this->on_up_pressed())
        up_press_handler.jsr(this->on_up_pressed());
    up_press_handler.movem_from_stack({ reg_D0 }, {});
    up_press_handler.rts();

    return rom.inject_code(up_press_handler);
}

} // namespace mdui