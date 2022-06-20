#include "info.hpp"
#include "../md_tools/md_tools.hpp"

ByteArray UiInfo::build_string_bytes() const
{
    ByteArray bytes;

    for(const UiString& str : _strings)
        bytes.add_bytes(str.text_bytes());
    bytes.add_byte(0xFF);

    return bytes;
}

ByteArray UiInfo::build_string_position_bytes() const
{
    ByteArray bytes;

    for(const UiString& str : _strings)
        bytes.add_word(str.position_bytes());
    bytes.add_word(0xFFFF);

    return bytes;
}

ByteArray UiInfo::build_selection_mapping_bytes() const
{
    ByteArray bytes;
    for(const auto& [selection_id, x, y, size] : _selection_mappings)
    {
        bytes.add_byte(selection_id);
        bytes.add_byte(x);
        bytes.add_byte(y);
        bytes.add_byte(size);
    }

    bytes.add_byte(0xFF);
    return bytes;
}

uint32_t UiInfo::inject_data(md::ROM& rom)
{
    if(_main_table_addr)
        return _main_table_addr;

    ByteArray main_bytes_table;
    main_bytes_table.add_long(rom.inject_bytes(build_string_bytes()));
    main_bytes_table.add_long(rom.inject_bytes(build_string_position_bytes()));
    main_bytes_table.add_long(rom.inject_bytes(build_selection_mapping_bytes()));
    main_bytes_table.add_long(_preinit_function_addr);
    main_bytes_table.add_long(_on_up_pressed_addr);
    main_bytes_table.add_long(_on_down_pressed_addr);
    main_bytes_table.add_long(_on_left_pressed_addr);
    main_bytes_table.add_long(_on_right_pressed_addr);
    main_bytes_table.add_long(_on_b_pressed_addr);
    main_bytes_table.add_long(_on_c_pressed_addr);
    main_bytes_table.add_long(_on_a_pressed_addr);
    main_bytes_table.add_long(_on_start_pressed_addr);

    _main_table_addr = rom.inject_bytes(main_bytes_table);
    return _main_table_addr;
}

uint8_t UiInfo::max_selection() const
{
    uint8_t highest_sel = 0;
    for(const auto& [sel,_2,_3,_4] : _selection_mappings)
        if(sel > highest_sel)
            highest_sel = sel;
    return highest_sel;
}