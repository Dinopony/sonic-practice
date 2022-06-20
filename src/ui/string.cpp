#include "string.hpp"

ByteArray UiString::text_bytes() const
{
    const std::vector<char> CONVERSION_TABLE = {
            ' ',
            '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
            '*', '@', ':', '.',
            'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M',
            'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
            '+', '_', '%',
            'l', 'r', 'u', 'd', 'a', 'b', 'c', 's'
    };

    ByteArray output;

    for(char char_in_string : _str)
    {
        for(size_t i=0 ; i<CONVERSION_TABLE.size() ; ++i)
        {
            if(CONVERSION_TABLE[i] == char_in_string)
            {
                output.add_byte(i + 0x0F);
                break;
            }
        }
    }

    uint8_t string_length = static_cast<uint8_t>(output.size()-1);
    output.insert(output.begin(), string_length);

    return output;
}

uint16_t UiString::position_bytes() const
{
    return (_position.first * 2) + (0x50 * _position.second);
}