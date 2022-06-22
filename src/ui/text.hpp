#pragma once

#include <string>
#include <utility>
#include "../tools/byte_array.hpp"
#include "position.hpp"

namespace mdui {

class Text {
private:
    std::string _str;
    Position _position;

public:
    explicit Text(std::string str, uint8_t x, uint8_t y) :
        _str        (std::move(str)),
        _position   (x, y)
    {}

    [[nodiscard]] const std::string& string() const { return _str; }
    [[nodiscard]] const Position& position() const { return _position; }

    [[nodiscard]] ByteArray text_bytes() const;
    [[nodiscard]] uint16_t position_bytes() const { return _position.as_offset_word(); }
};

} // namespace end