#pragma once

#include <cstddef>
#include <string>
#include <vector>

#include "../../third_party/nlohmann/json.hpp"

namespace pkmn::savegen::reporting {

struct RangeLedgerEntry {
    std::size_t start = 0;
    std::size_t endInclusive = 0;
    std::string classification;
    std::string reason;
};

struct GenerationReport {
    std::string generatorVersion;
    std::string targetJsonPath;
    std::string targetSourceSha256;
    std::string profileId;
    std::string templatePath;
    std::string templateSha256;
    std::string outputPath;
    std::string outputSha256;
    std::string dummyBoxPolicy;
    bool physicalImageIgnored = false;
    std::size_t outputSize = 0;
    std::vector<RangeLedgerEntry> ranges;
    std::vector<std::string> fieldsWritten;
    std::vector<std::string> warnings;

    nlohmann::json ToJson() const {
        nlohmann::json json;
        json["generatorVersion"] = generatorVersion;
        json["targetJsonPath"] = targetJsonPath;
        json["targetSourceSha256"] = targetSourceSha256;
        json["profileId"] = profileId;
        json["templatePath"] = templatePath;
        json["templateSha256"] = templateSha256;
        json["outputPath"] = outputPath;
        json["outputSha256"] = outputSha256;
        json["outputSize"] = outputSize;
        json["dummyBoxPolicy"] = dummyBoxPolicy;
        json["physicalImageIgnored"] = physicalImageIgnored;
        json["fieldsWritten"] = fieldsWritten;
        json["warnings"] = warnings;
        json["ranges"] = nlohmann::json::array();
        for (const auto& range : ranges) {
            json["ranges"].push_back({
                {"start", range.start},
                {"endInclusive", range.endInclusive},
                {"classification", range.classification},
                {"reason", range.reason}
            });
        }
        return json;
    }
};

}  // namespace pkmn::savegen::reporting
