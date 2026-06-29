#pragma once

#include <string>
#include <vector>

#include "CanonicalTemplateLoader.hpp"
#include "TemplateBaselineAnalyzer.hpp"
#include "TemplateProfile.hpp"

namespace pkmn::savegen::template_validation {

struct TemplateValidationResult {
    bool accepted = false;
    bool sizeMatches = false;
    bool hashMatches = false;
    bool mainChecksumMatchesExpectation = false;
    bool bank2ChecksumMatchesExpectation = false;
    bool bank3ChecksumMatchesExpectation = false;
    template_analysis::BaselineAnalysis baseline;
    std::vector<std::string> warnings;
    std::vector<std::string> errors;
};

class TemplateValidator {
public:
    static TemplateValidationResult Validate(const template_profile::TemplateProfile& profile,
                                             const template_loader::LoadedTemplate& loaded) {
        TemplateValidationResult result;
        result.sizeMatches = loaded.bytes.size() == profile.saveSize;
        result.hashMatches = loaded.sha256 == profile.templateSha256;
        result.baseline = template_analysis::TemplateBaselineAnalyzer::Analyze(loaded.bytes);
        result.mainChecksumMatchesExpectation =
            result.baseline.mainChecksumValid == profile.mainChecksumExpectation.requiredValid;
        result.bank2ChecksumMatchesExpectation =
            result.baseline.bank2AllChecksumValid == profile.bank2ChecksumExpectation.requiredValid;
        result.bank3ChecksumMatchesExpectation =
            result.baseline.bank3AllChecksumValid == profile.bank3ChecksumExpectation.requiredValid;

        if (!result.sizeMatches) {
            result.errors.push_back("Template size does not match the active profile.");
        }
        if (!result.hashMatches) {
            result.errors.push_back("Template SHA-256 does not match the active profile.");
        }
        if (!result.mainChecksumMatchesExpectation) {
            result.errors.push_back("Main checksum state does not match the profile expectation.");
        }
        if (!result.bank2ChecksumMatchesExpectation) {
            result.errors.push_back("Bank 2 checksum state does not match the profile expectation.");
        }
        if (!result.bank3ChecksumMatchesExpectation) {
            result.errors.push_back("Bank 3 checksum state does not match the profile expectation.");
        }

        result.warnings = result.baseline.warnings;
        if (result.baseline.permanentBoxesSuspicious) {
            result.warnings.push_back(
                "Working-copy normalization or proof of harmlessness is still required before Milestone 2 acceptance.");
        }
        result.accepted = result.errors.empty();
        return result;
    }
};

}  // namespace pkmn::savegen::template_validation
