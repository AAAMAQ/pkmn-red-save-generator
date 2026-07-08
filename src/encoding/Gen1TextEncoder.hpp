#pragma once

#include <cctype>
#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

#include "PrimitiveWriter.hpp"

namespace pkmn::savegen::encoding {

class Gen1TextEncoder {
public:
    static constexpr std::uint8_t Terminator = 0x50;

    static char ByteToAscii(std::uint8_t byte) {
        if (byte == Terminator) return '\0';
        if (byte >= 0x80 && byte <= 0x99) return static_cast<char>('A' + (byte - 0x80));
        if (byte >= 0xA0 && byte <= 0xB9) return static_cast<char>('a' + (byte - 0xA0));
        if (byte >= 0xF6 && byte <= 0xFF) return static_cast<char>('0' + (byte - 0xF6));
        if (byte == 0x7F) return ' ';
        if (byte == 0xE3) return '-';
        if (byte == 0xE6) return '?';
        if (byte == 0xE7) return '!';
        if (byte == 0xE8) return '.';
        if (byte == 0xF4) return ',';
        return '?';
    }

    static std::uint8_t AsciiToByte(char c) {
        if (c >= 'A' && c <= 'Z') return static_cast<std::uint8_t>(0x80 + (c - 'A'));
        if (c >= 'a' && c <= 'z') return static_cast<std::uint8_t>(0xA0 + (c - 'a'));
        if (c >= '0' && c <= '9') return static_cast<std::uint8_t>(0xF6 + (c - '0'));
        if (c == ' ') return 0x7F;
        if (c == '-') return 0xE3;
        if (c == '?') return 0xE6;
        if (c == '!') return 0xE7;
        if (c == '.') return 0xE8;
        if (c == ',') return 0xF4;
        throw std::invalid_argument("character is not encodable in the Milestone 1 Gen I charset subset");
    }

    static bool IsEncodable(char c) {
        try {
            static_cast<void>(AsciiToByte(c));
            return true;
        } catch (const std::invalid_argument&) {
            return false;
        }
    }

    static std::vector<std::uint8_t> EncodeName(std::string_view text, std::size_t fieldLength) {
        if (fieldLength == 0) {
            throw std::invalid_argument("field length must be non-zero");
        }
        if (text.size() >= fieldLength) {
            throw std::out_of_range("name is too long for the target field");
        }
        std::vector<std::uint8_t> encoded(fieldLength, Terminator);
        for (std::size_t i = 0; i < text.size(); ++i) {
            encoded[i] = AsciiToByte(text[i]);
        }
        return encoded;
    }

    static std::string DecodeName(const std::vector<std::uint8_t>& buffer, std::size_t offset, std::size_t fieldLength) {
        PrimitiveWriter::EnsureRange(buffer, offset, fieldLength);
        std::string decoded;
        decoded.reserve(fieldLength);
        for (std::size_t i = 0; i < fieldLength; ++i) {
            const char c = ByteToAscii(buffer[offset + i]);
            if (c == '\0') {
                break;
            }
            decoded.push_back(c);
        }
        return decoded;
    }

    static void WriteName(std::vector<std::uint8_t>& buffer,
                          std::size_t offset,
                          std::size_t fieldLength,
                          std::string_view text) {
        PrimitiveWriter::WriteBytes(buffer, offset, EncodeName(text, fieldLength));
    }
};

}  // namespace pkmn::savegen::encoding
