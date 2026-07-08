#pragma once

#include <string>
#include <vector>

#include "../../third_party/nlohmann/json.hpp"

namespace pkmn::savegen::input {

struct RedJsonValidationResult {
    bool ok = false;
    std::string format;
    std::string schemaVersion;
    std::string targetGame;
    std::string regionAssumption;
    bool physicalImagePresent = false;
    bool physicalImageWellFormed = false;
    std::vector<std::string> warnings;
    std::vector<std::string> errors;
};

class RedJsonValidator {
public:
    static RedJsonValidationResult Validate(const nlohmann::json& document) {
        RedJsonValidationResult result;

        auto require_path = [&result](const nlohmann::json& node,
                                      const std::vector<std::string>& path,
                                      const std::string& label) -> const nlohmann::json* {
            const nlohmann::json* current = &node;
            for (const std::string& part : path) {
                if (!current->is_object() || !current->contains(part)) {
                    result.errors.push_back("Missing required field: " + label);
                    return nullptr;
                }
                current = &current->at(part);
            }
            return current;
        };

        const nlohmann::json* schema = require_path(document, {"schema"}, "schema");
        static_cast<void>(require_path(document, {"source", "hashes", "wholeFileSha256"}, "source.hashes.wholeFileSha256"));
        const nlohmann::json* decoded = require_path(document, {"decoded"}, "decoded");

        if (schema != nullptr) {
            result.format = schema->value("format", "");
            result.schemaVersion = schema->value("schemaVersion", "");
            result.targetGame = schema->value("game", "");
            result.regionAssumption = schema->value("regionAssumption", "");

            if (result.format != "pkmn-red-master-save") {
                result.errors.push_back("Unsupported schema.format: " + result.format);
            }
            if (result.schemaVersion != "0.1.0") {
                result.errors.push_back("Unsupported schema.schemaVersion: " + result.schemaVersion);
            }
            if (result.targetGame != "Pokemon Red") {
                result.errors.push_back("Unsupported schema.game: " + result.targetGame);
            }
        }

        if (document.contains("physicalImage")) {
            result.physicalImagePresent = true;
            result.physicalImageWellFormed = document.at("physicalImage").is_object();
            if (!result.physicalImageWellFormed) {
                result.warnings.push_back("physicalImage is present but not an object; it will be ignored.");
            }
        } else {
            result.warnings.push_back("physicalImage is absent; semantic-only generation remains allowed.");
        }

        if (decoded != nullptr) {
            const std::vector<std::vector<std::string>> requiredPaths = {
                {"trainer"}, {"rival"}, {"options"}, {"playtime"}, {"moneyAndCoins"},
                {"badges"}, {"location"}, {"runtimeState"}, {"pokedex"}, {"inventory"}, {"party"},
                {"pcStorage"}, {"currentBoxCache"}, {"daycare"}, {"hallOfFame"},
                {"events"}, {"trainerBattles"}, {"staticBattles"}, {"storyProgress"},
                {"scripts"}, {"missableObjects"}, {"visitedTowns"}, {"hiddenItems"},
                {"hiddenCoins"}, {"worldState"}
            };
            for (const auto& path : requiredPaths) {
                const std::string label = "decoded." + path.front();
                static_cast<void>(require_path(*decoded, path, label));
            }
        }

        result.ok = result.errors.empty();
        return result;
    }
};

}  // namespace pkmn::savegen::input
