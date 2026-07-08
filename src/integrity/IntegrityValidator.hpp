#pragma once

#include <array>
#include <string>
#include <vector>

#include "../encoding/Gen1Layout.hpp"
#include "../encoding/PrimitiveWriter.hpp"
#include "ChecksumAlgorithms.hpp"

namespace pkmn::savegen::integrity {

struct IntegrityValidationResult {
    bool sizeOk = false;
    bool mainChecksumValid = false;
    bool bank2ChecksumValid = false;
    bool bank3ChecksumValid = false;
    std::array<bool, 12> boxChecksumsValid{};
    bool currentBoxCacheSynchronized = false;
    bool ok = false;
    std::vector<std::string> warnings;
    std::vector<std::string> errors;
};

class IntegrityValidator {
public:
    static IntegrityValidationResult ValidateGeneratedSave(
        const std::vector<std::uint8_t>& outputBytes) {
        IntegrityValidationResult result;
        result.sizeOk = outputBytes.size() == encoding::Gen1Layout::ExpectedSaveSize;
        result.mainChecksumValid = ChecksumAlgorithms::ValidateMainChecksum(outputBytes);
        result.bank2ChecksumValid = ChecksumAlgorithms::ValidateBank2AllChecksum(outputBytes);
        result.bank3ChecksumValid = ChecksumAlgorithms::ValidateBank3AllChecksum(outputBytes);

        for (int boxIndex = 1; boxIndex <= 12; ++boxIndex) {
            result.boxChecksumsValid[static_cast<std::size_t>(boxIndex - 1)] =
                ChecksumAlgorithms::ValidateBoxChecksum(outputBytes, boxIndex);
        }
        result.currentBoxCacheSynchronized = SelectedPermanentBoxMatchesCache(outputBytes);

        if (!result.sizeOk) {
            result.errors.push_back("Output save size is not 0x8000.");
        }
        if (!result.mainChecksumValid) {
            result.errors.push_back("Main checksum is invalid.");
        }
        if (!result.bank2ChecksumValid) {
            result.errors.push_back("Bank 2 all-box checksum is invalid.");
        }
        if (!result.bank3ChecksumValid) {
            result.errors.push_back("Bank 3 all-box checksum is invalid.");
        }
        for (int boxIndex = 1; boxIndex <= 12; ++boxIndex) {
            if (!result.boxChecksumsValid[static_cast<std::size_t>(boxIndex - 1)]) {
                result.errors.push_back(
                    "Per-box checksum is invalid for box " + std::to_string(boxIndex) + ".");
            }
        }
        if (!result.currentBoxCacheSynchronized) {
            result.errors.push_back(
                "Current-box cache does not match the selected permanent box.");
        }
        result.ok = result.errors.empty();
        return result;
    }

private:
    static bool SelectedPermanentBoxMatchesCache(
        const std::vector<std::uint8_t>& bytes) {
        const std::uint8_t raw = encoding::PrimitiveWriter::ReadU8(
            bytes, encoding::Gen1Layout::CurrentBoxByteOff);
        const int selected = static_cast<int>(raw & 0x7FU) + 1;
        if (selected < 1 || selected > 12) {
            return false;
        }
        const std::size_t permanentBase =
            encoding::Gen1Layout::PermanentBoxOffsets[static_cast<std::size_t>(selected - 1)];
        encoding::PrimitiveWriter::EnsureRange(
            bytes, permanentBase, encoding::Gen1Layout::BoxBlockSize);
        encoding::PrimitiveWriter::EnsureRange(
            bytes, encoding::Gen1Layout::CurrentBoxCacheOff, encoding::Gen1Layout::BoxBlockSize);
        for (std::size_t i = 0; i < encoding::Gen1Layout::BoxBlockSize; ++i) {
            if (bytes[permanentBase + i] !=
                bytes[encoding::Gen1Layout::CurrentBoxCacheOff + i]) {
                return false;
            }
        }
        return true;
    }
};

}  // namespace pkmn::savegen::integrity
