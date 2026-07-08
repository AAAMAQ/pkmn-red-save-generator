#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

#include "../encoding/Gen1Layout.hpp"
#include "../encoding/PrimitiveWriter.hpp"
#include "ChecksumAlgorithms.hpp"

namespace pkmn::savegen::integrity {

class BoxChecksumWriter {
public:
    static void WriteAll(std::vector<std::uint8_t>& bytes) {
        for (int boxIndex = 1; boxIndex <= 12; ++boxIndex) {
            const std::size_t tableBase =
                (boxIndex <= 6) ? encoding::Gen1Layout::Bank2BoxChecksumsOff
                                : encoding::Gen1Layout::Bank3BoxChecksumsOff;
            const int withinBank = (boxIndex <= 6) ? (boxIndex - 1) : (boxIndex - 7);
            encoding::PrimitiveWriter::WriteU8(
                bytes,
                tableBase + static_cast<std::size_t>(withinBank),
                ChecksumAlgorithms::ComputeBoxChecksum(bytes, boxIndex));
        }

        encoding::PrimitiveWriter::WriteU8(
            bytes,
            encoding::Gen1Layout::Bank2AllChecksumOff,
            ChecksumAlgorithms::ComputeBank2AllChecksum(bytes));
        encoding::PrimitiveWriter::WriteU8(
            bytes,
            encoding::Gen1Layout::Bank3AllChecksumOff,
            ChecksumAlgorithms::ComputeBank3AllChecksum(bytes));
    }
};

}  // namespace pkmn::savegen::integrity
