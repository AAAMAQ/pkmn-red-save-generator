#pragma once

#include <array>
#include <string>
#include <vector>

#include "../encoding/Gen1Layout.hpp"
#include "../encoding/PrimitiveWriter.hpp"
#include "../pokemon/Gen1PokemonData.hpp"
#include "ChecksumAlgorithms.hpp"

namespace pkmn::savegen::integrity {

struct IntegrityValidationResult {
    bool sizeOk = false;
    bool mainChecksumValid = false;
    bool bank2ChecksumValid = false;
    bool bank3ChecksumValid = false;
    std::array<bool, 12> boxChecksumsValid{};
    std::array<bool, 12> boxStructuresValid{};
    bool selectedBoxValid = false;
    bool currentBoxCacheValid = false;
    bool currentBoxCacheMatchesSelectedPermanent = false;
    // Backward-compatible report field; now means structurally coherent rather
    // than byte-identical to the permanent box.
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
            result.boxStructuresValid[static_cast<std::size_t>(boxIndex - 1)] =
                ValidateBoxStructure(
                    outputBytes,
                    encoding::Gen1Layout::PermanentBoxOffsets[
                        static_cast<std::size_t>(boxIndex - 1)]);
        }
        const std::uint8_t rawCurrentBox = encoding::PrimitiveWriter::ReadU8(
            outputBytes, encoding::Gen1Layout::CurrentBoxByteOff);
        const int selectedBox = static_cast<int>(rawCurrentBox & 0x7FU) + 1;
        result.selectedBoxValid = selectedBox >= 1 && selectedBox <= 12;
        result.currentBoxCacheValid = ValidateBoxStructure(
            outputBytes, encoding::Gen1Layout::CurrentBoxCacheOff);
        result.currentBoxCacheMatchesSelectedPermanent =
            SelectedPermanentBoxMatchesCache(outputBytes);
        result.currentBoxCacheSynchronized = result.currentBoxCacheValid;

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
            if (!result.boxStructuresValid[static_cast<std::size_t>(boxIndex - 1)]) {
                result.errors.push_back(
                    "Permanent box structure is invalid for box " +
                    std::to_string(boxIndex) + ".");
            }
        }
        if (!result.selectedBoxValid) {
            result.errors.push_back("Selected current-box number is outside 1..12.");
        }
        if (!result.currentBoxCacheValid) {
            result.errors.push_back(
                "Current-box cache is structurally invalid.");
        } else if (!result.currentBoxCacheMatchesSelectedPermanent) {
            result.warnings.push_back(
                "Current-box cache differs from the selected permanent box; this is valid Gen I working-box state and is preserved intentionally.");
        }
        result.ok = result.errors.empty();
        return result;
    }

private:
    static bool HasTerminator(const std::vector<std::uint8_t>& bytes,
                              std::size_t offset,
                              std::size_t length) {
        encoding::PrimitiveWriter::EnsureRange(bytes, offset, length);
        for (std::size_t i = 0; i < length; ++i) {
            if (bytes[offset + i] == 0x50U) {
                return true;
            }
        }
        return false;
    }

    static bool ValidateBoxStructure(const std::vector<std::uint8_t>& bytes,
                                     std::size_t base) {
        try {
            encoding::PrimitiveWriter::EnsureRange(
                bytes, base, encoding::Gen1Layout::BoxBlockSize);
            const std::uint8_t count = encoding::PrimitiveWriter::ReadU8(
                bytes, base + encoding::Gen1Layout::BoxCountRel);
            if (count > encoding::Gen1Layout::BoxMaxMons) {
                return false;
            }
            if (encoding::PrimitiveWriter::ReadU8(
                    bytes,
                    base + encoding::Gen1Layout::BoxSpeciesRel + count) != 0xFFU) {
                return false;
            }
            for (std::size_t i = 0; i < count; ++i) {
                const std::uint8_t species = encoding::PrimitiveWriter::ReadU8(
                    bytes, base + encoding::Gen1Layout::BoxSpeciesRel + i);
                if (pokemon::FindSpeciesData(species) == nullptr) {
                    return false;
                }
                const std::size_t structBase = base + encoding::Gen1Layout::BoxStructsRel
                    + (i * encoding::Gen1Layout::BoxStructSize);
                if (encoding::PrimitiveWriter::ReadU8(
                        bytes, structBase + encoding::Gen1Layout::BoxMonSpeciesRel) != species) {
                    return false;
                }
                const std::uint8_t level = encoding::PrimitiveWriter::ReadU8(
                    bytes, structBase + encoding::Gen1Layout::BoxMonLevelRel);
                if (level == 0U || level > 100U) {
                    return false;
                }
                if (!HasTerminator(
                        bytes,
                        base + encoding::Gen1Layout::BoxOTNamesRel
                            + (i * encoding::Gen1Layout::Gen1NameLen),
                        encoding::Gen1Layout::Gen1NameLen) ||
                    !HasTerminator(
                        bytes,
                        base + encoding::Gen1Layout::BoxNicknamesRel
                            + (i * encoding::Gen1Layout::Gen1NameLen),
                        encoding::Gen1Layout::Gen1NameLen)) {
                    return false;
                }
            }
            return true;
        } catch (const std::exception&) {
            return false;
        }
    }

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
