#pragma once

#include <cstdint>
#include <utility>
#include "position.hpp"
#include "../tools/byte_array.hpp"

namespace mdui {

class Selectable {
private:
    Position _coloring_position;      ///< The position where to start coloring when this Selectable is being selected
    uint8_t _coloring_size;           ///< The number of tiles to color after _coloring_position when this Selectable is being selected
    uint16_t _value_storage_address;  ///< The address in RAM where the currently selected value for this Selectable is stored
    std::vector<Text> _value_strings; ///< The Text strings of each available value for this Selectable

public:
    static constexpr uint32_t COLORING_POSITION_OFFSET = 0;
    static constexpr uint32_t COLORING_SIZE_OFFSET = COLORING_POSITION_OFFSET + 0x2;
    static constexpr uint32_t VALUES_COUNT_OFFSET = COLORING_SIZE_OFFSET + 0x1;
    static constexpr uint32_t VALUE_STORAGE_ADDRESS_OFFSET = VALUES_COUNT_OFFSET + 0x1;
    static constexpr uint32_t VALUE_STRINGS_OFFSET = VALUE_STORAGE_ADDRESS_OFFSET + 0x2;

    Selectable(Position coloring_pos, uint8_t coloring_size, uint16_t storage_address, std::vector<Text> value_strings) :
        _coloring_position      (coloring_pos),
        _coloring_size          (coloring_size),
        _value_storage_address  (storage_address),
        _value_strings          (std::move(value_strings))
    {}

    [[nodiscard]] ByteArray as_bytes() const
    {
        ByteArray bytes;
        bytes.add_word(_coloring_position.as_offset_word());
        bytes.add_byte(_coloring_size);
        bytes.add_byte(_value_strings.size()-1);
        bytes.add_word(_value_storage_address);

        for(const Text& text : _value_strings)
            bytes.add_bytes(text.as_bytes());

        return bytes;
    }
};

} // namespace end