#pragma once

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

#include "../encoding/Gen1Layout.hpp"
#include "../encoding/PrimitiveWriter.hpp"
#include "../integrity/ChecksumAlgorithms.hpp"

namespace pkmn::savegen::template_analysis {

struct BaselineAnalysis {
    bool mainChecksumValid = false;
    bool bank2AllChecksumValid = false;
    bool bank3AllChecksumValid = false;
    std::uint8_t currentBoxRawByte = 0;
    int selectedBoxNumber = 1;
    bool boxChangedFlag = false;
    std::uint8_t currentBoxCacheCount = 0;
    std::array<std::uint8_t, 12> permanentBoxCounts{};
    std::size_t currentCacheDifferenceCount = 0;
    bool permanentBoxesSuspicious = false;
    bool cacheDiffersFromPermanent = false;
    std::vector<std::string> warnings;
};

class TemplateBaselineAnalyzer {
public:
    static BaselineAnalysis Analyze(const std::vector<std::uint8_t>& bytes) {
        using encoding::Gen1Layout;
        using encoding::PrimitiveWriter;

        BaselineAnalysis analysis;
        analysis.mainChecksumValid = integrity::ChecksumAlgorithms::ValidateMainChecksum(bytes);
        analysis.bank2AllChecksumValid = integrity::ChecksumAlgorithms::ValidateBank2AllChecksum(bytes);
        analysis.bank3AllChecksumValid = integrity::ChecksumAlgorithms::ValidateBank3AllChecksum(bytes);
        analysis.currentBoxRawByte = PrimitiveWriter::ReadU8(bytes, Gen1Layout::CurrentBoxByteOff);
        analysis.selectedBoxNumber = static_cast<int>(analysis.currentBoxRawByte & 0x7FU) + 1;
        analysis.boxChangedFlag = (analysis.currentBoxRawByte & 0x80U) != 0;
        analysis.currentBoxCacheCount = PrimitiveWriter::ReadU8(bytes, Gen1Layout::CurrentBoxCacheOff);

        for (std::size_t i = 0; i < Gen1Layout::PermanentBoxOffsets.size(); ++i) {
            analysis.permanentBoxCounts[i] =
                PrimitiveWriter::ReadU8(bytes, Gen1Layout::PermanentBoxOffsets[i] + Gen1Layout::BoxCountRel);
        }

        const std::size_t currentBoxIndex =
            static_cast<std::size_t>(std::clamp(analysis.selectedBoxNumber, 1, 12) - 1);
        const std::size_t permanentBoxOffset = Gen1Layout::PermanentBoxOffsets[currentBoxIndex];
        for (std::size_t i = 0; i < Gen1Layout::CurrentBoxCacheLen; ++i) {
            if (bytes[Gen1Layout::CurrentBoxCacheOff + i] != bytes[permanentBoxOffset + i]) {
                ++analysis.currentCacheDifferenceCount;
            }
        }
        analysis.cacheDiffersFromPermanent = analysis.currentCacheDifferenceCount > 0;

        const bool allPermanentCountsTwenty =
            std::all_of(analysis.permanentBoxCounts.begin(),
                        analysis.permanentBoxCounts.end(),
                        [](std::uint8_t count) { return count == 20; });
        const bool allPermanentCountsFF =
            std::all_of(analysis.permanentBoxCounts.begin(),
                        analysis.permanentBoxCounts.end(),
                        [](std::uint8_t count) { return count == 0xFFU; });
        analysis.permanentBoxesSuspicious = allPermanentCountsTwenty || allPermanentCountsFF;

        if (allPermanentCountsTwenty) {
            analysis.warnings.push_back(
                "All 12 permanent PC boxes decode with count=20; template storage state is suspicious.");
        }
        if (allPermanentCountsFF) {
            analysis.warnings.push_back(
                "All 12 permanent PC box leading bytes are 0xFF; template storage appears uninitialized or stale.");
        }
        if (!analysis.bank2AllChecksumValid) {
            analysis.warnings.push_back("Bank 2 all-box checksum is invalid.");
        }
        if (!analysis.bank3AllChecksumValid) {
            analysis.warnings.push_back("Bank 3 all-box checksum is invalid.");
        }
        if (analysis.currentBoxCacheCount == 0 && analysis.permanentBoxesSuspicious) {
            analysis.warnings.push_back(
                "Current-box cache is empty while permanent storage appears full; do not trust box inheritance.");
        }
        if (analysis.cacheDiffersFromPermanent) {
            analysis.warnings.push_back("Current-box cache differs from the selected permanent box copy.");
        }

        return analysis;
    }
};

}  // namespace pkmn::savegen::template_analysis
