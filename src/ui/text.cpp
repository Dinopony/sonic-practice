#include "text.hpp"

namespace mdui {

const char CONVERSION_TABLE[] = {
     ' ',
     '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
     '*', '@', ':', '.',
     'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M',
     'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
     '+', '_', '%', '-',
     'l', 'r', 'u', 'd', 'a', 'b', 'c', 's',
     0x0
};

static uint8_t convert_char(char c)
{
    for(uint8_t i=0 ; CONVERSION_TABLE[i] ; ++i)
    {
        if(CONVERSION_TABLE[i] == c)
            return i + 0x0F;
    }
}

ByteArray Text::as_bytes() const
{
    ByteArray string_chars;
    for(char char_in_string : _str)
        string_chars.add_byte(convert_char(char_in_string));

    ByteArray output;
    output.add_word(_position.as_offset_word());
    output.add_byte(string_chars.size()-1);
    output.add_bytes(string_chars);
    return output;
}

} // namespace end