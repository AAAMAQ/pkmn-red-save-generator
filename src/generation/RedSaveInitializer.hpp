#pragma once

#include <utility>
#include <vector>

#include "../encoding/Gen1Layout.hpp"
#include "../reporting/GenerationReport.hpp"
#include "../template/CanonicalTemplateLoader.hpp"
#include "../template/TemplateProfile.hpp"

namespace pkmn::savegen::generation {

struct WorkingSaveBuffer {
    std::vector<std::uint8_t> bytes;
    reporting::GenerationReport report;
};

class RedSaveInitializer {
public:
    static WorkingSaveBuffer Initialize(const template_loader::LoadedTemplate& loadedTemplate,
                                        const template_profile::TemplateProfile& profile,
                                        bool physicalImageIgnored) {
        WorkingSaveBuffer working;
        working.bytes = loadedTemplate.bytes;
        working.report.profileId = profile.profileId;
        working.report.templatePath = profile.templateRelativePath.string();
        working.report.templateSha256 = loadedTemplate.sha256;
        working.report.physicalImageIgnored = physicalImageIgnored;
        working.report.ranges.push_back({
            0,
            loadedTemplate.bytes.empty() ? 0 : loadedTemplate.bytes.size() - 1,
            "template-inherited",
            "Initial deterministic working copy from canonical template",
            "RedSaveInitializer",
            "template"
        });
        return working;
    }
};

}  // namespace pkmn::savegen::generation
