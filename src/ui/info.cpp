#include "info.hpp"
#include "../md_tools/md_tools.hpp"

namespace mdui {

static ByteArray build_text_list_bytes(const std::vector<Text>& text_list)
{
    ByteArray bytes;

    for(const Text& str : text_list)
    {
        bytes.add_bytes(str.as_bytes());

        // Add a padding byte containing recognizable 0xFE in case we land on an odd address
        if(bytes.size() % 2 != 0)
            bytes.add_byte(0xFE);
    }
    bytes.add_word(0xFFFF);

    return bytes;
}

ByteArray Info::build_selection_mapping_bytes() const
{
    ByteArray bytes;
    for(const SelectionMapping& mapping : _selection_mappings)
        bytes.add_bytes(mapping.as_bytes());
    bytes.add_byte(0xFF);
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

ByteArray Info::build_option_values_bytes(md::ROM& rom) const
{
    // Data structure:
    //      - a word stating how many options we have
    //      - for each option, there is a long pointing to its own values text table (0x00000000 if no values)
    //      - values text tables are injected elsewhere and contain:
    //          - a word stating how many values there are
    //          - Text structure for each value are dumped contiguously
    //          - To get string X, you need to skip X-1 strings by adding string sizes (+ padding bytes) one after another

    ByteArray list_of_values_list_addrs;
    for(const std::vector<Text>& values_for_option : _option_values)
    {
        if(values_for_option.empty())
        {
            list_of_values_list_addrs.add_long(0x0);
        }
        else
        {
            ByteArray value_list;
            value_list.add_word(values_for_option.size());
            value_list.add_bytes(build_text_list_bytes(values_for_option));
            list_of_values_list_addrs.add_long(rom.inject_bytes(value_list));
        }
    }

    return list_of_values_list_addrs;
}

uint32_t Info::inject(md::ROM& rom)
{
    if(_main_table_addr)
        return _main_table_addr;

    ByteArray main_bytes_table;
    main_bytes_table.add_long(rom.inject_bytes(build_text_list_bytes(_strings)));
    main_bytes_table.add_long(rom.inject_bytes(build_selection_mapping_bytes()));

    main_bytes_table.add_long(_on_up_pressed_addr);
    main_bytes_table.add_long(_on_down_pressed_addr);
    main_bytes_table.add_long(_on_left_pressed_addr);
    main_bytes_table.add_long(_on_right_pressed_addr);
    main_bytes_table.add_long(_on_b_pressed_addr);
    main_bytes_table.add_long(_on_c_pressed_addr);
    main_bytes_table.add_long(_on_a_pressed_addr);
    main_bytes_table.add_long(_on_start_pressed_addr);
    main_bytes_table.add_bytes(build_palette_bytes());

    main_bytes_table.add_word(_option_values.size()-1);
    main_bytes_table.add_bytes(build_option_values_bytes(rom));

    _main_table_addr = rom.inject_bytes(main_bytes_table);
    return _main_table_addr;
}

void Info::add_string(uint8_t x, uint8_t y, const std::string& str)
{
    _strings.emplace_back(Text(str, x, y));
}

void Info::add_selectable_option(uint8_t x, uint8_t y, const std::string& str, const std::vector<std::string>& value_strings)
{
    this->add_string(x, y, str);

    uint8_t option_id = 0;
    if(!_selection_mappings.empty())
        option_id = last_option_id() + 1;

    SelectionMapping mapping(option_id , x, y, static_cast<uint8_t>(str.size()));
    _selection_mappings.emplace_back(mapping);

    std::vector<Text> option_values;
    for(const std::string& value_string : value_strings)
    {
        uint8_t value_x = 39 - value_string.size();
        option_values.emplace_back(Text(value_string, value_x, y));
    }
    _option_values.emplace_back(option_values);
}

uint8_t Info::last_option_id() const
{
    uint8_t highest_option_id = 0;
    for(const SelectionMapping& mapping : _selection_mappings)
        if(mapping.option_id > highest_option_id)
            highest_option_id = mapping.option_id;
    return highest_option_id;
}

} // namespace mdui
