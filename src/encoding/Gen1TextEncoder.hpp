#pragma once

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <iomanip>
#include <sstream>
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
        if (byte == 0xE0) return '\'';
        if (byte == 0xE3) return '-';
        if (byte == 0xE6) return '?';
        if (byte == 0xE7) return '!';
        if (byte == 0xE8 || byte == 0xF2) return '.';
        if (byte == 0xF3) return '/';
        if (byte == 0xF4) return ',';
        return '?';
    }

    static std::uint8_t AsciiToByte(char c) {
        if (c >= 'A' && c <= 'Z') return static_cast<std::uint8_t>(0x80 + (c - 'A'));
        if (c >= 'a' && c <= 'z') return static_cast<std::uint8_t>(0xA0 + (c - 'a'));
        if (c >= '0' && c <= '9') return static_cast<std::uint8_t>(0xF6 + (c - '0'));
        for (const auto& token : Tokens) {
            if (token.display.size() == 1U && token.display.front() == c) {
                return token.byte;
            }
        }
        throw std::invalid_argument(std::string("unsupported Gen I text character: ") + c);
    }

    static bool IsEncodable(char c) {
        try {
            static_cast<void>(AsciiToByte(c));
            return true;
        } catch (const std::invalid_argument&) {
            return false;
        }
    }

    static std::vector<std::uint8_t> EncodeName(std::string_view text,
                                                 std::size_t fieldLength) {
        if (fieldLength == 0) {
            throw std::invalid_argument("field length must be non-zero");
        }

        std::vector<std::uint8_t> glyphs;
        for (std::size_t pos = 0; pos < text.size();) {
            bool matched = false;
            for (const auto& token : Tokens) {
                if (token.lossless.size() > 1U && Consume(text, pos, token.lossless)) {
                    glyphs.push_back(token.byte);
                    pos += token.lossless.size();
                    matched = true;
                    break;
                }
            }
            if (matched) continue;

            std::uint8_t rawByte = 0;
            if (ConsumeRawByteToken(text, pos, &rawByte)) {
                if (rawByte == Terminator) {
                    throw std::invalid_argument(
                        "the Gen I terminator byte cannot appear inside a text value");
                }
                glyphs.push_back(rawByte);
                pos += 6U;
                continue;
            }

            const unsigned char c = static_cast<unsigned char>(text[pos]);
            if (c < 0x80U) {
                glyphs.push_back(AsciiToByte(static_cast<char>(c)));
                ++pos;
                continue;
            }

            for (const auto& token : Tokens) {
                if (Consume(text, pos, token.display)) {
                    glyphs.push_back(token.byte);
                    pos += token.display.size();
                    matched = true;
                    break;
                }
            }
            if (!matched) {
                throw std::invalid_argument("unsupported UTF-8 sequence in Gen I text field");
            }
        }

        if (glyphs.size() >= fieldLength) {
            throw std::out_of_range("name is too long for the target field");
        }
        std::vector<std::uint8_t> encoded(fieldLength, Terminator);
        std::copy(glyphs.begin(), glyphs.end(), encoded.begin());
        return encoded;
    }

    static std::string DecodeName(const std::vector<std::uint8_t>& buffer,
                                  std::size_t offset,
                                  std::size_t fieldLength) {
        return DecodeNameImpl(buffer, offset, fieldLength, false);
    }

    static std::string DecodeNameLossless(const std::vector<std::uint8_t>& buffer,
                                          std::size_t offset,
                                          std::size_t fieldLength) {
        return DecodeNameImpl(buffer, offset, fieldLength, true);
    }

    static void WriteName(std::vector<std::uint8_t>& buffer,
                          std::size_t offset,
                          std::size_t fieldLength,
                          std::string_view text) {
        PrimitiveWriter::WriteBytes(buffer, offset, EncodeName(text, fieldLength));
    }

private:
    struct Token {
        std::uint8_t byte;
        std::string_view display;
        std::string_view lossless;
    };

    inline static constexpr std::array<Token, 33> Tokens = {{
        {0x5B, "PC", "<PC>"},
        {0x5C, "TM", "<TM>"},
        {0x5D, "TRAINER", "<TRAINER>"},
        {0x5E, "ROCKET", "<ROCKET>"},
        {0x7F, " ", " "},
        {0x9A, "(", "("},
        {0x9B, ")", ")"},
        {0x9C, ":", ":"},
        {0x9D, ";", ";"},
        {0x9E, "[", "["},
        {0x9F, "]", "]"},
        {0xBA, "é", "é"},
        {0xBB, "'d", "<APOS_D>"},
        {0xBC, "'l", "<APOS_L>"},
        {0xBD, "'s", "<APOS_S>"},
        {0xBE, "'t", "<APOS_T>"},
        {0xBF, "'v", "<APOS_V>"},
        {0xE0, "'", "'"},
        {0xE1, "PK", "<PK>"},
        {0xE2, "MN", "<MN>"},
        {0xE3, "-", "-"},
        {0xE4, "'r", "<APOS_R>"},
        {0xE5, "'m", "<APOS_M>"},
        {0xE6, "?", "?"},
        {0xE7, "!", "!"},
        {0xE8, ".", "<PERIOD>"},
        {0xEF, "♂", "♂"},
        {0xF0, "¥", "¥"},
        {0xF1, "×", "×"},
        {0xF2, ".", "<DOT>"},
        {0xF3, "/", "/"},
        {0xF4, ",", ","},
        {0xF5, "♀", "♀"},
    }};

    static bool Consume(std::string_view input, std::size_t pos, std::string_view token) {
        return pos + token.size() <= input.size() && input.substr(pos, token.size()) == token;
    }

    static int HexNibble(char c) {
        if (c >= '0' && c <= '9') return c - '0';
        if (c >= 'A' && c <= 'F') return 10 + c - 'A';
        if (c >= 'a' && c <= 'f') return 10 + c - 'a';
        return -1;
    }

    static bool ConsumeRawByteToken(std::string_view input,
                                    std::size_t pos,
                                    std::uint8_t* byte) {
        if (pos + 6U > input.size() || input.substr(pos, 3U) != "<0x" ||
            input[pos + 5U] != '>') {
            return false;
        }
        const int high = HexNibble(input[pos + 3U]);
        const int low = HexNibble(input[pos + 4U]);
        if (high < 0 || low < 0) return false;
        *byte = static_cast<std::uint8_t>((high << 4) | low);
        return true;
    }

    static std::string TokenForByte(std::uint8_t byte, bool lossless) {
        if (byte >= 0x80 && byte <= 0x99) {
            return std::string(1, static_cast<char>('A' + (byte - 0x80)));
        }
        if (byte >= 0xA0 && byte <= 0xB9) {
            return std::string(1, static_cast<char>('a' + (byte - 0xA0)));
        }
        if (byte >= 0xF6 && byte <= 0xFF) {
            return std::string(1, static_cast<char>('0' + (byte - 0xF6)));
        }
        for (const auto& token : Tokens) {
            if (token.byte == byte) {
                return std::string(lossless ? token.lossless : token.display);
            }
        }
        std::ostringstream oss;
        oss << "<0x" << std::uppercase << std::hex << std::setw(2)
            << std::setfill('0') << static_cast<int>(byte) << ">";
        return oss.str();
    }

    static std::string DecodeNameImpl(const std::vector<std::uint8_t>& buffer,
                                      std::size_t offset,
                                      std::size_t fieldLength,
                                      bool lossless) {
        PrimitiveWriter::EnsureRange(buffer, offset, fieldLength);
        std::string decoded;
        for (std::size_t i = 0; i < fieldLength; ++i) {
            const std::uint8_t byte = buffer[offset + i];
            if (byte == Terminator) {
                break;
            }
            decoded += TokenForByte(byte, lossless);
        }
        return decoded;
    }
};

}  // namespace pkmn::savegen::encoding
