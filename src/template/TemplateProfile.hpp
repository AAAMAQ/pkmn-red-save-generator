#pragma once

#include <filesystem>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "../../third_party/nlohmann/json.hpp"

namespace pkmn::savegen::template_profile {

struct ChecksumExpectation {
    bool requiredValid = false;
    std::string statusLabel;
};

struct TemplateProfile {
    std::string profileId;
    std::string targetGame;
    std::string regionAssumption;
    std::string revisionAssumption;
    std::size_t saveSize = 0;
    std::filesystem::path templateRelativePath;
    std::string templateSha256;
    std::vector<std::string> supportedSchemaVersions;
    ChecksumExpectation mainChecksumExpectation;
    ChecksumExpectation bank2ChecksumExpectation;
    ChecksumExpectation bank3ChecksumExpectation;
    std::string outputLengthPolicy;
    std::string trailingDataPolicy;
    std::vector<std::string> knownLimitations;
    std::filesystem::path sourcePath;

    bool SupportsSchemaVersion(const std::string& schemaVersion) const {
        for (const std::string& version : supportedSchemaVersions) {
            if (version == schemaVersion) {
                return true;
            }
        }
        return false;
    }

    std::filesystem::path ResolveTemplatePath() const {
        return sourcePath.parent_path().parent_path() / templateRelativePath;
    }
};

class TemplateProfileLoader {
public:
    static TemplateProfile LoadFromFile(const std::filesystem::path& path) {
        std::ifstream input(path);
        if (!input) {
            throw std::runtime_error("failed to open template profile: " + path.string());
        }
        nlohmann::json json;
        input >> json;

        TemplateProfile profile;
        profile.profileId = json.at("profileId").get<std::string>();
        profile.targetGame = json.at("targetGame").get<std::string>();
        profile.regionAssumption = json.at("regionAssumption").get<std::string>();
        profile.revisionAssumption = json.at("revisionAssumption").get<std::string>();
        profile.saveSize = json.at("saveSize").get<std::size_t>();
        profile.templateRelativePath = json.at("templateRelativePath").get<std::string>();
        profile.templateSha256 = json.at("templateSha256").get<std::string>();
        profile.supportedSchemaVersions = json.at("supportedSchemaVersions").get<std::vector<std::string>>();
        profile.mainChecksumExpectation.requiredValid =
            json.at("mainChecksumExpectation").at("requiredValid").get<bool>();
        profile.mainChecksumExpectation.statusLabel =
            json.at("mainChecksumExpectation").value("statusLabel", "");
        profile.bank2ChecksumExpectation.requiredValid =
            json.at("bank2ChecksumExpectation").at("requiredValid").get<bool>();
        profile.bank2ChecksumExpectation.statusLabel =
            json.at("bank2ChecksumExpectation").value("statusLabel", "");
        profile.bank3ChecksumExpectation.requiredValid =
            json.at("bank3ChecksumExpectation").at("requiredValid").get<bool>();
        profile.bank3ChecksumExpectation.statusLabel =
            json.at("bank3ChecksumExpectation").value("statusLabel", "");
        profile.outputLengthPolicy = json.at("outputLengthPolicy").get<std::string>();
        profile.trailingDataPolicy = json.at("trailingDataPolicy").get<std::string>();
        profile.knownLimitations = json.at("knownLimitations").get<std::vector<std::string>>();
        profile.sourcePath = path;
        return profile;
    }
};

}  // namespace pkmn::savegen::template_profile
