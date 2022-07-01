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

    // Get the current selection, store it in D0 and add 1 to it
    func.clrl(reg_D0);
    func.moveb(addrw_(engine.current_selectable_addr()), reg_D0);
    func.addqb(1, reg_D0);

    // In case of an overflow, loop back to option 0
    func.movel(addr_(reg_A4, Info::SELECTABLES_OFFSET), reg_A1); // A1 now points on a word containing the number of Selectables in the UI
    func.cmpw(addr_(reg_A1), reg_D0);
    func.ble("no_overflow");
    {
        func.moveq(0, reg_D0);
    }
    func.label("no_overflow");

    // Set the current selection to D0, and update the layout accordingly
    func.jsr(engine.func_set_current_selectable());

    func.movem_from_stack({ reg_D0 }, {});
    func.rts();

    return rom.inject_code(func);
}

uint32_t VerticalMenu::inject_up_press_handler(md::ROM& rom, Engine& engine) const
{
    md::Code func;
    func.movem_to_stack({ reg_D0, reg_D2 }, {});

    func.clrl(reg_D0);
    func.moveb(addrw_(engine.current_selectable_addr()), reg_D0);
    func.subqb(1, reg_D0);
    func.bcc("no_underflow");
    {
        // In case of an underflow, loop back to last option
        func.movel(addr_(reg_A4, Info::SELECTABLES_OFFSET), reg_A1); // A1 now points on a word containing the number of Selectables in the UI
        func.movew(addr_(reg_A1), reg_D0);
    }
    func.label("no_underflow");
    func.jsr(engine.func_set_current_selectable());

    func.movem_from_stack({ reg_D0, reg_D2 }, {});
    func.rts();

    return rom.inject_code(func);
}

uint32_t VerticalMenu::inject_left_press_handler(md::ROM& rom, Engine& engine) const
{
    md::Code func;
    func.movem_to_stack({ reg_D0, reg_D1, reg_D2 }, {});

    // Get the value for the currently selected option and store it in D1
    func.clrl(reg_D0);
    func.moveb(addrw_(engine.current_selectable_addr()), reg_D0);
    func.jsr(engine.func_get_selectable_value()); // Current value --> D1

    // Store the number of distinct values for that Selectable inside D2
    func.jsr(engine.func_get_selectable_maximum_value());
    func.cmpib(0xFF, reg_D2);
    func.beq("no_values");
    {
        // Subtract the value by one
        func.subqb(1, reg_D1);
        func.bcc("no_underflow");
        {
            // In case of an underflow, loop back to last value
            func.moveb(reg_D2, reg_D1);
        }
        func.label("no_underflow");

        // Store this value as the new one for current option
        func.jsr(engine.func_set_option_value());
    }
    func.label("no_values");

    func.movem_from_stack({ reg_D0, reg_D1, reg_D2 }, {});
    func.rts();

    return rom.inject_code(func);
}

uint32_t VerticalMenu::inject_right_press_handler(md::ROM& rom, Engine& engine) const
{
    md::Code func;
    func.movem_to_stack({ reg_D0, reg_D1, reg_D2 }, {});

    // Get the value for the currently selected option and store it in D1
    func.clrl(reg_D0);
    func.moveb(addrw_(engine.current_selectable_addr()), reg_D0);
    func.jsr(engine.func_get_selectable_value());           // Current value --> D1

    // Store the number of distinct values for that Selectable inside D2
    func.jsr(engine.func_get_selectable_maximum_value());
    func.cmpib(0xFF, reg_D2);
    func.beq("no_values");
    {
        // Add one to the value
        func.addqb(1, reg_D1);

        // In case of an overflow, loop back to first option
        func.cmpb(reg_D2, reg_D1);
        func.ble("no_overflow");
        {
            func.moveb(0, reg_D1);
        }
        func.label("no_overflow");

        // Store this value as the new one for current option
        func.jsr(engine.func_set_option_value());
    }
    func.label("no_values");

    func.movem_from_stack({ reg_D0, reg_D1, reg_D2 }, {});
    func.rts();

    return rom.inject_code(func);
}

} // namespace mdui