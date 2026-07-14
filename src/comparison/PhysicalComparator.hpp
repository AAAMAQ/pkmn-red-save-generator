#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <vector>

namespace pkmn::savegen::comparison {

struct PhysicalComparisonRegion {
    std::size_t start = 0;
    std::size_t endInclusive = 0;
    bool equal = false;
    std::size_t differingBytes = 0;
};

struct PhysicalComparisonResult {
    std::size_t originalSize = 0;
    std::size_t generatedSize = 0;
    std::size_t comparedBytes = 0;
    std::size_t equalBytes = 0;
    std::size_t differingBytes = 0;
    std::size_t firstDifference = std::numeric_limits<std::size_t>::max();
    std::size_t lastDifference = std::numeric_limits<std::size_t>::max();
    std::vector<PhysicalComparisonRegion> regions;

    bool byteIdentical() const {
        return originalSize == generatedSize && differingBytes == 0U;
    }
};

class PhysicalComparator {
public:
    static PhysicalComparisonResult Compare(
        const std::vector<std::uint8_t>& original,
        const std::vector<std::uint8_t>& generated) {
        PhysicalComparisonResult result;
        result.originalSize = original.size();
        result.generatedSize = generated.size();
        result.comparedBytes = std::max(original.size(), generated.size());
        if (result.comparedBytes == 0U) return result;

        bool currentEqual = ByteEqual(original, generated, 0U);
        PhysicalComparisonRegion current{0U, 0U, currentEqual, currentEqual ? 0U : 1U};
        CountByte(result, currentEqual, 0U);

        for (std::size_t offset = 1U; offset < result.comparedBytes; ++offset) {
            const bool equal = ByteEqual(original, generated, offset);
            CountByte(result, equal, offset);
            if (equal == current.equal) {
                current.endInclusive = offset;
                if (!equal) ++current.differingBytes;
                continue;
            }
            result.regions.push_back(current);
            current = {offset, offset, equal, equal ? 0U : 1U};
        }
        result.regions.push_back(current);
        return result;
    }

private:
    static bool ByteEqual(const std::vector<std::uint8_t>& a,
                          const std::vector<std::uint8_t>& b,
                          std::size_t offset) {
        return offset < a.size() && offset < b.size() && a[offset] == b[offset];
    }

    static void CountByte(PhysicalComparisonResult& result,
                          bool equal,
                          std::size_t offset) {
        if (equal) {
            ++result.equalBytes;
            return;
        }
        ++result.differingBytes;
        if (result.firstDifference == std::numeric_limits<std::size_t>::max()) {
            result.firstDifference = offset;
        }
        result.lastDifference = offset;
    }
};

}  // namespace pkmn::savegen::comparison
