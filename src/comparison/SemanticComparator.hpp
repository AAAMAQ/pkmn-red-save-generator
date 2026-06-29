#pragma once

#include <string>
#include <vector>

#include "../model/RedSemanticState.hpp"

namespace pkmn::savegen::comparison {

enum class DifferenceCategory {
    ExactMismatch
};

struct Difference {
    DifferenceCategory category = DifferenceCategory::ExactMismatch;
    std::string fieldPath;
    std::string expectedValue;
    std::string actualValue;
};

struct ComparisonOptions {
    bool compareIdentity = true;
    bool compareCore = true;
    bool comparePokedex = true;
    bool compareInventory = true;
    bool compareParty = true;
    bool compareDaycare = true;
    bool compareHallOfFame = true;
    bool compareEventSubset = false;
};

class SemanticComparator {
public:
    static std::vector<Difference> CompareOwnedFields(const model::RedSemanticState& expected,
                                                      const model::RedSemanticState& actual,
                                                      const ComparisonOptions& options = {}) {
        std::vector<Difference> differences;
        if (expected == actual && options.compareEventSubset) {
            return differences;
        }

        auto add_if = [&differences](bool condition,
                                     std::string fieldPath,
                                     std::string expectedValue,
                                     std::string actualValue) {
            if (condition) {
                differences.push_back(
                    {DifferenceCategory::ExactMismatch, std::move(fieldPath), std::move(expectedValue), std::move(actualValue)});
            }
        };

        if (options.compareIdentity) {
            add_if(expected.identity.playerName != actual.identity.playerName,
               "identity.playerName",
               expected.identity.playerName,
               actual.identity.playerName);
            add_if(expected.identity.rivalName != actual.identity.rivalName,
               "identity.rivalName",
               expected.identity.rivalName,
               actual.identity.rivalName);
            add_if(expected.identity.trainerId != actual.identity.trainerId,
               "identity.trainerId",
               std::to_string(expected.identity.trainerId),
               std::to_string(actual.identity.trainerId));
        }

        if (options.compareCore) {
            add_if(expected.core.optionsByte != actual.core.optionsByte,
               "core.optionsByte",
               std::to_string(expected.core.optionsByte),
               std::to_string(actual.core.optionsByte));
            add_if(expected.core.letterDelayByte != actual.core.letterDelayByte,
               "core.letterDelayByte",
               std::to_string(expected.core.letterDelayByte),
               std::to_string(actual.core.letterDelayByte));
            add_if(expected.core.contrast != actual.core.contrast,
               "core.contrast",
               std::to_string(expected.core.contrast),
               std::to_string(actual.core.contrast));
            add_if(expected.core.badgesBitfield != actual.core.badgesBitfield,
               "core.badgesBitfield",
               std::to_string(expected.core.badgesBitfield),
               std::to_string(actual.core.badgesBitfield));
            add_if(expected.core.money != actual.core.money,
               "core.money",
               std::to_string(expected.core.money),
               std::to_string(actual.core.money));
            add_if(expected.core.coins != actual.core.coins,
               "core.coins",
               std::to_string(expected.core.coins),
               std::to_string(actual.core.coins));
            add_if(expected.core.mapId != actual.core.mapId,
               "core.mapId",
               std::to_string(expected.core.mapId),
               std::to_string(actual.core.mapId));
            add_if(expected.core.x != actual.core.x,
               "core.x",
               std::to_string(expected.core.x),
               std::to_string(actual.core.x));
            add_if(expected.core.y != actual.core.y,
               "core.y",
               std::to_string(expected.core.y),
               std::to_string(actual.core.y));
            add_if(expected.core.playHours != actual.core.playHours,
               "core.playHours",
               std::to_string(expected.core.playHours),
               std::to_string(actual.core.playHours));
            add_if(expected.core.playMinutes != actual.core.playMinutes,
               "core.playMinutes",
               std::to_string(expected.core.playMinutes),
               std::to_string(actual.core.playMinutes));
            add_if(expected.core.playSeconds != actual.core.playSeconds,
               "core.playSeconds",
               std::to_string(expected.core.playSeconds),
               std::to_string(actual.core.playSeconds));
        }

        if (options.compareInventory) {
            add_if(expected.inventory != actual.inventory,
               "inventory",
               "different",
               "different");
        }
        if (options.comparePokedex) {
            add_if(expected.pokedex != actual.pokedex,
               "pokedex",
               "different",
               "different");
        }
        if (options.compareParty) {
            add_if(expected.party != actual.party,
               "party",
               std::to_string(expected.party.count),
               std::to_string(actual.party.count));
        }
        if (options.compareDaycare) {
            add_if(expected.daycare != actual.daycare,
               "daycare",
               expected.daycare.inUse ? "true" : "false",
               actual.daycare.inUse ? "true" : "false");
        }
        if (options.compareHallOfFame) {
            add_if(expected.hallOfFame != actual.hallOfFame,
               "hallOfFame",
               std::to_string(expected.hallOfFame.entryCount),
               std::to_string(actual.hallOfFame.entryCount));
        }
        if (options.compareEventSubset) {
            add_if(expected.eventSubset != actual.eventSubset,
               "eventSubset",
               "different",
               "different");
        }
        return differences;
    }
};

}  // namespace pkmn::savegen::comparison
