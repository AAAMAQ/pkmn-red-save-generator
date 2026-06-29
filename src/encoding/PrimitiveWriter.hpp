#pragma once

#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <vector>

namespace pkmn::savegen::encoding {

class PrimitiveWriter {
public:
    static void EnsureRange(const std::vector<std::uint8_t>& buffer, std::size_t offset, std::size_t length) {
        if (offset > buffer.size()) {
            throw std::out_of_range("offset is outside the buffer");
        }
        if (length == 0) {
            return;
        }
        if (offset + length < offset || offset + length > buffer.size()) {
            throw std::out_of_range("range exceeds buffer size");
        }
    }

    static std::uint8_t ReadU8(const std::vector<std::uint8_t>& buffer, std::size_t offset) {
        EnsureRange(buffer, offset, 1);
        return buffer[offset];
    }

    static void WriteU8(std::vector<std::uint8_t>& buffer, std::size_t offset, std::uint8_t value) {
        EnsureRange(buffer, offset, 1);
        buffer[offset] = value;
    }

    static std::uint16_t ReadU16BigEndian(const std::vector<std::uint8_t>& buffer, std::size_t offset) {
        EnsureRange(buffer, offset, 2);
        return static_cast<std::uint16_t>((buffer[offset] << 8U) | buffer[offset + 1]);
    }

    static void WriteU16BigEndian(std::vector<std::uint8_t>& buffer, std::size_t offset, std::uint16_t value) {
        EnsureRange(buffer, offset, 2);
        buffer[offset] = static_cast<std::uint8_t>((value >> 8U) & 0xFFU);
        buffer[offset + 1] = static_cast<std::uint8_t>(value & 0xFFU);
    }

    static std::uint32_t ReadU24BigEndian(const std::vector<std::uint8_t>& buffer, std::size_t offset) {
        EnsureRange(buffer, offset, 3);
        return (static_cast<std::uint32_t>(buffer[offset]) << 16U) |
               (static_cast<std::uint32_t>(buffer[offset + 1]) << 8U) |
               static_cast<std::uint32_t>(buffer[offset + 2]);
    }

    static void WriteU24BigEndian(std::vector<std::uint8_t>& buffer, std::size_t offset, std::uint32_t value) {
        if (value > 0xFFFFFFU) {
            throw std::out_of_range("24-bit value overflow");
        }
        EnsureRange(buffer, offset, 3);
        buffer[offset] = static_cast<std::uint8_t>((value >> 16U) & 0xFFU);
        buffer[offset + 1] = static_cast<std::uint8_t>((value >> 8U) & 0xFFU);
        buffer[offset + 2] = static_cast<std::uint8_t>(value & 0xFFU);
    }

    static void WriteBytes(std::vector<std::uint8_t>& buffer,
                           std::size_t offset,
                           const std::vector<std::uint8_t>& values) {
        EnsureRange(buffer, offset, values.size());
        for (std::size_t i = 0; i < values.size(); ++i) {
            buffer[offset + i] = values[i];
        }
    }
};

}  // namespace pkmn::savegen::encoding
