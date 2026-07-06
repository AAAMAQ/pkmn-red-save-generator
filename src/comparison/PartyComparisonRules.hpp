#pragma once

#include <algorithm>
#include <string>
#include <vector>

#include "../model/RedSemanticState.hpp"
#include "ComparisonTypes.hpp"

namespace pkmn::savegen::comparison {

class PartyComparisonRules {
public:
    template <typename AddFn>
    static void Compare(const model::PartyState& expected,
                        const model::PartyState& actual,
                        AddFn&& add_if) {
        add_if(expected.count != actual.count,
               DifferenceCategory::RequiredExactMismatch,
               "party.count",
               std::to_string(expected.count),
               std::to_string(actual.count));

        const std::size_t sharedCount =
            std::min(expected.pokemon.size(), actual.pokemon.size());
        for (std::size_t i = 0; i < sharedCount; ++i) {
            ComparePokemon(expected.pokemon[i], actual.pokemon[i], i, add_if);
        }

        if (expected.pokemon.size() != actual.pokemon.size()) {
            add_if(true,
                   DifferenceCategory::RequiredExactMismatch,
                   "party.pokemon.length",
                   std::to_string(expected.pokemon.size()),
                   std::to_string(actual.pokemon.size()));
        }
    }

private:
    template <typename AddFn>
    static void ComparePokemon(const model::PartyPokemonState& expected,
                               const model::PartyPokemonState& actual,
                               std::size_t index,
                               AddFn&& add_if) {
        const std::string prefix = "party[" + std::to_string(index) + "]";

        add_if(expected.position != actual.position,
               DifferenceCategory::RequiredExactMismatch,
               prefix + ".position",
               std::to_string(expected.position),
               std::to_string(actual.position));
        add_if(expected.speciesId != actual.speciesId,
               DifferenceCategory::RequiredExactMismatch,
               prefix + ".speciesId",
               std::to_string(expected.speciesId),
               std::to_string(actual.speciesId));
        add_if(expected.nationalDexNumber != actual.nationalDexNumber,
               DifferenceCategory::RequiredExactMismatch,
               prefix + ".nationalDexNumber",
               std::to_string(expected.nationalDexNumber),
               std::to_string(actual.nationalDexNumber));
        add_if(expected.nickname != actual.nickname,
               DifferenceCategory::RequiredExactMismatch,
               prefix + ".nickname",
               expected.nickname,
               actual.nickname);
        add_if(expected.originalTrainerName != actual.originalTrainerName,
               DifferenceCategory::RequiredExactMismatch,
               prefix + ".originalTrainerName",
               expected.originalTrainerName,
               actual.originalTrainerName);
        add_if(expected.originalTrainerId != actual.originalTrainerId,
               DifferenceCategory::RequiredExactMismatch,
               prefix + ".originalTrainerId",
               std::to_string(expected.originalTrainerId),
               std::to_string(actual.originalTrainerId));
        add_if(expected.level != actual.level,
               DifferenceCategory::RequiredExactMismatch,
               prefix + ".level",
               std::to_string(expected.level),
               std::to_string(actual.level));
        add_if(expected.experience != actual.experience,
               DifferenceCategory::RequiredExactMismatch,
               prefix + ".experience",
               std::to_string(expected.experience),
               std::to_string(actual.experience));
        add_if(expected.statusRaw != actual.statusRaw,
               DifferenceCategory::RequiredExactMismatch,
               prefix + ".statusRaw",
               std::to_string(expected.statusRaw),
               std::to_string(actual.statusRaw));
        add_if(expected.currentHp != actual.currentHp,
               DifferenceCategory::RequiredExactMismatch,
               prefix + ".currentHp",
               std::to_string(expected.currentHp),
               std::to_string(actual.currentHp));
        add_if(expected.maxHp != actual.maxHp,
               DifferenceCategory::RequiredExactMismatch,
               prefix + ".maxHp",
               std::to_string(expected.maxHp),
               std::to_string(actual.maxHp));
        add_if(expected.attack != actual.attack,
               DifferenceCategory::RequiredExactMismatch,
               prefix + ".attack",
               std::to_string(expected.attack),
               std::to_string(actual.attack));
        add_if(expected.defense != actual.defense,
               DifferenceCategory::RequiredExactMismatch,
               prefix + ".defense",
               std::to_string(expected.defense),
               std::to_string(actual.defense));
        add_if(expected.speed != actual.speed,
               DifferenceCategory::RequiredExactMismatch,
               prefix + ".speed",
               std::to_string(expected.speed),
               std::to_string(actual.speed));
        add_if(expected.special != actual.special,
               DifferenceCategory::RequiredExactMismatch,
               prefix + ".special",
               std::to_string(expected.special),
               std::to_string(actual.special));

        add_if(expected.type1 != actual.type1,
               DifferenceCategory::DerivedMismatch,
               prefix + ".type1",
               std::to_string(expected.type1),
               std::to_string(actual.type1));
        add_if(expected.type2 != actual.type2,
               DifferenceCategory::DerivedMismatch,
               prefix + ".type2",
               std::to_string(expected.type2),
               std::to_string(actual.type2));
        add_if(expected.catchRate != actual.catchRate,
               DifferenceCategory::DerivedMismatch,
               prefix + ".catchRate",
               std::to_string(expected.catchRate),
               std::to_string(actual.catchRate));

        add_if(expected.dvs.hp != actual.dvs.hp,
               DifferenceCategory::RequiredExactMismatch,
               prefix + ".dvs.hp",
               std::to_string(expected.dvs.hp),
               std::to_string(actual.dvs.hp));
        add_if(expected.dvs.attack != actual.dvs.attack,
               DifferenceCategory::RequiredExactMismatch,
               prefix + ".dvs.attack",
               std::to_string(expected.dvs.attack),
               std::to_string(actual.dvs.attack));
        add_if(expected.dvs.defense != actual.dvs.defense,
               DifferenceCategory::RequiredExactMismatch,
               prefix + ".dvs.defense",
               std::to_string(expected.dvs.defense),
               std::to_string(actual.dvs.defense));
        add_if(expected.dvs.speed != actual.dvs.speed,
               DifferenceCategory::RequiredExactMismatch,
               prefix + ".dvs.speed",
               std::to_string(expected.dvs.speed),
               std::to_string(actual.dvs.speed));
        add_if(expected.dvs.special != actual.dvs.special,
               DifferenceCategory::RequiredExactMismatch,
               prefix + ".dvs.special",
               std::to_string(expected.dvs.special),
               std::to_string(actual.dvs.special));

        add_if(expected.statExperience.hp != actual.statExperience.hp,
               DifferenceCategory::RequiredExactMismatch,
               prefix + ".statExperience.hp",
               std::to_string(expected.statExperience.hp),
               std::to_string(actual.statExperience.hp));
        add_if(expected.statExperience.attack != actual.statExperience.attack,
               DifferenceCategory::RequiredExactMismatch,
               prefix + ".statExperience.attack",
               std::to_string(expected.statExperience.attack),
               std::to_string(actual.statExperience.attack));
        add_if(expected.statExperience.defense != actual.statExperience.defense,
               DifferenceCategory::RequiredExactMismatch,
               prefix + ".statExperience.defense",
               std::to_string(expected.statExperience.defense),
               std::to_string(actual.statExperience.defense));
        add_if(expected.statExperience.speed != actual.statExperience.speed,
               DifferenceCategory::RequiredExactMismatch,
               prefix + ".statExperience.speed",
               std::to_string(expected.statExperience.speed),
               std::to_string(actual.statExperience.speed));
        add_if(expected.statExperience.special != actual.statExperience.special,
               DifferenceCategory::RequiredExactMismatch,
               prefix + ".statExperience.special",
               std::to_string(expected.statExperience.special),
               std::to_string(actual.statExperience.special));

        for (std::size_t moveIndex = 0; moveIndex < std::min(expected.moves.size(), actual.moves.size()); ++moveIndex) {
            const std::string movePrefix = prefix + ".moves[" + std::to_string(moveIndex) + "]";
            add_if(expected.moves[moveIndex].moveId != actual.moves[moveIndex].moveId,
                   DifferenceCategory::RequiredExactMismatch,
                   movePrefix + ".moveId",
                   std::to_string(expected.moves[moveIndex].moveId),
                   std::to_string(actual.moves[moveIndex].moveId));
            add_if(expected.moves[moveIndex].ppCurrent != actual.moves[moveIndex].ppCurrent,
                   DifferenceCategory::RequiredExactMismatch,
                   movePrefix + ".ppCurrent",
                   std::to_string(expected.moves[moveIndex].ppCurrent),
                   std::to_string(actual.moves[moveIndex].ppCurrent));
            add_if(expected.moves[moveIndex].ppUps != actual.moves[moveIndex].ppUps,
                   DifferenceCategory::RequiredExactMismatch,
                   movePrefix + ".ppUps",
                   std::to_string(expected.moves[moveIndex].ppUps),
                   std::to_string(actual.moves[moveIndex].ppUps));
        }
        if (expected.moves.size() != actual.moves.size()) {
            add_if(true,
                   DifferenceCategory::RequiredExactMismatch,
                   prefix + ".moves.length",
                   std::to_string(expected.moves.size()),
                   std::to_string(actual.moves.size()));
        }
    }
};

}  // namespace pkmn::savegen::comparison
