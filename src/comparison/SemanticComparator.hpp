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

class SemanticComparator {
public:
    static std::vector<Difference> CompareOwnedFields(const model::RedSemanticState& expected,
                                                      const model::RedSemanticState& actual) {
        std::vector<Difference> differences;
        if (expected == actual) {
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
        add_if(expected.inventory != actual.inventory,
               "inventory",
               "different",
               "different");
        add_if(expected.pokedex != actual.pokedex,
               "pokedex",
               "different",
               "different");
        add_if(expected.eventSubset != actual.eventSubset,
               "eventSubset",
               "different",
               "different");
        return differences;
    }
};

}  // namespace pkmn::savegen::comparison
