#pragma once

#include <string>

#include "../../third_party/nlohmann/json.hpp"

namespace pkmn::savegen::input {

struct SanitizedRedJson {
    nlohmann::json document;
    bool physicalImageWasPresent = false;
    bool physicalImageRemoved = false;
    std::string ignoredPhysicalImageType = "absent";
};

class PhysicalImageIsolationGuard {
public:
    static SanitizedRedJson Sanitize(const nlohmann::json& document) {
        SanitizedRedJson sanitized;
        sanitized.document = document;
        sanitized.physicalImageWasPresent = sanitized.document.contains("physicalImage");
        if (sanitized.physicalImageWasPresent) {
            sanitized.ignoredPhysicalImageType = sanitized.document.at("physicalImage").type_name();
            sanitized.document.erase("physicalImage");
            sanitized.physicalImageRemoved = true;
        }
        return sanitized;
    }
};

}  // namespace pkmn::savegen::input
