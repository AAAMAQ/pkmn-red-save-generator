#pragma once

#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <vector>

#include "PrimitiveWriter.hpp"

namespace pkmn::savegen::encoding {

class BcdEncoder {
public:
    static void ValidateBcdDigit(std::uint8_t byte) {
        if (((byte >> 4U) & 0x0FU) > 9U || (byte & 0x0FU) > 9U) {
            throw std::runtime_error("invalid packed BCD digit");
        }
    }

    static std::vector<std::uint8_t> Encode3(std::uint32_t value) {
        if (value > 999999U) {
            throw std::out_of_range("3-byte BCD value exceeds 999999");
        }
        return {
            static_cast<std::uint8_t>(((value / 100000U) % 10U) << 4U | ((value / 10000U) % 10U)),
            static_cast<std::uint8_t>(((value / 1000U) % 10U) << 4U | ((value / 100U) % 10U)),
            static_cast<std::uint8_t>(((value / 10U) % 10U) << 4U | (value % 10U))
        };
    }

    static std::vector<std::uint8_t> Encode2(std::uint16_t value) {
        if (value > 9999U) {
            throw std::out_of_range("2-byte BCD value exceeds 9999");
        }
        return {
            static_cast<std::uint8_t>(((value / 1000U) % 10U) << 4U | ((value / 100U) % 10U)),
            static_cast<std::uint8_t>(((value / 10U) % 10U) << 4U | (value % 10U))
        };
    }

    static std::uint32_t Decode3(const std::vector<std::uint8_t>& buffer, std::size_t offset) {
        PrimitiveWriter::EnsureRange(buffer, offset, 3);
        ValidateBcdDigit(buffer[offset]);
        ValidateBcdDigit(buffer[offset + 1]);
        ValidateBcdDigit(buffer[offset + 2]);
        return static_cast<std::uint32_t>(((buffer[offset] >> 4U) & 0x0FU) * 100000U +
                                          (buffer[offset] & 0x0FU) * 10000U +
                                          ((buffer[offset + 1] >> 4U) & 0x0FU) * 1000U +
                                          (buffer[offset + 1] & 0x0FU) * 100U +
                                          ((buffer[offset + 2] >> 4U) & 0x0FU) * 10U +
                                          (buffer[offset + 2] & 0x0FU));
    }

    static std::uint16_t Decode2(const std::vector<std::uint8_t>& buffer, std::size_t offset) {
        PrimitiveWriter::EnsureRange(buffer, offset, 2);
        ValidateBcdDigit(buffer[offset]);
        ValidateBcdDigit(buffer[offset + 1]);
        return static_cast<std::uint16_t>(((buffer[offset] >> 4U) & 0x0FU) * 1000U +
                                          (buffer[offset] & 0x0FU) * 100U +
                                          ((buffer[offset + 1] >> 4U) & 0x0FU) * 10U +
                                          (buffer[offset + 1] & 0x0FU));
    }

    static void Write3(std::vector<std::uint8_t>& buffer, std::size_t offset, std::uint32_t value) {
        PrimitiveWriter::WriteBytes(buffer, offset, Encode3(value));
    }

    static void Write2(std::vector<std::uint8_t>& buffer, std::size_t offset, std::uint16_t value) {
        PrimitiveWriter::WriteBytes(buffer, offset, Encode2(value));
    }
};

}  // namespace pkmn::savegen::encoding
