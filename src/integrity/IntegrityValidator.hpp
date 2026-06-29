#pragma once

#include <string>
#include <vector>

#include "../encoding/Gen1Layout.hpp"
#include "ChecksumAlgorithms.hpp"

namespace pkmn::savegen::integrity {

struct IntegrityValidationResult {
    bool sizeOk = false;
    bool mainChecksumValid = false;
    bool bank2ChecksumValid = false;
    bool bank3ChecksumValid = false;
    bool bank2MatchesPolicy = false;
    bool bank3MatchesPolicy = false;
    bool bankStorageUnchanged = false;
    bool ok = false;
    std::vector<std::string> warnings;
    std::vector<std::string> errors;
};

class IntegrityValidator {
public:
    static IntegrityValidationResult ValidateMilestone2(
        const std::vector<std::uint8_t>& outputBytes,
        const std::vector<std::uint8_t>& templateBytes,
        bool expectBankAllValid) {
        IntegrityValidationResult result;
        result.sizeOk = outputBytes.size() == encoding::Gen1Layout::ExpectedSaveSize;
        result.mainChecksumValid = ChecksumAlgorithms::ValidateMainChecksum(outputBytes);
        result.bank2ChecksumValid = ChecksumAlgorithms::ValidateBank2AllChecksum(outputBytes);
        result.bank3ChecksumValid = ChecksumAlgorithms::ValidateBank3AllChecksum(outputBytes);
        result.bank2MatchesPolicy = result.bank2ChecksumValid == expectBankAllValid;
        result.bank3MatchesPolicy = result.bank3ChecksumValid == expectBankAllValid;
        constexpr std::size_t kBankStart = encoding::Gen1Layout::Bank2PayloadStart;
        constexpr std::size_t kBankEndInclusive = encoding::Gen1Layout::Bank3AllChecksumOff + 6;
        result.bankStorageUnchanged = CompareRanges(
            outputBytes, templateBytes, kBankStart, (kBankEndInclusive - kBankStart + 1));

        if (!result.sizeOk) {
            result.errors.push_back("Output save size is not 0x8000.");
        }
        if (!result.mainChecksumValid) {
            result.errors.push_back("Main checksum is invalid.");
        }
        if (!result.bank2MatchesPolicy) {
            result.errors.push_back("Bank 2 all-box checksum does not match the active Policy A expectation.");
        }
        if (!result.bank3MatchesPolicy) {
            result.errors.push_back("Bank 3 all-box checksum does not match the active Policy A expectation.");
        }
        if (!result.bankStorageUnchanged) {
            result.errors.push_back("Policy A requires the bank 2/3 permanent box storage region to remain byte-identical to the committed dummy.");
        }
        result.ok = result.errors.empty();
        return result;
    }

private:
    static bool CompareRanges(const std::vector<std::uint8_t>& left,
                              const std::vector<std::uint8_t>& right,
                              std::size_t start,
                              std::size_t len) {
        if (left.size() < start + len || right.size() < start + len) {
            return false;
        }
        for (std::size_t i = 0; i < len; ++i) {
            if (left[start + i] != right[start + i]) {
                return false;
            }
        }
        return true;
    }
};

}  // namespace pkmn::savegen::integrity
