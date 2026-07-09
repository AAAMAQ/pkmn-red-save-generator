#pragma once

#include <algorithm>
#include <string>
#include <vector>

#include "../model/RedSemanticState.hpp"
#include "ComparisonTypes.hpp"
#include "PartyComparisonRules.hpp"
#include "StorageComparisonRules.hpp"

namespace pkmn::savegen::comparison {

struct ComparisonOptions {
    bool compareIdentity = true;
    bool compareCore = true;
    bool comparePokedex = true;
    bool compareInventory = true;
    bool compareParty = true;
    bool compareStorage = true;
    bool compareDaycare = true;
    bool compareHallOfFame = true;
    bool compareWorldSubset = true;
    bool compareEvents = true;
    bool compareTrainerBattles = true;
    bool compareStaticBattles = true;
    bool compareStoryProgress = true;
    bool compareScripts = true;
    bool compareMissableObjects = true;
};

class SemanticComparator {
public:
    static std::vector<Difference> CompareOwnedFields(const model::RedSemanticState& expected,
                                                      const model::RedSemanticState& actual,
                                                      const ComparisonOptions& options = {}) {
        std::vector<Difference> differences;

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
            CompareCore(expected.core, actual.core, add_if);
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
        if (options.compareStorage) {
            StorageComparisonRules::Compare(expected.storage, actual.storage, add_if);
        }
        if (options.compareDaycare) {
            CompareDaycare(expected.daycare, actual.daycare, add_if);
        }
        if (options.compareHallOfFame) {
            CompareHallOfFame(expected.hallOfFame, actual.hallOfFame, add_if);
        }
        if (options.compareWorldSubset) {
            CompareVisitedTowns(expected.visitedTowns, actual.visitedTowns, add_if);
            CompareHiddenObjects(expected.hiddenItems, actual.hiddenItems, "hiddenItems", add_if);
            CompareHiddenObjects(expected.hiddenCoins, actual.hiddenCoins, "hiddenCoins", add_if);
        }
        if (options.compareMissableObjects) {
            CompareMissableObjects(expected.missableObjects, actual.missableObjects, add_if);
        }
        if (options.compareEvents) {
            CompareNamedFlags(expected.events, actual.events, "events.flags", add_if);
        }
        if (options.compareTrainerBattles) {
            CompareNamedFlags(expected.trainerBattles, actual.trainerBattles, "trainerBattles.records", add_if);
        }
        if (options.compareStaticBattles) {
            CompareNamedFlags(expected.staticBattles, actual.staticBattles, "staticBattles.records", add_if);
        }
        if (options.compareStoryProgress) {
            CompareNamedFlags(expected.storyProgress, actual.storyProgress, "storyProgress.storyFlags", add_if);
        }
        if (options.compareScripts) {
            CompareScripts(expected.scripts, actual.scripts, add_if);
        }

        return differences;
    }

    static bool HasBlockingDifferences(const std::vector<Difference>& differences) {
        for (const auto& difference : differences) {
            if (difference.category != DifferenceCategory::PermittedCanonicalDifference) {
                return true;
            }
        }
        return false;
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

private:
    template <typename AddFn>
    static void CompareDaycare(const model::DaycareState& expected,
                               const model::DaycareState& actual,
                               AddFn&& add_if) {
        add_if(expected.inUse != actual.inUse,
               DifferenceCategory::RequiredExactMismatch,
               "daycare.inUse",
               expected.inUse ? "true" : "false",
               actual.inUse ? "true" : "false");
        if (expected.inUse != actual.inUse || !expected.inUse) {
            return;
        }
        add_if(expected.pokemon.has_value() != actual.pokemon.has_value(),
               DifferenceCategory::RequiredExactMismatch,
               "daycare.pokemon.present",
               expected.pokemon.has_value() ? "true" : "false",
               actual.pokemon.has_value() ? "true" : "false");
        if (!expected.pokemon.has_value() || !actual.pokemon.has_value()) {
            return;
        }
        const auto& expectedMon = expected.pokemon.value();
        const auto& actualMon = actual.pokemon.value();
        add_if(expectedMon.speciesId != actualMon.speciesId,
               DifferenceCategory::RequiredExactMismatch,
               "daycare.pokemon.speciesId",
               std::to_string(expectedMon.speciesId),
               std::to_string(actualMon.speciesId));
        add_if(expectedMon.nickname != actualMon.nickname,
               DifferenceCategory::RequiredExactMismatch,
               "daycare.pokemon.nickname",
               expectedMon.nickname,
               actualMon.nickname);
        add_if(expectedMon.originalTrainerName != actualMon.originalTrainerName,
               DifferenceCategory::RequiredExactMismatch,
               "daycare.pokemon.originalTrainerName",
               expectedMon.originalTrainerName,
               actualMon.originalTrainerName);
        add_if(expectedMon.originalTrainerId != actualMon.originalTrainerId,
               DifferenceCategory::RequiredExactMismatch,
               "daycare.pokemon.originalTrainerId",
               std::to_string(expectedMon.originalTrainerId),
               std::to_string(actualMon.originalTrainerId));
        add_if(expectedMon.experience != actualMon.experience,
               DifferenceCategory::RequiredExactMismatch,
               "daycare.pokemon.experience",
               std::to_string(expectedMon.experience),
               std::to_string(actualMon.experience));
        add_if(expectedMon.level != actualMon.level,
               DifferenceCategory::PermittedCanonicalDifference,
               "daycare.pokemon.level",
               std::to_string(expectedMon.level),
               std::to_string(actualMon.level));
    }

    template <typename AddFn>
    static void CompareHallOfFame(const model::HallOfFameState& expected,
                                  const model::HallOfFameState& actual,
                                  AddFn&& add_if) {
        add_if(expected.entryCount != actual.entryCount,
               DifferenceCategory::RequiredExactMismatch,
               "hallOfFame.entryCount",
               std::to_string(expected.entryCount),
               std::to_string(actual.entryCount));
        add_if(expected.entries.size() != actual.entries.size(),
               DifferenceCategory::RequiredExactMismatch,
               "hallOfFame.entries.length",
               std::to_string(expected.entries.size()),
               std::to_string(actual.entries.size()));
        const std::size_t sharedEntries = std::min(expected.entries.size(), actual.entries.size());
        for (std::size_t entryIndex = 0; entryIndex < sharedEntries; ++entryIndex) {
            const std::string prefix = "hallOfFame.entries[" + std::to_string(entryIndex) + "]";
            const auto& expectedEntry = expected.entries[entryIndex];
            const auto& actualEntry = actual.entries[entryIndex];
            add_if(expectedEntry.entryNumber != actualEntry.entryNumber,
                   DifferenceCategory::RequiredExactMismatch,
                   prefix + ".entryNumber",
                   std::to_string(expectedEntry.entryNumber),
                   std::to_string(actualEntry.entryNumber));
            add_if(expectedEntry.pokemon.size() != actualEntry.pokemon.size(),
                   DifferenceCategory::RequiredExactMismatch,
                   prefix + ".pokemon.length",
                   std::to_string(expectedEntry.pokemon.size()),
                   std::to_string(actualEntry.pokemon.size()));
            const std::size_t sharedMons = std::min(expectedEntry.pokemon.size(), actualEntry.pokemon.size());
            for (std::size_t monIndex = 0; monIndex < sharedMons; ++monIndex) {
                const std::string monPrefix = prefix + ".pokemon[" + std::to_string(monIndex) + "]";
                const auto& expectedMon = expectedEntry.pokemon[monIndex];
                const auto& actualMon = actualEntry.pokemon[monIndex];
                add_if(expectedMon.partyOrder != actualMon.partyOrder,
                       DifferenceCategory::RequiredExactMismatch,
                       monPrefix + ".partyOrder",
                       std::to_string(expectedMon.partyOrder),
                       std::to_string(actualMon.partyOrder));
                add_if(expectedMon.speciesId != actualMon.speciesId,
                       DifferenceCategory::RequiredExactMismatch,
                       monPrefix + ".speciesId",
                       std::to_string(expectedMon.speciesId),
                       std::to_string(actualMon.speciesId));
                add_if(expectedMon.level != actualMon.level,
                       DifferenceCategory::RequiredExactMismatch,
                       monPrefix + ".level",
                       std::to_string(expectedMon.level),
                       std::to_string(actualMon.level));
                add_if(expectedMon.nickname != actualMon.nickname,
                       DifferenceCategory::RequiredExactMismatch,
                       monPrefix + ".nickname",
                       expectedMon.nickname,
                       actualMon.nickname);
            }
        }
    }

    template <typename AddFn>
    static void CompareVisitedTowns(const std::vector<model::VisitedTownState>& expected,
                                    const std::vector<model::VisitedTownState>& actual,
                                    AddFn&& add_if) {
        add_if(expected.size() != actual.size(),
               DifferenceCategory::RequiredExactMismatch,
               "visitedTowns.length",
               std::to_string(expected.size()),
               std::to_string(actual.size()));
        const std::size_t shared = std::min(expected.size(), actual.size());
        for (std::size_t index = 0; index < shared; ++index) {
            const std::string prefix = "visitedTowns[" + std::to_string(index) + "]";
            add_if(expected[index].index != actual[index].index,
                   DifferenceCategory::RequiredExactMismatch,
                   prefix + ".index",
                   std::to_string(expected[index].index),
                   std::to_string(actual[index].index));
            add_if(expected[index].visited != actual[index].visited,
                   DifferenceCategory::RequiredExactMismatch,
                   prefix + ".visited",
                   expected[index].visited ? "true" : "false",
                   actual[index].visited ? "true" : "false");
        }
    }

    template <typename AddFn>
    static void CompareHiddenObjects(const std::vector<model::HiddenObjectState>& expected,
                                     const std::vector<model::HiddenObjectState>& actual,
                                     const std::string& fieldName,
                                     AddFn&& add_if) {
        add_if(expected.size() != actual.size(),
               DifferenceCategory::RequiredExactMismatch,
               fieldName + ".length",
               std::to_string(expected.size()),
               std::to_string(actual.size()));
        const std::size_t shared = std::min(expected.size(), actual.size());
        for (std::size_t index = 0; index < shared; ++index) {
            const std::string prefix = fieldName + "[" + std::to_string(index) + "]";
            add_if(expected[index].index != actual[index].index,
                   DifferenceCategory::RequiredExactMismatch,
                   prefix + ".index",
                   std::to_string(expected[index].index),
                   std::to_string(actual[index].index));
            add_if(expected[index].collected != actual[index].collected,
                   DifferenceCategory::RequiredExactMismatch,
                   prefix + ".collected",
                   expected[index].collected ? "true" : "false",
                   actual[index].collected ? "true" : "false");
        }
    }

    template <typename AddFn>
    static void CompareMissableObjects(const std::vector<model::MissableObjectState>& expected,
                                       const std::vector<model::MissableObjectState>& actual,
                                       AddFn&& add_if) {
        add_if(expected.size() != actual.size(),
               DifferenceCategory::RequiredExactMismatch,
               "missableObjects.length",
               std::to_string(expected.size()),
               std::to_string(actual.size()));
        const std::size_t shared = std::min(expected.size(), actual.size());
        for (std::size_t index = 0; index < shared; ++index) {
            const std::string prefix = "missableObjects[" + std::to_string(index) + "]";
            add_if(expected[index].index != actual[index].index,
                   DifferenceCategory::RequiredExactMismatch,
                   prefix + ".index",
                   std::to_string(expected[index].index),
                   std::to_string(actual[index].index));
            add_if(expected[index].toggledOff != actual[index].toggledOff,
                   DifferenceCategory::RequiredExactMismatch,
                   prefix + ".toggledOff",
                   expected[index].toggledOff ? "true" : "false",
                   actual[index].toggledOff ? "true" : "false");
        }
    }

    template <typename AddFn>
    static void CompareNamedFlags(const std::vector<model::NamedFlagState>& expected,
                                  const std::vector<model::NamedFlagState>& actual,
                                  const std::string& fieldName,
                                  AddFn&& add_if) {
        add_if(expected.size() != actual.size(),
               DifferenceCategory::RequiredExactMismatch,
               fieldName + ".length",
               std::to_string(expected.size()),
               std::to_string(actual.size()));
        const std::size_t shared = std::min(expected.size(), actual.size());
        for (std::size_t index = 0; index < shared; ++index) {
            const std::string prefix = fieldName + "[" + std::to_string(index) + "]";
            add_if(expected[index].flagIndex != actual[index].flagIndex,
                   DifferenceCategory::RequiredExactMismatch,
                   prefix + ".flagIndex",
                   std::to_string(expected[index].flagIndex),
                   std::to_string(actual[index].flagIndex));
            add_if(expected[index].set != actual[index].set,
                   DifferenceCategory::RequiredExactMismatch,
                   prefix + ".set",
                   expected[index].set ? "true" : "false",
                   actual[index].set ? "true" : "false");
        }
    }

    template <typename AddFn>
    static void CompareScripts(const std::vector<model::ScriptState>& expected,
                               const std::vector<model::ScriptState>& actual,
                               AddFn&& add_if) {
        add_if(expected.size() != actual.size(),
               DifferenceCategory::RequiredExactMismatch,
               "scripts.length",
               std::to_string(expected.size()),
               std::to_string(actual.size()));
        const std::size_t shared = std::min(expected.size(), actual.size());
        for (std::size_t index = 0; index < shared; ++index) {
            const std::string prefix = "scripts[" + std::to_string(index) + "]";
            add_if(expected[index].index != actual[index].index,
                   DifferenceCategory::RequiredExactMismatch,
                   prefix + ".index",
                   std::to_string(expected[index].index),
                   std::to_string(actual[index].index));
            add_if(expected[index].relativeOffset != actual[index].relativeOffset,
                   DifferenceCategory::RequiredExactMismatch,
                   prefix + ".relativeOffset",
                   std::to_string(expected[index].relativeOffset),
                   std::to_string(actual[index].relativeOffset));
            add_if(expected[index].size != actual[index].size,
                   DifferenceCategory::RequiredExactMismatch,
                   prefix + ".size",
                   std::to_string(expected[index].size),
                   std::to_string(actual[index].size));
            add_if(expected[index].value != actual[index].value,
                   DifferenceCategory::RequiredExactMismatch,
                   prefix + ".value",
                   std::to_string(expected[index].value),
                   std::to_string(actual[index].value));
        }
    }

    template <typename AddFn>
    static void CompareCore(const model::CoreState& expected,
                            const model::CoreState& actual,
                            AddFn&& add_if) {
        add_if(expected.optionsByte != actual.optionsByte,
               DifferenceCategory::RequiredExactMismatch,
               "core.optionsByte",
               std::to_string(expected.optionsByte),
               std::to_string(actual.optionsByte));
        add_if(expected.letterDelayByte != actual.letterDelayByte,
               DifferenceCategory::RequiredExactMismatch,
               "core.letterDelayByte",
               std::to_string(expected.letterDelayByte),
               std::to_string(actual.letterDelayByte));
        add_if(expected.contrast != actual.contrast,
               DifferenceCategory::RequiredExactMismatch,
               "core.contrast",
               std::to_string(expected.contrast),
               std::to_string(actual.contrast));
        add_if(expected.badgesBitfield != actual.badgesBitfield,
               DifferenceCategory::RequiredExactMismatch,
               "core.badgesBitfield",
               std::to_string(expected.badgesBitfield),
               std::to_string(actual.badgesBitfield));
        add_if(expected.money != actual.money,
               DifferenceCategory::RequiredExactMismatch,
               "core.money",
               std::to_string(expected.money),
               std::to_string(actual.money));
        add_if(expected.coins != actual.coins,
               DifferenceCategory::RequiredExactMismatch,
               "core.coins",
               std::to_string(expected.coins),
               std::to_string(actual.coins));
        add_if(expected.mapId != actual.mapId,
               DifferenceCategory::RequiredExactMismatch,
               "core.mapId",
               std::to_string(expected.mapId),
               std::to_string(actual.mapId));
        add_if(expected.x != actual.x,
               DifferenceCategory::RequiredExactMismatch,
               "core.x",
               std::to_string(expected.x),
               std::to_string(actual.x));
        add_if(expected.y != actual.y,
               DifferenceCategory::RequiredExactMismatch,
               "core.y",
               std::to_string(expected.y),
               std::to_string(actual.y));
        add_if(expected.previousMapId != actual.previousMapId,
               DifferenceCategory::RequiredExactMismatch,
               "core.previousMapId",
               std::to_string(expected.previousMapId),
               std::to_string(actual.previousMapId));
        add_if(expected.xBlockCoord != actual.xBlockCoord,
               DifferenceCategory::RequiredExactMismatch,
               "core.xBlockCoord",
               std::to_string(expected.xBlockCoord),
               std::to_string(actual.xBlockCoord));
        add_if(expected.yBlockCoord != actual.yBlockCoord,
               DifferenceCategory::RequiredExactMismatch,
               "core.yBlockCoord",
               std::to_string(expected.yBlockCoord),
               std::to_string(actual.yBlockCoord));
        add_if(expected.movementMode != actual.movementMode,
               DifferenceCategory::RequiredExactMismatch,
               "core.movementMode",
               expected.movementMode,
               actual.movementMode);
        add_if(expected.playerMoveDirection != actual.playerMoveDirection,
               DifferenceCategory::RequiredExactMismatch,
               "core.playerMoveDirection",
               expected.playerMoveDirection,
               actual.playerMoveDirection);
        add_if(expected.playerCurrentDirection != actual.playerCurrentDirection,
               DifferenceCategory::RequiredExactMismatch,
               "core.playerCurrentDirection",
               expected.playerCurrentDirection,
               actual.playerCurrentDirection);
        add_if(expected.strengthOutsideBattle != actual.strengthOutsideBattle,
               DifferenceCategory::RequiredExactMismatch,
               "core.strengthOutsideBattle",
               expected.strengthOutsideBattle ? "true" : "false",
               actual.strengthOutsideBattle ? "true" : "false");
        add_if(expected.surfingAllowed != actual.surfingAllowed,
               DifferenceCategory::RequiredExactMismatch,
               "core.surfingAllowed",
               expected.surfingAllowed ? "true" : "false",
               actual.surfingAllowed ? "true" : "false");
        add_if(expected.flyOutOfBattle != actual.flyOutOfBattle,
               DifferenceCategory::RequiredExactMismatch,
               "core.flyOutOfBattle",
               expected.flyOutOfBattle ? "true" : "false",
               actual.flyOutOfBattle ? "true" : "false");
        add_if(expected.isBattle != actual.isBattle,
               DifferenceCategory::RequiredExactMismatch,
               "core.isBattle",
               expected.isBattle ? "true" : "false",
               actual.isBattle ? "true" : "false");
        add_if(expected.isTrainerBattle != actual.isTrainerBattle,
               DifferenceCategory::RequiredExactMismatch,
               "core.isTrainerBattle",
               expected.isTrainerBattle ? "true" : "false",
               actual.isTrainerBattle ? "true" : "false");
        add_if(expected.countPlaytime != actual.countPlaytime,
               DifferenceCategory::RequiredExactMismatch,
               "core.countPlaytime",
               expected.countPlaytime ? "true" : "false",
               actual.countPlaytime ? "true" : "false");
        add_if(expected.gotOldRod != actual.gotOldRod,
               DifferenceCategory::RequiredExactMismatch,
               "worldState.storyEvidence.gotOldRod",
               expected.gotOldRod ? "true" : "false",
               actual.gotOldRod ? "true" : "false");
        add_if(expected.gotGoodRod != actual.gotGoodRod,
               DifferenceCategory::RequiredExactMismatch,
               "worldState.storyEvidence.gotGoodRod",
               expected.gotGoodRod ? "true" : "false",
               actual.gotGoodRod ? "true" : "false");
        add_if(expected.gotSuperRod != actual.gotSuperRod,
               DifferenceCategory::RequiredExactMismatch,
               "worldState.storyEvidence.gotSuperRod",
               expected.gotSuperRod ? "true" : "false",
               actual.gotSuperRod ? "true" : "false");
        add_if(expected.satisfiedSaffronGuards != actual.satisfiedSaffronGuards,
               DifferenceCategory::RequiredExactMismatch,
               "worldState.storyEvidence.satisfiedSaffronGuards",
               expected.satisfiedSaffronGuards ? "true" : "false",
               actual.satisfiedSaffronGuards ? "true" : "false");
        add_if(expected.gotLapras != actual.gotLapras,
               DifferenceCategory::RequiredExactMismatch,
               "worldState.storyEvidence.gotLapras",
               expected.gotLapras ? "true" : "false",
               actual.gotLapras ? "true" : "false");
        add_if(expected.everHealedPokemon != actual.everHealedPokemon,
               DifferenceCategory::RequiredExactMismatch,
               "worldState.storyEvidence.everHealedPokemon",
               expected.everHealedPokemon ? "true" : "false",
               actual.everHealedPokemon ? "true" : "false");
        add_if(expected.gotStarter != actual.gotStarter,
               DifferenceCategory::RequiredExactMismatch,
               "worldState.storyEvidence.gotStarter",
               expected.gotStarter ? "true" : "false",
               actual.gotStarter ? "true" : "false");
        add_if(expected.defeatedLoreleiRoomState != actual.defeatedLoreleiRoomState,
               DifferenceCategory::RequiredExactMismatch,
               "worldState.storyEvidence.defeatedLoreleiRoomState",
               expected.defeatedLoreleiRoomState ? "true" : "false",
               actual.defeatedLoreleiRoomState ? "true" : "false");
        add_if(expected.safariGameOver != actual.safariGameOver,
               DifferenceCategory::RequiredExactMismatch,
               "core.safariGameOver",
               expected.safariGameOver ? "true" : "false",
               actual.safariGameOver ? "true" : "false");
        add_if(expected.safariBallCount != actual.safariBallCount,
               DifferenceCategory::RequiredExactMismatch,
               "core.safariBallCount",
               std::to_string(expected.safariBallCount),
               std::to_string(actual.safariBallCount));
        add_if(expected.safariSteps != actual.safariSteps,
               DifferenceCategory::RequiredExactMismatch,
               "core.safariSteps",
               std::to_string(expected.safariSteps),
               std::to_string(actual.safariSteps));
        add_if(expected.playHours != actual.playHours,
               DifferenceCategory::RequiredExactMismatch,
               "core.playHours",
               std::to_string(expected.playHours),
               std::to_string(actual.playHours));
        add_if(expected.playMinutes != actual.playMinutes,
               DifferenceCategory::RequiredExactMismatch,
               "core.playMinutes",
               std::to_string(expected.playMinutes),
               std::to_string(actual.playMinutes));
        add_if(expected.playSeconds != actual.playSeconds,
               DifferenceCategory::RequiredExactMismatch,
               "core.playSeconds",
               std::to_string(expected.playSeconds),
               std::to_string(actual.playSeconds));
    }
};

}  // namespace pkmn::savegen::comparison
