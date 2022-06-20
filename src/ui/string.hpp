#pragma once

#include <string>
#include <utility>
#include "../tools/byte_array.hpp"

class UiString {
private:
    std::string _str;
    std::pair<uint8_t, uint8_t> _position;

public:
    explicit UiString(std::string str, std::pair<uint8_t, uint8_t> position) :
        _str        (std::move(str)),
        _position   (std::move(position))
    {}

    [[nodiscard]] const std::string& string() const { return _str; }
    [[nodiscard]] const std::pair<uint8_t, uint8_t>& psoition() const { return _position; }

    [[nodiscard]] ByteArray text_bytes() const;
    [[nodiscard]] uint16_t position_bytes() const;
};