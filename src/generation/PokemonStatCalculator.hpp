#pragma once

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <stdexcept>
#include <string>

#include "../model/RedSemanticState.hpp"
#include "../pokemon/Gen1PokemonData.hpp"

namespace pkmn::savegen::generation {

struct CalculatedPokemonStats {
    std::uint16_t maxHp = 0;
    std::uint16_t attack = 0;
    std::uint16_t defense = 0;
    std::uint16_t speed = 0;
    std::uint16_t special = 0;
};

class PokemonStatCalculator {
public:
    static std::uint8_t DeriveHpDv(const model::PokemonDVState& dvs) {
        return static_cast<std::uint8_t>(((dvs.attack & 0x01U) << 3U) |
                                         ((dvs.defense & 0x01U) << 2U) |
                                         ((dvs.speed & 0x01U) << 1U) |
                                         (dvs.special & 0x01U));
    }

    static std::uint32_t ExperienceForLevel(pokemon::GrowthRate growthRate, std::uint8_t level) {
        const std::uint32_t n = level;
        const std::uint32_t cube = n * n * n;
        const std::uint32_t square = n * n;
        switch (growthRate) {
            case pokemon::GrowthRate::MediumFast:
                return cube;
            case pokemon::GrowthRate::SlightlyFast:
                return ((3U * cube) / 4U) + (10U * square) - 30U;
            case pokemon::GrowthRate::SlightlySlow:
                return ((3U * cube) / 4U) + (20U * square) - 70U;
            case pokemon::GrowthRate::MediumSlow:
                return ((6U * cube) / 5U) - (15U * square) + (100U * n) - 140U;
            case pokemon::GrowthRate::Fast:
                return (4U * cube) / 5U;
            case pokemon::GrowthRate::Slow:
                return (5U * cube) / 4U;
        }
        throw std::runtime_error("Unsupported growth rate.");
    }

    static std::uint8_t LevelFromExperience(const pokemon::SpeciesData& species,
                                            std::uint32_t experience) {
        std::uint8_t level = 1;
        for (std::uint8_t candidate = 2; candidate <= 100; ++candidate) {
            if (ExperienceForLevel(species.growthRate, candidate) > experience) {
                break;
            }
            level = candidate;
        }
        return level;
    }

    static std::uint8_t EffectivePpUps(std::uint8_t value) {
        if (value > 3U) {
            throw std::runtime_error("PP Ups used must be in 0..3.");
        }
        return value;
    }

    static std::uint8_t PpMaxForMove(std::uint8_t moveId, std::uint8_t ppUps) {
        if (moveId == 0) {
            return 0;
        }
        const auto* move = pokemon::FindMoveData(moveId);
        if (move == nullptr) {
            throw std::runtime_error("Unsupported move id " + std::to_string(moveId) + ".");
        }
        const std::uint8_t ppUpsUsed = EffectivePpUps(ppUps);
        const std::uint8_t bonus = static_cast<std::uint8_t>((move->basePp / 5U) * ppUpsUsed);
        return static_cast<std::uint8_t>(move->basePp + bonus);
    }

    static std::uint8_t PackMovePp(std::uint8_t moveId,
                                   std::uint8_t ppCurrent,
                                   std::uint8_t ppUps) {
        const std::uint8_t ppMax = PpMaxForMove(moveId, ppUps);
        if (ppCurrent > ppMax) {
            throw std::runtime_error("Move PP exceeds calculated maximum.");
        }
        return static_cast<std::uint8_t>((EffectivePpUps(ppUps) << 6U) | (ppCurrent & 0x3FU));
    }

    static CalculatedPokemonStats CalculatePartyStats(
        const pokemon::SpeciesData& species,
        const model::PartyPokemonState& mon) {
        CalculatedPokemonStats stats;
        stats.maxHp = CalculateStat(species.baseHp, mon.dvs.hp, mon.statExperience.hp, mon.level, true);
        stats.attack =
            CalculateStat(species.baseAttack, mon.dvs.attack, mon.statExperience.attack, mon.level, false);
        stats.defense =
            CalculateStat(species.baseDefense, mon.dvs.defense, mon.statExperience.defense, mon.level, false);
        stats.speed =
            CalculateStat(species.baseSpeed, mon.dvs.speed, mon.statExperience.speed, mon.level, false);
        stats.special =
            CalculateStat(species.baseSpecial, mon.dvs.special, mon.statExperience.special, mon.level, false);
        return stats;
    }

    static CalculatedPokemonStats CalculateStoredPokemonStats(
        const pokemon::SpeciesData& species,
        const model::StoredPokemonState& mon) {
        CalculatedPokemonStats stats;
        stats.maxHp = CalculateStat(
            species.baseHp, mon.dvs.hp, mon.statExperience.hp, mon.level, true);
        stats.attack = CalculateStat(
            species.baseAttack, mon.dvs.attack, mon.statExperience.attack, mon.level, false);
        stats.defense = CalculateStat(
            species.baseDefense, mon.dvs.defense, mon.statExperience.defense, mon.level, false);
        stats.speed = CalculateStat(
            species.baseSpeed, mon.dvs.speed, mon.statExperience.speed, mon.level, false);
        stats.special = CalculateStat(
            species.baseSpecial, mon.dvs.special, mon.statExperience.special, mon.level, false);
        return stats;
    }

private:
    static std::uint16_t CalculateStat(std::uint8_t base,
                                       std::uint8_t dv,
                                       std::uint16_t statExperience,
                                       std::uint8_t level,
                                       bool hp) {
        const std::uint32_t basePlusDvTimes2 = static_cast<std::uint32_t>((base + dv) * 2U);
        // The Gen I game routine uses ceil(sqrt(stat experience)) before the
        // integer division by four. Using floor is observably one HP/stat low
        // for values such as 50 and rejects otherwise valid boxed Pokemon.
        const std::uint32_t statExpTerm = CeilSqrt(statExperience) / 4U;
        const std::uint32_t scaled =
            ((basePlusDvTimes2 + statExpTerm) * static_cast<std::uint32_t>(level)) / 100U;
        const std::uint32_t total = hp ? (scaled + level + 10U) : (scaled + 5U);
        return static_cast<std::uint16_t>(std::min<std::uint32_t>(total, 999U));
    }

    static std::uint32_t CeilSqrt(std::uint16_t value) {
        std::uint32_t root = 0;
        while ((root + 1U) * (root + 1U) <= value) {
            ++root;
        }
        return root * root == value ? root : root + 1U;
    }
};

}  // namespace pkmn::savegen::generation
