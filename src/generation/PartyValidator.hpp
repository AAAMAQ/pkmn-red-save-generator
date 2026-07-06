#pragma once

#include <cstdint>
#include <sstream>
#include <stdexcept>
#include <string>

#include "../encoding/Gen1TextEncoder.hpp"
#include "../model/RedSemanticState.hpp"
#include "../pokemon/Gen1PokemonData.hpp"
#include "PokemonStatCalculator.hpp"

namespace pkmn::savegen::generation {

class PartyValidator {
public:
    static void ValidateOrThrow(const model::PartyState& party) {
        if (party.count < 0 || party.count > 6) {
            throw std::runtime_error("decoded.party.count must be in 0..6.");
        }
        if (static_cast<std::size_t>(party.count) != party.pokemon.size()) {
            throw std::runtime_error(
                "decoded.party.count must match decoded.party.pokemon length.");
        }

        for (std::size_t i = 0; i < party.pokemon.size(); ++i) {
            ValidatePokemon(party.pokemon[i], i);
        }
    }

private:
    static void ValidatePokemon(const model::PartyPokemonState& mon, std::size_t index) {
        const auto* species = pokemon::FindSpeciesData(mon.speciesId);
        if (species == nullptr) {
            throw IndexedError(index, "speciesId is unsupported.");
        }
        if (mon.nationalDexNumber != species->nationalDexNumber) {
            throw IndexedError(index, "nationalDexNumber does not match species id.");
        }
        if (mon.level == 0 || mon.level > 100) {
            throw IndexedError(index, "level must be in 1..100.");
        }
        if (mon.experience > 0xFFFFFFU) {
            throw IndexedError(index, "experience must fit in a 24-bit Gen I field.");
        }
        const std::uint8_t derivedLevel = PokemonStatCalculator::LevelFromExperience(*species, mon.experience);
        if (derivedLevel != mon.level) {
            throw IndexedError(index, "level is inconsistent with species growth rate and experience.");
        }
        if (mon.position != static_cast<int>(index + 1U)) {
            throw IndexedError(index, "position must be sequential and 1-based.");
        }

        ValidateStatus(mon.statusRaw, index);
        ValidateNames(mon, index);

        if (mon.dvs.hp != PokemonStatCalculator::DeriveHpDv(mon.dvs)) {
            throw IndexedError(index, "HP DV does not match the packed low-bit derivation rule.");
        }
        ValidateDv(mon.dvs.attack, "attack DV", index);
        ValidateDv(mon.dvs.defense, "defense DV", index);
        ValidateDv(mon.dvs.speed, "speed DV", index);
        ValidateDv(mon.dvs.special, "special DV", index);

        ValidateMoveSlots(mon, index);

        ValidateStoredStat(mon.maxHp, "maxHp", index);
        ValidateStoredStat(mon.attack, "attack", index);
        ValidateStoredStat(mon.defense, "defense", index);
        ValidateStoredStat(mon.speed, "speed", index);
        ValidateStoredStat(mon.special, "special", index);
        if (mon.currentHp > mon.maxHp) {
            throw IndexedError(index, "currentHp must not exceed maxHp.");
        }
    }

    static void ValidateStatus(std::uint8_t statusRaw, std::size_t index) {
        if (statusRaw == 0) {
            return;
        }

        const bool hasMajorStatus = (statusRaw & 0x78U) != 0;
        const std::uint8_t sleepTurns = static_cast<std::uint8_t>(statusRaw & 0x07U);
        if (sleepTurns != 0U) {
            if (sleepTurns > 7U || hasMajorStatus) {
                throw IndexedError(index, "status byte is not a valid Gen I sleep encoding.");
            }
            return;
        }

        switch (statusRaw) {
            case 0x08:
            case 0x10:
            case 0x20:
            case 0x40:
                return;
            default:
                throw IndexedError(index, "status byte is not a supported Gen I party status.");
        }
    }

    static void ValidateNames(const model::PartyPokemonState& mon, std::size_t index) {
        try {
            encoding::Gen1TextEncoder::EncodeName(mon.nickname, 11);
        } catch (const std::exception& ex) {
            throw IndexedError(index, std::string("nickname is not Gen I encodable: ") + ex.what());
        }
        try {
            encoding::Gen1TextEncoder::EncodeName(mon.originalTrainerName, 11);
        } catch (const std::exception& ex) {
            throw IndexedError(index, std::string("originalTrainer name is not Gen I encodable: ") + ex.what());
        }
    }

    static void ValidateDv(std::uint8_t value, const std::string& label, std::size_t index) {
        if (value > 15U) {
            throw IndexedError(index, label + " must be in 0..15.");
        }
    }

    static void ValidateMoveSlots(const model::PartyPokemonState& mon, std::size_t index) {
        if (mon.moves.size() != 4U) {
            throw IndexedError(index, "party Pokemon must contain exactly four move slots.");
        }

        for (std::size_t moveIndex = 0; moveIndex < mon.moves.size(); ++moveIndex) {
            const auto& move = mon.moves[moveIndex];
            if (move.moveId == 0U) {
                if (move.ppCurrent != 0U || move.ppUps != 0U) {
                    throw IndexedError(index,
                                       "NO MOVE slots must have PP=0 and PP Ups=0 at move slot " +
                                           std::to_string(moveIndex) + ".");
                }
                continue;
            }

            const auto* moveData = pokemon::FindMoveData(move.moveId);
            if (moveData == nullptr) {
                throw IndexedError(index, "move id is unsupported at move slot " +
                                              std::to_string(moveIndex) + ".");
            }
            const std::uint8_t ppMax = PokemonStatCalculator::PpMaxForMove(move.moveId, move.ppUps);
            if (move.ppCurrent > ppMax) {
                throw IndexedError(index, "move PP exceeds maximum at move slot " +
                                              std::to_string(moveIndex) + ".");
            }
        }
    }

    static void ValidateStoredStat(std::uint16_t value,
                                   const std::string& label,
                                   std::size_t index) {
        if (value == 0U || value > 999U) {
            throw IndexedError(index, label + " must be in 1..999.");
        }
    }

    static std::runtime_error IndexedError(std::size_t index, const std::string& message) {
        std::ostringstream oss;
        oss << "decoded.party.pokemon[" << index << "]: " << message;
        return std::runtime_error(oss.str());
    }
};

}  // namespace pkmn::savegen::generation
