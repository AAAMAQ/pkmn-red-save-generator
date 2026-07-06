#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

#include "../encoding/Gen1Layout.hpp"
#include "../encoding/Gen1TextEncoder.hpp"
#include "../encoding/PrimitiveWriter.hpp"
#include "../reporting/GenerationReport.hpp"
#include "PokemonStatCalculator.hpp"
#include "RedSaveInitializer.hpp"

namespace pkmn::savegen::generation {

class PartySerializer {
public:
    static void WriteParty(WorkingSaveBuffer& working, const model::PartyState& party) {
        ZeroRange(working.bytes, encoding::Gen1Layout::PartyBase, encoding::Gen1Layout::PartyBlockLen);

        encoding::PrimitiveWriter::WriteU8(
            working.bytes, encoding::Gen1Layout::PartyCountOff, static_cast<std::uint8_t>(party.count));
        for (std::size_t i = 0; i < party.pokemon.size(); ++i) {
            const auto& mon = party.pokemon[i];
            encoding::PrimitiveWriter::WriteU8(
                working.bytes,
                encoding::Gen1Layout::PartySpeciesOff + i,
                mon.speciesId);
        }
        encoding::PrimitiveWriter::WriteU8(
            working.bytes,
            encoding::Gen1Layout::PartySpeciesOff + party.pokemon.size(),
            0xFF);

        for (std::size_t i = 0; i < party.pokemon.size(); ++i) {
            WritePartyPokemonStruct(working, party.pokemon[i], i);
            encoding::Gen1TextEncoder::WriteName(
                working.bytes,
                encoding::Gen1Layout::PartyOTNamesOff + (i * encoding::Gen1Layout::Gen1NameLen),
                encoding::Gen1Layout::Gen1NameLen,
                party.pokemon[i].originalTrainerName);
            encoding::Gen1TextEncoder::WriteName(
                working.bytes,
                encoding::Gen1Layout::PartyNicknamesOff + (i * encoding::Gen1Layout::Gen1NameLen),
                encoding::Gen1Layout::Gen1NameLen,
                party.pokemon[i].nickname);
            working.report.fieldsWritten.push_back("party[" + std::to_string(i) + "].originalTrainerName");
            working.report.fieldsWritten.push_back("party[" + std::to_string(i) + "].nickname");
        }

        MarkRange(working.report,
                  encoding::Gen1Layout::PartyBase,
                  encoding::Gen1Layout::PartyBase + encoding::Gen1Layout::PartyBlockLen - 1U,
                  party.count == 0 ? "intentionally-cleared" : "overwritten-from-target",
                  party.count == 0 ? "empty party canonical default" : "party data");
        working.report.fieldsWritten.push_back("party.count");
        working.report.fieldsWritten.push_back("party.speciesList");
    }

private:
    static void WritePartyPokemonStruct(WorkingSaveBuffer& working,
                                        const model::PartyPokemonState& mon,
                                        std::size_t slotIndex) {
        const std::size_t base =
            encoding::Gen1Layout::PartyStructsOff + (slotIndex * encoding::Gen1Layout::PartyStructSize);

        encoding::PrimitiveWriter::WriteU8(
            working.bytes, base + encoding::Gen1Layout::PartyMonSpeciesRel, mon.speciesId);
        encoding::PrimitiveWriter::WriteU16BigEndian(
            working.bytes, base + encoding::Gen1Layout::PartyMonCurrentHpRel, mon.currentHp);
        encoding::PrimitiveWriter::WriteU8(
            working.bytes, base + encoding::Gen1Layout::PartyMonBoxLevelRel, mon.level);
        encoding::PrimitiveWriter::WriteU8(
            working.bytes, base + encoding::Gen1Layout::PartyMonStatusRel, mon.statusRaw);
        encoding::PrimitiveWriter::WriteU8(
            working.bytes, base + encoding::Gen1Layout::PartyMonType1Rel, mon.type1);
        encoding::PrimitiveWriter::WriteU8(
            working.bytes, base + encoding::Gen1Layout::PartyMonType2Rel, mon.type2);
        encoding::PrimitiveWriter::WriteU8(
            working.bytes, base + encoding::Gen1Layout::PartyMonCatchRateRel, mon.catchRate);

        for (std::size_t i = 0; i < mon.moves.size(); ++i) {
            encoding::PrimitiveWriter::WriteU8(
                working.bytes, base + encoding::Gen1Layout::PartyMonMovesRel + i, mon.moves[i].moveId);
            encoding::PrimitiveWriter::WriteU8(
                working.bytes,
                base + encoding::Gen1Layout::PartyMonPpRel + i,
                PokemonStatCalculator::PackMovePp(
                    mon.moves[i].moveId, mon.moves[i].ppCurrent, mon.moves[i].ppUps));
        }

        encoding::PrimitiveWriter::WriteU16BigEndian(
            working.bytes, base + encoding::Gen1Layout::PartyMonTrainerIdRel, mon.originalTrainerId);
        encoding::PrimitiveWriter::WriteU24BigEndian(
            working.bytes, base + encoding::Gen1Layout::PartyMonExperienceRel, mon.experience);

        encoding::PrimitiveWriter::WriteU16BigEndian(
            working.bytes, base + encoding::Gen1Layout::PartyMonHpStatExpRel, mon.statExperience.hp);
        encoding::PrimitiveWriter::WriteU16BigEndian(
            working.bytes, base + encoding::Gen1Layout::PartyMonAttackStatExpRel, mon.statExperience.attack);
        encoding::PrimitiveWriter::WriteU16BigEndian(
            working.bytes, base + encoding::Gen1Layout::PartyMonDefenseStatExpRel, mon.statExperience.defense);
        encoding::PrimitiveWriter::WriteU16BigEndian(
            working.bytes, base + encoding::Gen1Layout::PartyMonSpeedStatExpRel, mon.statExperience.speed);
        encoding::PrimitiveWriter::WriteU16BigEndian(
            working.bytes, base + encoding::Gen1Layout::PartyMonSpecialStatExpRel, mon.statExperience.special);

        const std::uint16_t packedDvs = static_cast<std::uint16_t>(
            ((mon.dvs.attack & 0x0FU) << 12U) |
            ((mon.dvs.defense & 0x0FU) << 8U) |
            ((mon.dvs.speed & 0x0FU) << 4U) |
            (mon.dvs.special & 0x0FU));
        encoding::PrimitiveWriter::WriteU16BigEndian(
            working.bytes, base + encoding::Gen1Layout::PartyMonDvWordRel, packedDvs);

        encoding::PrimitiveWriter::WriteU8(
            working.bytes, base + encoding::Gen1Layout::PartyMonLevelRel, mon.level);
        encoding::PrimitiveWriter::WriteU16BigEndian(
            working.bytes, base + encoding::Gen1Layout::PartyMonMaxHpRel, mon.maxHp);
        encoding::PrimitiveWriter::WriteU16BigEndian(
            working.bytes, base + encoding::Gen1Layout::PartyMonAttackRel, mon.attack);
        encoding::PrimitiveWriter::WriteU16BigEndian(
            working.bytes, base + encoding::Gen1Layout::PartyMonDefenseRel, mon.defense);
        encoding::PrimitiveWriter::WriteU16BigEndian(
            working.bytes, base + encoding::Gen1Layout::PartyMonSpeedRel, mon.speed);
        encoding::PrimitiveWriter::WriteU16BigEndian(
            working.bytes, base + encoding::Gen1Layout::PartyMonSpecialRel, mon.special);

        working.report.fieldsWritten.push_back("party[" + std::to_string(slotIndex) + "].struct");
    }

    static void ZeroRange(std::vector<std::uint8_t>& bytes, std::size_t offset, std::size_t len) {
        encoding::PrimitiveWriter::EnsureRange(bytes, offset, len);
        std::fill(bytes.begin() + static_cast<std::ptrdiff_t>(offset),
                  bytes.begin() + static_cast<std::ptrdiff_t>(offset + len),
                  static_cast<std::uint8_t>(0));
    }

    static void MarkRange(reporting::GenerationReport& report,
                          std::size_t start,
                          std::size_t endInclusive,
                          const std::string& classification,
                          const std::string& reason) {
        report.ranges.push_back({start, endInclusive, classification, reason});
    }
};

}  // namespace pkmn::savegen::generation
