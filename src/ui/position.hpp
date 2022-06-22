#pragma once

#include <cstdint>

namespace mdui {

struct Position {
    uint8_t x;
    uint8_t y;

    Position(uint8_t x, uint8_t y) : x(x), y(y) {}

    [[nodiscard]] uint16_t as_offset_word() const
    {
        return (x * 2) + (y * 0x50);
    }

};

} // namespace mdui