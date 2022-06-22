#pragma once

#include <cstdint>
#include "position.hpp"
#include "../tools/byte_array.hpp"

namespace mdui {

struct SelectionMapping {
    uint8_t option_id;
    Position position;
    uint8_t size;

    SelectionMapping(uint8_t option_id, uint8_t x, uint8_t y, uint8_t size) :
        option_id  (option_id),
        position   (x, y),
        size       (size)
    {}

    [[nodiscard]] ByteArray as_bytes() const
    {
        ByteArray bytes;
        bytes.add_byte(option_id);
        bytes.add_byte(size);
        bytes.add_word(position.as_offset_word());
        return bytes;
    }
};

} // namespace end