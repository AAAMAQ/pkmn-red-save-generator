#pragma once

#include <string>
#include <vector>

#include "../model/RedSemanticState.hpp"
#include "ComparisonTypes.hpp"
#include "PartyComparisonRules.hpp"

namespace pkmn::savegen::comparison {

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
                                     DifferenceCategory category,
                                     std::string fieldPath,
                                     std::string expectedValue,
                                     std::string actualValue) {
            if (condition) {
                differences.push_back({category,
                                       std::move(fieldPath),
                                       std::move(expectedValue),
                                       std::move(actualValue)});
            }
        };

        if (options.compareIdentity) {
            add_if(expected.identity.playerName != actual.identity.playerName,
               DifferenceCategory::RequiredExactMismatch,
               "identity.playerName",
               expected.identity.playerName,
               actual.identity.playerName);
            add_if(expected.identity.rivalName != actual.identity.rivalName,
               DifferenceCategory::RequiredExactMismatch,
               "identity.rivalName",
               expected.identity.rivalName,
               actual.identity.rivalName);
            add_if(expected.identity.trainerId != actual.identity.trainerId,
               DifferenceCategory::RequiredExactMismatch,
               "identity.trainerId",
               std::to_string(expected.identity.trainerId),
               std::to_string(actual.identity.trainerId));
        }

        if (options.compareCore) {
            add_if(expected.core.optionsByte != actual.core.optionsByte,
               DifferenceCategory::RequiredExactMismatch,
               "core.optionsByte",
               std::to_string(expected.core.optionsByte),
               std::to_string(actual.core.optionsByte));
            add_if(expected.core.letterDelayByte != actual.core.letterDelayByte,
               DifferenceCategory::RequiredExactMismatch,
               "core.letterDelayByte",
               std::to_string(expected.core.letterDelayByte),
               std::to_string(actual.core.letterDelayByte));
            add_if(expected.core.contrast != actual.core.contrast,
               DifferenceCategory::RequiredExactMismatch,
               "core.contrast",
               std::to_string(expected.core.contrast),
               std::to_string(actual.core.contrast));
            add_if(expected.core.badgesBitfield != actual.core.badgesBitfield,
               DifferenceCategory::RequiredExactMismatch,
               "core.badgesBitfield",
               std::to_string(expected.core.badgesBitfield),
               std::to_string(actual.core.badgesBitfield));
            add_if(expected.core.money != actual.core.money,
               DifferenceCategory::RequiredExactMismatch,
               "core.money",
               std::to_string(expected.core.money),
               std::to_string(actual.core.money));
            add_if(expected.core.coins != actual.core.coins,
               DifferenceCategory::RequiredExactMismatch,
               "core.coins",
               std::to_string(expected.core.coins),
               std::to_string(actual.core.coins));
            add_if(expected.core.mapId != actual.core.mapId,
               DifferenceCategory::RequiredExactMismatch,
               "core.mapId",
               std::to_string(expected.core.mapId),
               std::to_string(actual.core.mapId));
            add_if(expected.core.x != actual.core.x,
               DifferenceCategory::RequiredExactMismatch,
               "core.x",
               std::to_string(expected.core.x),
               std::to_string(actual.core.x));
            add_if(expected.core.y != actual.core.y,
               DifferenceCategory::RequiredExactMismatch,
               "core.y",
               std::to_string(expected.core.y),
               std::to_string(actual.core.y));
            add_if(expected.core.playHours != actual.core.playHours,
               DifferenceCategory::RequiredExactMismatch,
               "core.playHours",
               std::to_string(expected.core.playHours),
               std::to_string(actual.core.playHours));
            add_if(expected.core.playMinutes != actual.core.playMinutes,
               DifferenceCategory::RequiredExactMismatch,
               "core.playMinutes",
               std::to_string(expected.core.playMinutes),
               std::to_string(actual.core.playMinutes));
            add_if(expected.core.playSeconds != actual.core.playSeconds,
               DifferenceCategory::RequiredExactMismatch,
               "core.playSeconds",
               std::to_string(expected.core.playSeconds),
               std::to_string(actual.core.playSeconds));
        }

        if (options.compareInventory) {
            add_if(expected.inventory.bagItems != actual.inventory.bagItems,
               DifferenceCategory::RequiredExactMismatch,
               "inventory.bagItems",
               "different",
               "different");
            add_if(expected.inventory.pcItems != actual.inventory.pcItems,
               DifferenceCategory::RequiredExactMismatch,
               "inventory.pcItems",
               "different",
               "different");
        }
        if (options.comparePokedex) {
            add_if(expected.pokedex.owned != actual.pokedex.owned,
               DifferenceCategory::RequiredExactMismatch,
               "pokedex.owned",
               "different",
               "different");
            add_if(expected.pokedex.seen != actual.pokedex.seen,
               DifferenceCategory::RequiredExactMismatch,
               "pokedex.seen",
               "different",
               "different");
            add_if(expected.pokedex.ownedCount != actual.pokedex.ownedCount,
               DifferenceCategory::DerivedMismatch,
               "pokedex.ownedCount",
               std::to_string(expected.pokedex.ownedCount),
               std::to_string(actual.pokedex.ownedCount));
            add_if(expected.pokedex.seenCount != actual.pokedex.seenCount,
               DifferenceCategory::DerivedMismatch,
               "pokedex.seenCount",
               std::to_string(expected.pokedex.seenCount),
               std::to_string(actual.pokedex.seenCount));
        }
        if (options.compareParty) {
            PartyComparisonRules::Compare(expected.party, actual.party, add_if);
        }
        if (options.compareDaycare) {
            add_if(expected.daycare != actual.daycare,
               DifferenceCategory::PermittedCanonicalDifference,
               "daycare",
               expected.daycare.inUse ? "true" : "false",
               actual.daycare.inUse ? "true" : "false");
        }
        if (options.compareHallOfFame) {
            add_if(expected.hallOfFame != actual.hallOfFame,
               DifferenceCategory::PermittedCanonicalDifference,
               "hallOfFame",
               std::to_string(expected.hallOfFame.entryCount),
               std::to_string(actual.hallOfFame.entryCount));
        }
        if (options.compareEventSubset) {
            add_if(expected.eventSubset.visitedTowns != actual.eventSubset.visitedTowns,
               DifferenceCategory::RequiredExactMismatch,
               "eventSubset.visitedTowns",
               "different",
               "different");
            add_if(expected.eventSubset.hiddenItems != actual.eventSubset.hiddenItems,
               DifferenceCategory::RequiredExactMismatch,
               "eventSubset.hiddenItems",
               "different",
               "different");
            add_if(expected.eventSubset.hiddenCoins != actual.eventSubset.hiddenCoins,
               DifferenceCategory::RequiredExactMismatch,
               "eventSubset.hiddenCoins",
               "different",
               "different");
        }
        return differences;
    }

    static const char* CategoryLabel(DifferenceCategory category) {
        switch (category) {
            case DifferenceCategory::RequiredExactMismatch:
                return "required-exact-mismatch";
            case DifferenceCategory::DerivedMismatch:
                return "derived-mismatch";
            case DifferenceCategory::PermittedCanonicalDifference:
                return "permitted-canonical-difference";
        }
        return "unknown";
    }
};

}  // namespace pkmn::savegen::comparison
