#pragma once

#include <set>
#include <sstream>
#include <stdexcept>
#include <string>

#include "../encoding/Gen1TextEncoder.hpp"
#include "../model/RedSemanticState.hpp"
#include "../pokemon/Gen1PokemonData.hpp"
#include "PokemonStatCalculator.hpp"

namespace pkmn::savegen::generation {

class StorageValidator {
public:
    static void ValidateOrThrow(const model::StorageState& storage) {
        if (storage.boxes.size() != 12U) {
            throw std::runtime_error("decoded.pcStorage.boxes must contain exactly 12 boxes.");
        }
        if (storage.selectedBoxNumber < 1 || storage.selectedBoxNumber > 12) {
            throw std::runtime_error("decoded.currentBoxCache.selectedBoxNumber must be in 1..12.");
        }
        if (!storage.hasCurrentBoxCache) {
            throw std::runtime_error(
                "decoded.currentBoxCache.cache is required because Gen I keeps the player-visible current box in Bank 1.");
        }

        std::size_t totalPokemon = 0;
        for (std::size_t i = 0; i < storage.boxes.size(); ++i) {
            ValidateBox(storage.boxes[i], i);
            totalPokemon += storage.boxes[i].pokemon.size();
        }
        if (totalPokemon > 240U) {
            throw std::runtime_error("decoded.pcStorage total Pokemon exceeds 240.");
        }
        ValidateCurrentBoxCache(storage);
    }

    static void ValidateDaycareOrThrow(const model::DaycareState& daycare) {
        if (!daycare.inUse) {
            if (daycare.pokemon.has_value()) {
                throw std::runtime_error("decoded.daycare cannot provide a Pokemon while inUse=false.");
            }
            return;
        }
        if (!daycare.pokemon.has_value()) {
            throw std::runtime_error("decoded.daycare.inUse requires decoded.daycare.pokemon.");
        }
        ValidateStoredPokemon(*daycare.pokemon, "decoded.daycare.pokemon");
        if (daycare.pokemon->level == 0U || daycare.pokemon->level > 100U) {
            throw std::runtime_error("decoded.daycare.pokemon: level must be in 1..100.");
        }
    }

    static void ValidatePokemonOperabilityOrThrow(
        const model::StoredPokemonState& pokemon,
        const std::string& semanticPath) {
        ValidateStoredPokemon(pokemon, semanticPath);
    }

private:
    static void ValidateBox(const model::StorageBoxState& box, std::size_t index) {
        if (box.boxNumber != static_cast<int>(index + 1U)) {
            throw IndexedBoxError(index, "boxNumber must be sequential and 1-based.");
        }
        if (box.count < 0 || box.count > 20) {
            throw IndexedBoxError(index, "count must be in 0..20.");
        }
        if (static_cast<std::size_t>(box.count) != box.pokemon.size()) {
            throw IndexedBoxError(index, "count must match pokemon length.");
        }
        for (std::size_t monIndex = 0; monIndex < box.pokemon.size(); ++monIndex) {
            if (box.pokemon[monIndex].position != static_cast<int>(monIndex + 1U)) {
                throw IndexedBoxError(index, "Pokemon positions must be sequential and 1-based.");
            }
            ValidateStoredPokemon(box.pokemon[monIndex],
                                  "decoded.pcStorage.boxes[" + std::to_string(index) +
                                      "].pokemon[" + std::to_string(monIndex) + "]");
        }
    }

    static void ValidateCurrentBoxCache(const model::StorageState& storage) {
        const auto& cache = storage.currentBoxCache;
        if (cache.count < 0 || cache.count > 20) {
            throw std::runtime_error("decoded.currentBoxCache.cache.count must be in 0..20.");
        }
        if (static_cast<std::size_t>(cache.count) != cache.pokemon.size()) {
            throw std::runtime_error(
                "decoded.currentBoxCache.cache.count must match decoded.currentBoxCache.cache.pokemon length.");
        }
        if (cache.boxNumber != storage.selectedBoxNumber) {
            throw std::runtime_error(
                "decoded.currentBoxCache.cache.boxNumber must match decoded.currentBoxCache.selectedBoxNumber.");
        }
        for (std::size_t monIndex = 0; monIndex < cache.pokemon.size(); ++monIndex) {
            if (cache.pokemon[monIndex].position != static_cast<int>(monIndex + 1U)) {
                throw std::runtime_error(
                    "decoded.currentBoxCache.cache Pokemon positions must be sequential and 1-based.");
            }
            ValidateStoredPokemon(cache.pokemon[monIndex],
                                  "decoded.currentBoxCache.cache.pokemon[" +
                                      std::to_string(monIndex) + "]");
        }
    }

    static void ValidateStoredPokemon(const model::StoredPokemonState& mon,
                                      const std::string& label) {
        const auto* species = pokemon::FindSpeciesData(mon.speciesId);
        if (species == nullptr) {
            throw std::runtime_error(label + ": speciesId is unsupported.");
        }
        if (mon.nationalDexNumber != species->nationalDexNumber) {
            throw std::runtime_error(label + ": nationalDexNumber does not match species id.");
        }
        if (mon.experience > 0xFFFFFFU) {
            throw std::runtime_error(label + ": experience must fit in a 24-bit Gen I field.");
        }
        if (mon.level == 0U || mon.level > 100U) {
            throw std::runtime_error(label + ": level must be in 1..100.");
        }
        const std::uint8_t derivedLevel =
            PokemonStatCalculator::LevelFromExperience(*species, mon.experience);
        if (derivedLevel != mon.level) {
            throw std::runtime_error(
                label + ": level is inconsistent with species growth rate and experience.");
        }
        if (mon.dvs.hp != PokemonStatCalculator::DeriveHpDv(mon.dvs)) {
            throw std::runtime_error(label + ": HP DV does not match the packed derivation rule.");
        }
        ValidateDv(mon.dvs.attack, label, "attack DV");
        ValidateDv(mon.dvs.defense, label, "defense DV");
        ValidateDv(mon.dvs.speed, label, "speed DV");
        ValidateDv(mon.dvs.special, label, "special DV");
        ValidateNames(mon, label);
        ValidateMoveSlots(mon, label);
        const CalculatedPokemonStats calculated =
            PokemonStatCalculator::CalculateStoredPokemonStats(*species, mon);
        if (calculated.maxHp == 0U) {
            throw std::runtime_error(label + ": boxed Pokemon cannot derive a nonzero maximum HP.");
        }
        // Box records store current HP independently. Some emulator-valid or
        // externally edited saves contain a value above the currently derived
        // maximum. Preserve and report that source state rather than silently
        // clamping it; withdrawal viability requires a nonzero derived max HP.
    }

    static void ValidateNames(const model::StoredPokemonState& mon,
                              const std::string& label) {
        try {
            encoding::Gen1TextEncoder::EncodeName(mon.nickname, 11);
            encoding::Gen1TextEncoder::EncodeName(mon.originalTrainerName, 11);
        } catch (const std::exception& ex) {
            throw std::runtime_error(label + ": name is not Gen I encodable: " + ex.what());
        }
    }

    static void ValidateDv(std::uint8_t value,
                           const std::string& label,
                           const std::string& fieldName) {
        if (value > 15U) {
            throw std::runtime_error(label + ": " + fieldName + " must be in 0..15.");
        }
    }

    static void ValidateMoveSlots(const model::StoredPokemonState& mon,
                                  const std::string& label) {
        if (mon.moves.size() != 4U) {
            throw std::runtime_error(label + ": boxed Pokemon must contain exactly four move slots.");
        }
        for (std::size_t moveIndex = 0; moveIndex < mon.moves.size(); ++moveIndex) {
            const auto& move = mon.moves[moveIndex];
            if (move.moveId == 0U) {
                if (move.ppCurrent != 0U || move.ppUps != 0U) {
                    throw std::runtime_error(
                        label + ": NO MOVE slot must have PP=0 and PP Ups=0.");
                }
                continue;
            }
            const auto* moveData = pokemon::FindMoveData(move.moveId);
            if (moveData == nullptr) {
                throw std::runtime_error(label + ": move id is unsupported.");
            }
            const std::uint8_t ppMax =
                PokemonStatCalculator::PpMaxForMove(move.moveId, move.ppUps);
            if (move.ppCurrent > ppMax) {
                throw std::runtime_error(label + ": move PP exceeds maximum.");
            }
        }
    }

    static std::runtime_error IndexedBoxError(std::size_t index, const std::string& message) {
        std::ostringstream oss;
        oss << "decoded.pcStorage.boxes[" << index << "]: " << message;
        return std::runtime_error(oss.str());
    }
};

}  // namespace pkmn::savegen::generation
