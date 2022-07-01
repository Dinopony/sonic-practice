#include "info.hpp"
#include "../md_tools/md_tools.hpp"

namespace mdui {

ByteArray Info::build_strings_list_bytes() const
{
    ByteArray bytes;

    for(const Text& str : _strings)
        bytes.add_bytes(str.as_bytes());
    bytes.add_word(0xFFFF);

    return bytes;
}

ByteArray Info::build_selectables_list_bytes(md::ROM& rom) const
{
    ByteArray bytes;
    bytes.add_word((uint16_t) _selectables.size() - 1);
    for(const Selectable& selectable : _selectables)
    {
        uint32_t selectable_addr = rom.inject_bytes(selectable.as_bytes());
        bytes.add_long(selectable_addr);
    }
    return bytes;
}

ByteArray Info::build_palette_bytes() const
{
    std::vector<const Color*> colors = {
            &_background_color,
            &_text_color_palette_neutral.first, &_text_color_palette_neutral.second,
            &_text_color_palette_selected.first, &_text_color_palette_selected.second
    };

    ByteArray bytes;
    for(const Color* color : colors)
    {
        auto& [r, g, b] = *color;
        uint16_t color_word = ((b & 0xF) << 8) + ((g & 0xF) << 4) + (r & 0xF);
        bytes.add_word(color_word);
    }

    return bytes;
}

uint32_t Info::inject(md::ROM& rom)
{
    if(_main_table_addr)
        return _main_table_addr;

    ByteArray main_bytes_table;
    main_bytes_table.add_long(rom.inject_bytes(build_strings_list_bytes()));
    main_bytes_table.add_long(rom.inject_bytes(build_selectables_list_bytes(rom)));

    main_bytes_table.add_long(_on_up_pressed_addr);
    main_bytes_table.add_long(_on_down_pressed_addr);
    main_bytes_table.add_long(_on_left_pressed_addr);
    main_bytes_table.add_long(_on_right_pressed_addr);
    main_bytes_table.add_long(_on_b_pressed_addr);
    main_bytes_table.add_long(_on_c_pressed_addr);
    main_bytes_table.add_long(_on_a_pressed_addr);
    main_bytes_table.add_long(_on_start_pressed_addr);
    main_bytes_table.add_bytes(build_palette_bytes());

    _main_table_addr = rom.inject_bytes(main_bytes_table);
    return _main_table_addr;
}

void Info::add_string(uint8_t x, uint8_t y, const std::string& str)
{
    _strings.emplace_back(Text(str, x, y));
}

void Info::add_selectable_option(uint8_t x, uint8_t y, const std::string& str, uint16_t storage_addr,
                                 const std::vector<std::string>& value_strings)
{
    this->add_string(x, y, str + " ");

    std::vector<Text> selectable_values;
    for(std::string value_string : value_strings)
    {
        value_string = " " + value_string;
        uint8_t value_x = 39 - value_string.size();
        selectable_values.emplace_back(Text(value_string, value_x, y));
    }

    Selectable selectable(Position(x,y), 37, storage_addr, selectable_values);
    _selectables.emplace_back(selectable);
}

} // namespace mdui
