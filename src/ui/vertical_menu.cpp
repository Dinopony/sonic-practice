#include "vertical_menu.hpp"
#include "engine.hpp"

namespace mdui {

constexpr uint32_t RAM_start = 0xFFFF0000;
constexpr uint32_t VDP_data_port = 0xC00000;
constexpr uint32_t VDP_control_port = 0xC00004;

uint32_t VerticalMenu::inject_down_press_handler(md::ROM& rom, Engine& engine) const
{
    md::Code func;
    func.movem_to_stack({ reg_D0 }, {});

    func.clrl(reg_D0);
    func.moveb(addrw_(engine.current_option_ram_addr()), reg_D0);
    func.addqb(1, reg_D0);
    func.cmpb(addr_(reg_A4, Info::LAST_OPTION_ID_OFFSET), reg_D0);
    func.ble("no_overflow");
    func.moveq(0, reg_D0); // In case of an overflow, loop back to option 0
    func.label("no_overflow");
    func.jsr(engine.func_set_selected_option());

    func.movem_from_stack({ reg_D0 }, {});
    func.rts();

    return rom.inject_code(func);
}

uint32_t VerticalMenu::inject_up_press_handler(md::ROM& rom, Engine& engine) const
{
    md::Code func;
    func.movem_to_stack({ reg_D0 }, {});

    func.clrl(reg_D0);
    func.moveb(addrw_(engine.current_option_ram_addr()), reg_D0);
    func.subqb(1, reg_D0);
    func.bcc("no_underflow");
    func.moveb(addr_(reg_A4, Info::LAST_OPTION_ID_OFFSET), reg_D0); // In case of an underflow, loop back to last option
    func.label("no_underflow");
    func.jsr(engine.func_set_selected_option());

    func.movem_from_stack({ reg_D0 }, {});
    func.rts();

    return rom.inject_code(func);
}



} // namespace mdui