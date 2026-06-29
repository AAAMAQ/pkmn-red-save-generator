#pragma once

#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <vector>

#include "PrimitiveWriter.hpp"

namespace pkmn::savegen::encoding {

class BitfieldWriter {
public:
    static bool ReadBit(const std::vector<std::uint8_t>& buffer, std::size_t byteOffset, std::uint8_t bitIndex) {
        if (bitIndex > 7) {
            throw std::out_of_range("bit index must be 0..7");
        }
        const std::uint8_t byte = PrimitiveWriter::ReadU8(buffer, byteOffset);
        return (byte & static_cast<std::uint8_t>(1U << bitIndex)) != 0;
    }

    static void WriteBit(std::vector<std::uint8_t>& buffer,
                         std::size_t byteOffset,
                         std::uint8_t bitIndex,
                         bool value) {
        if (bitIndex > 7) {
            throw std::out_of_range("bit index must be 0..7");
        }
        PrimitiveWriter::EnsureRange(buffer, byteOffset, 1);
        if (value) {
            buffer[byteOffset] |= static_cast<std::uint8_t>(1U << bitIndex);
        } else {
            buffer[byteOffset] &= static_cast<std::uint8_t>(~(1U << bitIndex));
        }
    }
};

}  // namespace pkmn::savegen::encoding
