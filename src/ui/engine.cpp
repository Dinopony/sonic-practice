#include "engine.hpp"
#include "info.hpp"
#include "../../assets/nem_decomp.bin.hxx"

namespace mdui {

// TODO: rename "option" by "selectable" pretty much everywhere
// TODO: Add padding byte removal to erase_text() to remove the trailing alignment character
// TODO: Crash on left / right on value-less options

// TODO: change start icon in tileset + add a slash character (+ underline?)
// TODO: allow moving the plane map (changes references to RAM_Start)
// TODO: on init, load options from SRAM
// TODO: add some kind of checkered background sent as a one-shot on init (use color 0 from palettes for that?)

// Global RAM offsets that are always valid
constexpr uint32_t RAM_start = 0xFFFF0000;
constexpr uint32_t VDP_data_port = 0xC00000;
constexpr uint32_t VDP_control_port = 0xC00004;
constexpr uint32_t Controller_1_data_port = 0xA10003;
// constexpr uint32_t Z80_bus_request = 0xA11100;

#include "../../assets/gui_tileset.bin.hxx"

///////////////////////////////////////////////////////////////////////////////////////////////////////////
///        OPTIONS HANDLING
///////////////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * Find the address of the given Selectable structure in the ROM
 *
 * Input:
 *      - D0.b: selectable ID
 *      - A4: pointer on the UI descriptor structure
 * Output:
 *      - A1: pointer on the Selectable structure
 */
uint32_t Engine::func_get_selectable_addr()
{
    if(_func_get_selectable_addr)
        return _func_get_selectable_addr;
    // ------------------------------------------------------------------------------------------

    md::Code func;
    func.movem_to_stack({ reg_D0 }, {});

    // Find the address of the first value text for selectable D0, and store it in A1
    func.movel(addr_(reg_A4, Info::SELECTABLES_OFFSET), reg_A1);
    func.addql(0x2, reg_A1); // Skip array size word

    // Add the appropriate offset to get address to Selectable #D0
    func.andil(0x000000FF, reg_D0);
    func.lslw(2, reg_D0);
    func.adda(reg_D0, reg_A1);

    // Resolve the address and store it in A1
    func.movel(addr_(reg_A1), reg_A1);

    func.movem_from_stack({ reg_D0 }, {});
    func.rts();

    // ------------------------------------------------------------------------------------------
    _func_get_selectable_addr = _rom.inject_code(func);
    return _func_get_selectable_addr;
}

/**
 * Find the address of the given value text for the given option.
 *
 * Input:
 *      - D0.b: option ID
 *      - D1.b: value ID for option
 * Output:
 *      - A1: address pointing on the text (or 0x0 if text does not exist)
 */
uint32_t Engine::func_get_selectable_value_text_addr()
{
    if(_func_get_selectable_value_text_addr)
        return _func_get_selectable_value_text_addr;
    // ------------------------------------------------------------------------------------------

    md::Code func;
    func.movem_to_stack({ reg_D0,reg_D1 }, {});

    func.jsr(func_get_selectable_addr()); // Selectable addr --> A1

    // If we have no values for this selectable, just return a zero address
    func.cmpib(0xFF, addr_(reg_A1, Selectable::VALUES_COUNT_OFFSET));
    func.bne("has_values");
    {
        func.movel(0x0, reg_A1);
        func.bra("return");
    }
    func.label("has_values");
    func.adda(Selectable::VALUE_STRINGS_OFFSET, reg_A1);

    // Skip D1 texts to reach the requested one
    func.label("loop_skip_texts");
    {
        // If D1 reached 0, it means we already skipped all the strings we needed to skip
        func.tstb(reg_D1);
        func.beq("return");

        func.addql(0x2, reg_A1);                    // skip position bytes
        func.moveb(addr_postinc_(reg_A1), reg_D0);  // string size - 1 --> D0
        func.addqb(0x1, reg_D0);
        func.adda(reg_D0, reg_A1);                  // skip as many bytes as the string contains

        // We want all text structures to begin on an even address because reading the string position word from an odd address is illegal.
        // This is why, at injection time, we add a padding byte (an easily recognizable 0xFE) when needed to make sure next string starts at a valid address.
        // Here, we check if we encounter this padding byte, and ignore it if that's the case.
        func.cmpib(0xFE, addr_(reg_A1));
        func.bne("no_padding_byte");
        func.addql(0x1, reg_A1);
        func.label("no_padding_byte");
    }
    func.subqb(0x1, reg_D1);
    func.bra("loop_skip_texts");

    func.label("return");
    func.movem_from_stack({ reg_D0,reg_D1 }, {});
    func.rts();

    // ------------------------------------------------------------------------------------------
    _func_get_selectable_value_text_addr = _rom.inject_code(func);
    return _func_get_selectable_value_text_addr;
}

/**
 * Draw the current value text for all options (typically used on UI boot)
 */
uint32_t Engine::func_draw_all_selectable_values()
{
    if(_func_draw_all_selectable_values)
        return _func_draw_all_selectable_values;
    // ------------------------------------------------------------------------------------------

    md::Code func;
    func.movem_to_stack({ reg_D0, reg_D1 }, { reg_A1, reg_A2 });

    // Put the last option ID in D0
    func.movel(addr_(reg_A4, Info::SELECTABLES_OFFSET), reg_A1);
    func.movew(addr_(reg_A1), reg_D0);

    func.label("loop");
    {
        // Store the currently selected value for Selectable inside D1
        func.jsr(func_get_selectable_value());
        // For this Selectable, store the address of the currently selected value Text inside A1
        func.jsr(func_get_selectable_value_text_addr());
        func.cmpa(0x0, reg_A1);
        func.beq("null_address");
        {
            // If we get a non-null address, we can draw the selectable value pointed by A1
            func.jsr(func_draw_alignment_helper_line());
            func.jsr(func_draw_text());
        }
        func.label("null_address");
    }
    func.dbra(reg_D0, "loop"); /// @TestMe: Doesn't this loop skip the first Selectable?

    func.label("return");
    /// @Polish: What is that?
//    func.movew(0xFFFF, addr_postinc_(reg_A6));
    func.movem_from_stack({ reg_D0, reg_D1 }, { reg_A1,reg_A2 }); /// @Polish: is A2 necessary here?
    func.rts();

    // ------------------------------------------------------------------------------------------
    _func_draw_all_selectable_values = _rom.inject_code(func);
    return _func_draw_all_selectable_values;
}

/**
 * Get the current value for a given Selectable
 *
 * Input:
 *      - D0.b: the selectable ID
 *      - A4: pointer on the UI descriptor structure
 * Output:
 *      - D1.b: the currently selected value for that option (or 0 if option has no values)
 */
uint32_t Engine::func_get_selectable_value()
{
    if(_func_get_selectable_value)
        return _func_get_selectable_value;
    // ------------------------------------------------------------------------------------------

    md::Code func;
    func.movem_to_stack({}, { reg_A0, reg_A1 });

    // Get the address of Selectable #D0, and store it in A1
    func.jsr(func_get_selectable_addr());

    // Get the address where current value is stored in RAM for this Selectable, and store it in A0
    func.movel(0xFFFF0000, reg_A0);
    func.movew(addr_(reg_A1, Selectable::VALUE_STORAGE_ADDRESS_OFFSET), reg_A0);
    func.clrl(reg_D1);
    func.cmpa(0xFFFFFFFF, reg_A0);
    func.beq("no_value");
    {
        // Fetch the current value ID for this option, and store it in D1
        func.moveb(addr_(reg_A0), reg_D1);
    }
    func.label("no_value");
    func.movem_from_stack({}, { reg_A0, reg_A1 });
    func.rts();

    // ------------------------------------------------------------------------------------------
    _func_get_selectable_value = _rom.inject_code(func);
    return _func_get_selectable_value;
}

/**
 * Set the value for a given option, updating all graphics accordingly
 *
 * Input:
 *      - D0.b: the option ID
 *      - D1.b: the new value for that option
 *      - A4: pointer on the UI descriptor
 */
uint32_t Engine::func_set_option_value()
{
    if(_func_set_option_value)
        return _func_set_option_value;
    // ------------------------------------------------------------------------------------------

    md::Code func;
    func.movem_to_stack({}, { reg_A1 });

    // Get the address of Selectable #D0, and store it in A1
    func.jsr(func_get_selectable_addr());

    // Get the address where current value is stored in RAM for this Selectable, and store it in A0
    func.movel(0xFFFF0000, reg_A0);
    func.movew(addr_(reg_A1, Selectable::VALUE_STORAGE_ADDRESS_OFFSET), reg_A0);

    // Erase the old value text
    func.moveb(reg_D1, reg_D2);
    func.moveb(addr_(reg_A0), reg_D1);               // Put current value ID in D1
    func.jsr(func_get_selectable_value_text_addr()); // Address for current value's text --> A1
    func.cmpa(0x0, reg_A1);
    func.beq("has_no_values");
    {
        func.jsr(func_erase_text());

        // Set the new value, and draw its text
        func.moveb(reg_D2, reg_D1);
        func.moveb(reg_D1, addr_(reg_A0));               // Put new value ID in D1
        func.jsr(func_get_selectable_value_text_addr()); // Address for new value's text --> A1
        func.jsr(func_draw_text());

        func.movem_from_stack({}, { reg_A1 });
    }
    func.label("has_no_values");
    func.rts();

    // ------------------------------------------------------------------------------------------
    _func_set_option_value = _rom.inject_code(func);
    return _func_set_option_value;
}

/**
 * Read the maximum value for a given option.
 *
 * Input:
 *      - D0.b: the Selectable ID to get the maximum value for
 *      - A4: pointer on the UI descriptor
 * Output:
 *      - D2.b: the maximum value for this Selectable
 */
uint32_t Engine::func_get_selectable_maximum_value()
{
    if(_func_get_selectable_maximum_value)
        return _func_get_selectable_maximum_value;
    // ------------------------------------------------------------------------------------------

    md::Code func;
    func.movem_to_stack({}, { reg_A1 });

    // Get the address for Selectable #D0, and store it in A1
    func.jsr(func_get_selectable_addr());

    // Read the appropriate offset to get the values count for this Selectable
    func.lea(addr_(reg_A1, Selectable::VALUES_COUNT_OFFSET), reg_A1);
    func.clrl(reg_D2);
    func.moveb(addr_(reg_A1), reg_D2);

    func.movem_from_stack({}, { reg_A1 });
    func.rts();

    // ------------------------------------------------------------------------------------------
    _func_get_selectable_maximum_value = _rom.inject_code(func);
    return _func_get_selectable_maximum_value;
}

/**
 * Changes the currently selected Selectable, updating the layout accordingly
 *
 * Input:
 *      - D0.b: ID of the new Selectable to select
 */
uint32_t Engine::func_set_current_selectable()
{
    if(_func_set_current_selectable)
        return _func_set_current_selectable;
    // ------------------------------------------------------------------------------------------

    md::Code func;
    func.movem_to_stack({ reg_D3 }, {});

    // Apply a neutral palette to previous selection
    func.movew(0x0000, reg_D3);
    func.jsr(func_apply_selection_mapping());

    // Change the currently selected option
    func.moveb(reg_D0, addrw_(_current_selectable_ram_addr));

    // Apply a highlighted palette to the new selection
    func.movew(0x2000, reg_D3);
    func.jsr(func_apply_selection_mapping());

    func.movem_from_stack({ reg_D3 }, {});
    func.rts();

    // ------------------------------------------------------------------------------------------
    _func_set_current_selectable = _rom.inject_code(func);
    return _func_set_current_selectable;
}

/**
 * Change the palette of N consecutive tiles, starting at the given address.
 *
 * Input:
 *      - D1.w: Position offset of the first tile to change
 *      - D2.l: Number of consecutive tiles to change
 *      - D3.w: Palette mask to apply (0x0000, 0x2000, 0x4000, 0x6000 for palettes 0, 1, 2, 3 respectively)
 *
 *      - A2: Plane map starting address
 */
uint32_t Engine::func_set_palette()
{
    if(_func_set_palette)
        return _func_set_palette;
    // ------------------------------------------------------------------------------------------

    md::Code func;
    func.movem_to_stack({ reg_D0 }, { reg_A1 });

    // Deduce the first tile address from plane map starting address (A2) and first tile offset (D1)
    func.lea(addr_(reg_A2, reg_D1), reg_A1);

    // Change the palette for the D4 consecutive tiles
    func.label("loop");
    func.movew(addr_(reg_A1), reg_D0);
    func.andiw(0x9FFF, reg_D0);
    func.orw(reg_D3, reg_D0);
    func.movew(reg_D0, addr_postinc_(reg_A1));
    func.dbra(reg_D2, "loop");

    func.label("return");
    func.movem_from_stack({ reg_D0 }, { reg_A1 });
    func.rts();

    // ------------------------------------------------------------------------------------------
    _func_set_palette = _rom.inject_code(func);
    return _func_set_palette;
}

/**
 * Apply currently selected option's "selection mapping", highlighting specific tiles of the layout with a specific
 * palette to show it's being selected.
 *
 * Input:
 *      - D3.w: Palette mask to apply (0x0000, 0x2000, 0x4000, 0x6000 for palettes 0, 1, 2, 3 respectively)
 *      - A4: Layout descriptor addr (must be at least a VerticalMenu to have selection mappings)
 */
uint32_t Engine::func_apply_selection_mapping()
{
    if(_func_apply_selection_mapping)
        return _func_apply_selection_mapping;
    // ------------------------------------------------------------------------------------------

    md::Code func;
    func.movem_to_stack({ reg_D0,reg_D1,reg_D2 }, { reg_A1,reg_A2 });

    // Set plane map starting address in A2
    func.lea(addr_(RAM_start), reg_A2);

    // Mask argument passed in D3 so it cannot alter the tile attributes other than palette ID
    func.andiw(0x6000, reg_D3);

    // Store currently selected option in D0
    func.clrl(reg_D0);
    func.moveb(addr_(_current_selectable_ram_addr), reg_D0);

    // Get the address for Selectable #D0, and store it in A1
    func.jsr(func_get_selectable_addr());
    // Read the coloring data from Selectable structure, and call the set_palette function
    func.clrl(reg_D1);
    func.clrl(reg_D2);
    func.movew(addr_(reg_A1, Selectable::COLORING_POSITION_OFFSET), reg_D1);
    func.moveb(addr_(reg_A1, Selectable::COLORING_SIZE_OFFSET), reg_D2);
    func.jsr(func_set_palette());

    func.label("return");
    func.movem_from_stack({ reg_D0,reg_D1,reg_D2 }, { reg_A1,reg_A2 });
    func.rts();

    // ------------------------------------------------------------------------------------------
    _func_apply_selection_mapping = _rom.inject_code(func);
    return _func_apply_selection_mapping;
}

/**
 * Decompiles data which was compressed using the Nemesis algorithm.
 *
 * Input:
 *      - A0: points on the data to decompress
 */
uint32_t Engine::func_nemesis_decomp()
{
    if(_func_nemesis_decomp)
        return _func_nemesis_decomp;
    // ------------------------------------------------------------------------------------------

    _func_nemesis_decomp = _rom.inject_bytes(NEM_DECOMP, NEM_DECOMP_SIZE);
    return _func_nemesis_decomp;
}

/**
 * Poll the controller status and update the appropriate RAM slot with bits related to the currently pressed buttons.
 */
uint32_t Engine::func_poll_controller()
{
    if(_func_poll_controller)
        return _func_poll_controller;
    // ------------------------------------------------------------------------------------------

    md::Code func;
    func.movem_to_stack({ reg_D0,reg_D1 }, { reg_A0 });

    // Might be necessary to ensure this works in any game:
    // moveq	#$40,d0
    // move.b	d0,(HW_Port_1_Control).l
    func.lea(addr_(Controller_1_data_port), reg_A0);

    func.moveb(0x00, addr_(reg_A0)); // Poll controller data port
    func.nop(2);
    func.moveb(addr_(reg_A0), reg_D0); // Get controller port data (start/A)
    func.lslb(0x2, reg_D0);
    func.andib(0xC0, reg_D0);

    func.moveb(0x40, addr_(reg_A0)); // Poll controller data port again
    func.nop(2);
    func.moveb(addr_(reg_A0), reg_D1); // Get controller port data (B/C/Dpad)
    func.andib(0x3F, reg_D1);
    func.orb(reg_D1, reg_D0); // Fuse together into one controller bit array
    func.notb(reg_D0);

    func.moveb(reg_D0, addrw_(_input_bits_ram_addr));

    func.movem_from_stack({ reg_D0,reg_D1 }, { reg_A0 });
    func.rts();

    // ------------------------------------------------------------------------------------------
    _func_poll_controller = _rom.inject_code(func);
    return _func_poll_controller;
}

/**
 * Vertical interruption working as a wrapper for the original VInt inside the ROM the UI lib is injected in.
 * It overrides initial behavior by custom behavior to make the UI game-independent and self-sufficient, regardless
 * of the game it is injected in.
 */
uint32_t Engine::func_v_int()
{
    if(_func_v_int)
        return _func_v_int;
    // ------------------------------------------------------------------------------------------

    uint32_t original_v_int = _rom.get_long(0x78);

    md::Code func;

    func.nop();
    func.tstb(addr_(_ui_mode_ram_addr));
    func.beq("fallback_to_old_vint"); // If UI Mode == 0, jump to the usual VInt for this game

    func.movem_to_stack({ reg_D0_D7 }, { reg_A0_A6 });
    func.label("wait_for_vertical_blanking");
    func.movew(addr_(VDP_control_port), reg_D0);
    func.andiw(0x8, reg_D0);
    func.beq("wait_for_vertical_blanking");

    // If PAL system, wait a bit
//    func_v_int.btst(0x6, addrw_(0xFFD8)); // Graphics_flags
//    func_v_int.beq("ntsc_system");
//    func_v_int.movew(0x700, reg_D0);
//    func_v_int.label("pal_wait_loop");
//    func_v_int.dbra(reg_D0, "pal_wait_loop");
//    func_v_int.label("ntsc_system");

    // Stop Z80
//    func_v_int.movew(0x100, addr_(Z80_bus_request));
//    func_v_int.label("wait_z80_stop");
//    func_v_int.btst(0x0, addr_(Z80_bus_request));
//    func_v_int.bne("wait_z80_stop");

    func.jsr(func_poll_controller()); // pollControllers = 0x10DE

    // dma68kToVDP 0xFC00 (Palette),$0000,$80,CRAM
    func.lea(addr_(VDP_control_port), reg_A5);
    func.movel(0x94009340, addr_(reg_A5));
    func.movel(0x96FE9500, addr_(reg_A5));
    func.movew(0x977F, addr_(reg_A5));
    func.movew(0xC000, addr_(reg_A5));
    func.movew(0x80, addrw_(0xF640));
    func.movew(addrw_(0xF640), addr_(reg_A5));

    // dma68kToVDP 0xF800 (Sprite_table_buffer),$F800,$280,VRAM
    func.lea(addr_(VDP_control_port), reg_A5);
    func.movel(0x94019340, addr_(reg_A5));
    func.movel(0x96FC9500, addr_(reg_A5));
    func.movew(0x977F, addr_(reg_A5));
    func.movew(0x7800, addr_(reg_A5));
    func.movew(0x83, addrw_(0xF640));
    func.movew(addrw_(0xF640), addr_(reg_A5));

//    func_v_int.jsr(0x1588); // Process_DMA_Queue

    // Start Z80
//    func_v_int.movew(0x0, addr_(Z80_bus_request));

    func.moveb(UI_MODE_V_INT_OCCURRED, addr_(_ui_mode_ram_addr)); // Set GUI Mode to "VInt occurred"
    func.movem_from_stack({ reg_D0_D7 }, { reg_A0_A6 });
    func.rte();

    func.label("fallback_to_old_vint");
    func.jmp(original_v_int);

    // ------------------------------------------------------------------------------------------
    _func_v_int = _rom.inject_code(func);
    return _func_v_int;
}

/**
 * Closed loop waiting for our custom vertical interruption to kick in (see func_v_int) in order to leave the function.
 * This ensures vertical blank occurred and the frame was drawn by the VDP, effectively ensuring the code alongside
 * its call is run exactly once per frame.
 */
uint32_t Engine::func_wait_vsync()
{
    if(_func_wait_vsync)
        return _func_wait_vsync;
    // ------------------------------------------------------------------------------------------

    md::Code func;
    func.move_to_sr(0x2300); // Enable VInts

    func.label("wait_for_v_int");
    func.cmpib(UI_MODE_V_INT_OCCURRED, addr_(_ui_mode_ram_addr));
    func.bne("wait_for_v_int");

    func.moveb(UI_MODE_ENABLED, addr_(_ui_mode_ram_addr));

    func.move_to_sr(0x2700); // Disable VInts
    func.rts();

    // ------------------------------------------------------------------------------------------
    _func_wait_vsync = _rom.inject_code(func);
    return _func_wait_vsync;
}

/**
 * Send the full plane map to VRAM, updating the tiles displayed on screen.
 *
 * Input:
 *      - D0.w: VDP control word
 *      - D1.b: tiles per row
 *      - D2.b: number of rows to copy
 */
uint32_t Engine::func_copy_plane_map_to_vram()
{
    if(_func_copy_plane_map_to_vram)
        return _func_copy_plane_map_to_vram;
    // ------------------------------------------------------------------------------------------

    md::Code func;

    func.lea(addr_(RAM_start), reg_A1);
    func.movel(0x40000003, reg_D0);
    func.moveq(0x27, reg_D1);
    func.moveq(0x1B, reg_D2);

    func.lea(addr_(VDP_data_port), reg_A6);

    func.label("loop_row");
    func.movel(reg_D0, addr_(VDP_control_port));
    func.movew(reg_D1, reg_D3);

    func.label("loop_tile");
    func.movew(addr_postinc_(reg_A1), addr_(reg_A6));
    func.dbra(reg_D3, "loop_tile");   // copy one row

    func.addil(0x800000, reg_D0); // move onto next row
    func.dbra(reg_D2, "loop_row");
    func.rts();

    // ------------------------------------------------------------------------------------------
    _func_copy_plane_map_to_vram = _rom.inject_code(func);
    return _func_copy_plane_map_to_vram;
}

/**
 * Erase text pointed by A1, replacing it with placeholder / empty character.
 *
 * Input:
 *      - A1: pointer on the text structure
 */
uint32_t Engine::func_erase_text()
{
    if(_func_erase_text)
        return _func_erase_text;
    // ------------------------------------------------------------------------------------------

    md::Code func;
    func.movem_to_stack({ reg_D0,reg_D2,reg_D3 }, { reg_A2,reg_A3 });

    // Put the plane map starting address in A3
    func.lea(addr_(RAM_start), reg_A3);

    // Read the string position (D3), then use it to deduce plane map address where to draw the string (A2)
    func.movew(addr_postinc_(reg_A1), reg_D3);
    func.lea(addrw_(reg_A3, reg_D3), reg_A2);

    // Store the string size (first byte of the string) in D2
    func.moveq(0, reg_D2);
    func.moveb(addr_postinc_(reg_A1), reg_D2);

    // Iterate D2 times to put a placeholder character in place of the option text into the RAM
    func.label("loop_erase_letter");
    {
        func.movew(addr_(reg_A2), reg_D0);              // Fetch data already present in the plane map
        func.andiw(0x6000, reg_D0);                     // Filter everything but the palette info (in order to keep palette)
        func.orib(_alignment_helper_character, reg_D0); // Set the new tile ID to the alignement helper character
        func.movew(reg_D0, addr_postinc_(reg_A2));      // Send the new data back inside the plane map
    }
    func.dbra(reg_D2, "loop_erase_letter");

    func.movem_from_stack({ reg_D0,reg_D2,reg_D3 }, { reg_A2,reg_A3 });
    func.rts();

    // ------------------------------------------------------------------------------------------
    _func_erase_text = _rom.inject_code(func);
    return _func_erase_text;
}

/**
 * Draw an "alignment helper line", consisting of a small character helping the user to see the alignment between
 * options labels and values.
 *
 * Input:
 *      - A1: pointer on the option value Text structure, used to get the position where to draw the line
 */
uint32_t Engine::func_draw_alignment_helper_line()
{
    if(_func_draw_alignment_helper_line)
        return _func_draw_alignment_helper_line;
    // ------------------------------------------------------------------------------------------

    md::Code func;
    func.movem_to_stack({ reg_D0,reg_D2,reg_D3 }, { reg_A2 });

    // Read the string position (D3), then use it to deduce plane map address where we should draw the option string (A2)
    func.lea(addr_(RAM_start), reg_A2);
    func.movew(addr_(reg_A1), reg_D3);
    func.lea(addrw_(reg_A2, reg_D3), reg_A2);

    // Read the string size (D3), then subtract it from the line size to know how much characters we need to draw
    func.moveb(addr_(reg_A1, 0x2), reg_D3);
    func.moveq(37, reg_D2);
    func.subb(reg_D3, reg_D2);
    func.suba(reg_D2, reg_A2);
    func.suba(reg_D2, reg_A2);

    // Now, loop as much as we have moved back to fill the line with alignment helper tiles
    func.label("loop_write_letter");
    func.movew(addr_(reg_A2), reg_D0);              // Fetch data already present in the plane map
    func.andiw(0x6000, reg_D0);                     // Filter everything but the palette info (in order to keep palette)
    func.orib(_alignment_helper_character, reg_D0); // Set the new tile ID
    func.movew(reg_D0, addr_postinc_(reg_A2));      // Send the new data back inside the plane map
    func.dbra(reg_D2, "loop_write_letter");

    func.movem_from_stack({ reg_D0,reg_D2,reg_D3 }, { reg_A2 });
    func.rts();

    // ------------------------------------------------------------------------------------------
    _func_draw_alignment_helper_line = _rom.inject_code(func);
    return _func_draw_alignment_helper_line;
}

/**
 * Draw the text structure pointed by A1 on screen, respecting its position and its contents.
 *
 * Input:
 *      - A1: pointer on the text structure
 */
uint32_t Engine::func_draw_text()
{
    if(_func_draw_text)
        return _func_draw_text;
    // ------------------------------------------------------------------------------------------

    md::Code func;
    func.movem_to_stack({ reg_D0,reg_D2,reg_D3 }, { reg_A2,reg_A3 });

    // Put the plane map starting address in A3
    func.lea(addr_(RAM_start), reg_A3);

    // Read the string position (D3), then use it to deduce plane map address where to draw the string (A2)
    func.movew(addr_postinc_(reg_A1), reg_D3);
    func.lea(addrw_(reg_A3, reg_D3), reg_A2);

    // Store the string size (first byte of the string) in D2
    func.moveq(0, reg_D2);
    func.moveb(addr_postinc_(reg_A1), reg_D2);

    // Iterate D2 times to write each letter tile of the string into the RAM
    func.label("loop_write_letter");
    {
        func.movew(addr_(reg_A2), reg_D0);          // Store tile descriptor already present in the plane map inside D0
        func.andiw(0x6000, reg_D0);                 // Filter everything but the palette info (in order to keep palette)
        func.orb(addr_postinc_(reg_A1), reg_D0);    // Set the new tile ID
        func.movew(reg_D0, addr_postinc_(reg_A2));  // Send the new data back inside the plane map
    }
    func.dbra(reg_D2, "loop_write_letter");

    func.movem_from_stack({ reg_D0,reg_D2,reg_D3 }, { reg_A2,reg_A3 });
    func.rts();

    // ------------------------------------------------------------------------------------------
    _func_draw_text = _rom.inject_code(func);
    return _func_draw_text;
}

/**
 * Builds the initial plane map containing the tiles to be drawn on screen on each frame.
 * It is built using the static strings and the dynamic option values.
 */
uint32_t Engine::func_build_initial_plane_map()
{
    if(_func_build_initial_plane_map)
        return _func_build_initial_plane_map;
    // ------------------------------------------------------------------------------------------

    md::Code func;
    func.jsr(func_draw_all_selectable_values());

    func.movel(addr_(reg_A4, Info::STRINGS_OFFSET), reg_A1);
    func.moveq(0, reg_D0);

    // Double loop to write each letter of each line of text
    func.label("loop_write_string");
    // If string position is 0xFFFF, it means we reached end of table
    func.cmpiw(0xFFFF, addr_(reg_A1));
    func.beq("complete");
    func.jsr(func_draw_text());
    func.cmpib(0xFE, addr_(reg_A1));
    func.bne("loop_write_string");
    // If next byte is 0xFE, it's a padding byte we need to skip (because reading words on odd addresses is illegal)
    func.addql(0x1, reg_A1);
    func.bra("loop_write_string");

    // Send our built plane map to VRAM
    func.label("complete");
    func.rts();

    // ------------------------------------------------------------------------------------------
    _func_build_initial_plane_map = _rom.inject_code(func);
    return _func_build_initial_plane_map;
}

/**
 * Check the currently pressed buttons, and call the appropriate event handlers accordingly
 */
uint32_t Engine::func_handle_ui_controls()
{
    if(_func_handle_ui_controls)
        return _func_handle_ui_controls;
    // ------------------------------------------------------------------------------------------

    auto HANDLE_BUTTON_PRESS = [this](md::Code& func, const std::string& button_name, uint8_t button_bit)
    {
        std::string end_label = button_name + "_end";

        // If callback address is 0x0, it means this button is not handled, so no need to check it
        func.cmpil(0x0, addr_(reg_A1));
        func.beq(end_label);

        // There is a valid callback, test the actual bit for this button
        func.btst(button_bit, reg_D0);
        func.beq(end_label);

        // Button is pressed, call the callback function and set the repeat counter to prevent
        // high-speed consecutive inputs
        func.movel(addr_(reg_A1), reg_A2);
        func.jsr(addr_(reg_A2));
        func.moveb(0x9, addr_(_input_repeat_ram_addr));

        func.label(end_label);
        func.addql(0x4, reg_A1);
    };

    md::Code func;

    // First of all, check if we didn't already press a button recently. If so, just leave.
    func.tstb(addr_(_input_repeat_ram_addr));
    func.beq("can_process_inputs");
    func.subib(1, addr_(_input_repeat_ram_addr));
    func.rts();

    func.label("can_process_inputs");
    // Make D0 contain the currently pressed button bits
    func.moveb(addr_(_input_bits_ram_addr), reg_D0);
    // Make A1 point on the first controller event callback address
    func.lea(addr_(reg_A4, Info::CONTROLLER_EVENTS_OFFSET), reg_A1);

    // Test consecutively each button, and call the appropriate callback if pressed
    HANDLE_BUTTON_PRESS(func, "up",     0);
    HANDLE_BUTTON_PRESS(func, "down",   1);
    HANDLE_BUTTON_PRESS(func, "left",   2);
    HANDLE_BUTTON_PRESS(func, "right",  3);
    HANDLE_BUTTON_PRESS(func, "b",      4);
    HANDLE_BUTTON_PRESS(func, "c",      5);
    HANDLE_BUTTON_PRESS(func, "a",      6);
    HANDLE_BUTTON_PRESS(func, "start",  7);

    func.rts();

    // ------------------------------------------------------------------------------------------
    _func_handle_ui_controls = _rom.inject_code(func);
    return _func_handle_ui_controls;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
///        INITIAL BOOTUP AND CORE HANDLING
///////////////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * Schedule a change of UI for the next frame, letting the rest of events in current frame unroll.
 *
 * Input:
 *      - A0: address on the new UI to display, or 0xFFFFFFFF to go back to previous UI
 */
uint32_t Engine::func_schedule_ui_change()
{
    if(_func_schedule_ui_change)
        return _func_schedule_ui_change;
    // ------------------------------------------------------------------------------------------
    md::Code func;

    func.moveb(mdui::Engine::UI_MODE_CHANGE, addrw_(_ui_mode_ram_addr));
    func.cmpa(0xFFFFFFFF, reg_A0);
    func.beq("return");
    func.movel(reg_A0, addrw_(_next_ui_addr));
    func.label("return");
    func.rts();

    // ------------------------------------------------------------------------------------------
    _func_schedule_ui_change = _rom.inject_code(func);
    return _func_schedule_ui_change;
}

/**
 * Schedule exiting the UI for the next frame, letting the rest of events in current frame unroll.
 */
uint32_t Engine::func_schedule_ui_exit()
{
    if(_func_schedule_ui_exit)
        return _func_schedule_ui_exit;
    // ------------------------------------------------------------------------------------------
    md::Code func;

    func.moveb(mdui::Engine::UI_MODE_EXIT, addrw_(_ui_mode_ram_addr));
    func.rts();

    // ------------------------------------------------------------------------------------------
    _func_schedule_ui_exit = _rom.inject_code(func);
    return _func_schedule_ui_exit;
}


/**
 * Initialize all the required things for the UI engine to work properly.
 * This only need to be called once at the beginning of the UI session.
 */
uint32_t Engine::func_init_engine()
{
    if(_func_init_engine)
        return _func_init_engine;
    // ------------------------------------------------------------------------------------------

    uint32_t gui_tileset_addr = _rom.inject_bytes(GUI_TILESET, GUI_TILESET_SIZE);

    md::Code func;
    func.move_to_sr(0x2700); // Disable VInts

    // Call the pre-init function if there is one
    if(_preinit_function_addr)
        func.jsr(_preinit_function_addr);

//    func_init_gui.movew(addrw_(VDP_reg_1_command), reg_D0);  // 0xFFFFF60E
//    func_init_gui.andib(0xBF, reg_D0);
//    func_init_gui.movew(reg_D0, addr_(VDP_control_port));
//    func_init_gui.jsr(Clear_DisplayData);     // 0x11CA

    func.lea(addr_(VDP_control_port), reg_A6);
    func.movew(0x8004, addr_(reg_A6));
    func.movew(0x8230, addr_(reg_A6));
    func.movew(0x8407, addr_(reg_A6));
    func.movew(0x8230, addr_(reg_A6));
    func.movew(0x8700, addr_(reg_A6));
    func.movew(0x8C81, addr_(reg_A6));
    func.movew(0x9001, addr_(reg_A6));
    func.movew(0x8B00, addr_(reg_A6));

//    func_init_gui.clrw(addrw_(DMA_queue));  // 0xFFFFFB00
//    func_init_gui.movel(DMA_queue, addrw_(DMA_queue_slot)); // 0xFFFFFBFC
    func.movel(0x42000000, addr_(VDP_control_port));

    // Decompress GUI tileset
    func.lea(addr_(gui_tileset_addr), reg_A0);
    func.jsr(func_nemesis_decomp());

    func.moveb(UI_MODE_ENABLED, addr_(_ui_mode_ram_addr));
    func.rts();

    // ------------------------------------------------------------------------------------------
    _func_init_engine = _rom.inject_code(func);
    return _func_init_engine;
}

/**
 * Initialize all the things related to the specific UI we are going to display.
 * This needs to be called every time a new UI is starting to be displayed.
 */
uint32_t Engine::func_init_ui()
{
    if(_func_init_ui)
        return _func_init_ui;
    // ------------------------------------------------------------------------------------------

    md::Code func;

    func.lea(addr_(RAM_start), reg_A1);
    func.clrl(reg_D0);
    func.movew(0x8C0 / 2, reg_D0);
    func.label("loop_clear_plane_map");
    func.movew(0, addr_postinc_(reg_A1));
    func.dbra(reg_D0, "loop_clear_plane_map");

    // Init palettes
    constexpr uint16_t PALETTE_0_ADDR = 0xFC00;
    func.lea(addr_(reg_A4, Info::COLOR_PALETTES_OFFSET), reg_A1);
    func.movew(addr_postinc_(reg_A1), addrw_(PALETTE_0_ADDR));
    func.movew(addr_postinc_(reg_A1), addrw_(PALETTE_0_ADDR + 0xC));
    func.movew(addr_postinc_(reg_A1), addrw_(PALETTE_0_ADDR + 0xE));

    constexpr uint16_t PALETTE_1_ADDR = PALETTE_0_ADDR + 0x20;
    func.movew(addr_postinc_(reg_A1), addrw_(PALETTE_1_ADDR + 0xC));
    func.movew(addr_postinc_(reg_A1), addrw_(PALETTE_1_ADDR + 0xE));

    func.movew(0x8134, reg_D0);
    func.orib(0x40, reg_D0);
    func.movew(reg_D0, addr_(VDP_control_port));

    // Build the initial plane map and apply the initial selection (if any)
    func.jsr(func_build_initial_plane_map());
    func.movew(0x2000, reg_D3);
    func.jsr(func_apply_selection_mapping());

    func.rts();

    // ------------------------------------------------------------------------------------------
    _func_init_ui = _rom.inject_code(func);
    return _func_init_ui;
}

/**
 * Entry point for the UI engine, call this function to start displaying a given UI.
 *
 * Input:
 *      - A4: pointer on the UI descriptor structure
 */
uint32_t Engine::func_boot_ui()
{
    if(_func_boot_ui)
        return _func_boot_ui;
    // ------------------------------------------------------------------------------------------

    md::Code func;

    func.jsr(func_init_engine());

    func.label("init_ui");
    func.jsr(func_init_ui());

    func.label("main_loop");
    // Handle controls, calling callback functions that have been associated to the layout if matching buttons are pressed
    func.jsr(func_handle_ui_controls());
    // If asked to, exit the UI
    func.cmpib(UI_MODE_EXIT, addrw_(_ui_mode_ram_addr));
    func.beq("exit_ui");
    func.cmpib(UI_MODE_CHANGE, addrw_(_ui_mode_ram_addr));
    func.beq("ui_change");

    // Copy the plane map to VRAM
    func.jsr(func_copy_plane_map_to_vram());
    // Wait for the frame to be drawn by the VDP before processing another one
    func.jsr(func_wait_vsync());
    // Otherwise, just loop indefinitely
    func.bra("main_loop");

    func.label("exit_ui");
    func.moveb(UI_MODE_DISABLED, addrw_(_ui_mode_ram_addr));
    func.moveb(0x0, addrw_(0xFF69));
    func.rts();

    func.label("ui_change");
    func.moveb(UI_MODE_ENABLED, addrw_(_ui_mode_ram_addr));
    func.movel(reg_A4, reg_A1);
    func.movel(addrw_(_next_ui_addr), reg_A4);
    func.movel(reg_A1, addrw_(_next_ui_addr)); // Put the previous UI back there to make going back easy if needed
    func.bra("init_ui");

    // ------------------------------------------------------------------------------------------
    _func_boot_ui = _rom.inject_code(func);
    return _func_boot_ui;
}

} // namespace end