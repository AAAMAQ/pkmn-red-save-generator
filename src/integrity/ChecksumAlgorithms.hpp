#pragma once

#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <vector>

#include "../encoding/Gen1Layout.hpp"
#include "../encoding/PrimitiveWriter.hpp"

namespace pkmn::savegen::integrity {

class ChecksumAlgorithms {
public:
    static std::uint8_t ComputeComplementOfSum(const std::vector<std::uint8_t>& buffer,
                                               std::size_t start,
                                               std::size_t endInclusive) {
        if (start > endInclusive) {
            throw std::invalid_argument("invalid checksum range");
        }
        encoding::PrimitiveWriter::EnsureRange(buffer, start, endInclusive - start + 1);
        std::uint32_t sum = 0;
        for (std::size_t offset = start; offset <= endInclusive; ++offset) {
            sum += buffer[offset];
        }
        return static_cast<std::uint8_t>(0xFFU - (sum & 0xFFU));
    }

    static std::uint8_t ComputeMainChecksum(const std::vector<std::uint8_t>& buffer) {
        return ComputeComplementOfSum(
            buffer,
            encoding::Gen1Layout::MainChecksumStart,
            encoding::Gen1Layout::MainChecksumEndInclusive);
    }

    static bool ValidateMainChecksum(const std::vector<std::uint8_t>& buffer) {
        return encoding::PrimitiveWriter::ReadU8(buffer, encoding::Gen1Layout::MainChecksumOff) ==
               ComputeMainChecksum(buffer);
    }

    static std::uint8_t ComputeBank2AllChecksum(const std::vector<std::uint8_t>& buffer) {
        return ComputeComplementOfSum(
            buffer,
            encoding::Gen1Layout::Bank2PayloadStart,
            encoding::Gen1Layout::Bank2PayloadEndInclusive);
    }

    static std::uint8_t ComputeBank3AllChecksum(const std::vector<std::uint8_t>& buffer) {
        return ComputeComplementOfSum(
            buffer,
            encoding::Gen1Layout::Bank3PayloadStart,
            encoding::Gen1Layout::Bank3PayloadEndInclusive);
    }

    static bool ValidateBank2AllChecksum(const std::vector<std::uint8_t>& buffer) {
        return encoding::PrimitiveWriter::ReadU8(buffer, encoding::Gen1Layout::Bank2AllChecksumOff) ==
               ComputeBank2AllChecksum(buffer);
    }

    static bool ValidateBank3AllChecksum(const std::vector<std::uint8_t>& buffer) {
        return encoding::PrimitiveWriter::ReadU8(buffer, encoding::Gen1Layout::Bank3AllChecksumOff) ==
               ComputeBank3AllChecksum(buffer);
    }

    static std::uint8_t ComputeBoxChecksum(const std::vector<std::uint8_t>& buffer,
                                           int boxIndex1to12) {
        if (boxIndex1to12 < 1 || boxIndex1to12 > 12) {
            throw std::invalid_argument("box index must be in 1..12");
        }
        const std::size_t start = encoding::Gen1Layout::PermanentBoxOffsets[static_cast<std::size_t>(boxIndex1to12 - 1)];
        return ComputeComplementOfSum(buffer, start, start + encoding::Gen1Layout::BoxBlockSize - 1U);
    }

    static bool ValidateBoxChecksum(const std::vector<std::uint8_t>& buffer,
                                    int boxIndex1to12) {
        const std::size_t tableBase =
            (boxIndex1to12 <= 6) ? encoding::Gen1Layout::Bank2BoxChecksumsOff
                                 : encoding::Gen1Layout::Bank3BoxChecksumsOff;
        const int withinBank = (boxIndex1to12 <= 6) ? (boxIndex1to12 - 1) : (boxIndex1to12 - 7);
        return encoding::PrimitiveWriter::ReadU8(
                   buffer, tableBase + static_cast<std::size_t>(withinBank)) ==
               ComputeBoxChecksum(buffer, boxIndex1to12);
    }
};

}  // namespace pkmn::savegen::integrity
