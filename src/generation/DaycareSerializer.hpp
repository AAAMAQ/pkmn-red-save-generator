#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

#include "../encoding/Gen1Layout.hpp"
#include "../encoding/Gen1TextEncoder.hpp"
#include "../encoding/PrimitiveWriter.hpp"
#include "../pokemon/Gen1PokemonData.hpp"
#include "../reporting/GenerationReport.hpp"
#include "PokemonStatCalculator.hpp"
#include "RedSaveInitializer.hpp"

namespace pkmn::savegen::generation {

class DaycareSerializer {
public:
    static void WriteDaycare(WorkingSaveBuffer& working, const model::DaycareState& daycare) {
        ZeroRange(working.bytes, encoding::Gen1Layout::DaycareInUseOff, encoding::Gen1Layout::DaycareLen);
        if (!daycare.inUse) {
            MarkRange(working.report,
                      encoding::Gen1Layout::DaycareInUseOff,
                      encoding::Gen1Layout::DaycareInUseOff + encoding::Gen1Layout::DaycareLen - 1U,
                      "intentionally-cleared",
                      "empty daycare canonical default");
            working.report.fieldsWritten.push_back("daycare");
            return;
        }

        const model::StoredPokemonState& mon = daycare.pokemon.value();
        encoding::PrimitiveWriter::WriteU8(working.bytes, encoding::Gen1Layout::DaycareInUseOff, 1);
        encoding::Gen1TextEncoder::WriteName(
            working.bytes,
            encoding::Gen1Layout::DaycareNicknameOff,
            encoding::Gen1Layout::Gen1NameLen,
            mon.nickname);
        encoding::Gen1TextEncoder::WriteName(
            working.bytes,
            encoding::Gen1Layout::DaycareOTNameOff,
            encoding::Gen1Layout::Gen1NameLen,
            mon.originalTrainerName);
        WriteBoxMonStruct(working.bytes, encoding::Gen1Layout::DaycareBoxMonOff, mon);

        MarkRange(working.report,
                  encoding::Gen1Layout::DaycareInUseOff,
                  encoding::Gen1Layout::DaycareInUseOff + encoding::Gen1Layout::DaycareLen - 1U,
                  "overwritten-from-target",
                  "daycare");
        working.report.fieldsWritten.push_back("daycare");
    }

private:
    static void WriteBoxMonStruct(std::vector<std::uint8_t>& bytes,
                                  std::size_t base,
                                  const model::StoredPokemonState& mon) {
        const auto* speciesData = pokemon::FindSpeciesData(mon.speciesId);
        if (speciesData == nullptr) {
            throw std::runtime_error("Daycare serializer encountered an unsupported species id.");
        }
        const std::uint8_t storedLevel =
            PokemonStatCalculator::LevelFromExperience(*speciesData, mon.experience);
        encoding::PrimitiveWriter::WriteU8(
            bytes, base + encoding::Gen1Layout::BoxMonSpeciesRel, mon.speciesId);
        encoding::PrimitiveWriter::WriteU16BigEndian(
            bytes, base + encoding::Gen1Layout::BoxMonCurrentHpRel, mon.currentHp);
        encoding::PrimitiveWriter::WriteU8(
            bytes, base + encoding::Gen1Layout::BoxMonLevelRel, storedLevel);
        encoding::PrimitiveWriter::WriteU8(
            bytes, base + encoding::Gen1Layout::BoxMonStatusRel, mon.statusRaw);
        encoding::PrimitiveWriter::WriteU8(
            bytes, base + encoding::Gen1Layout::BoxMonType1Rel, mon.type1);
        encoding::PrimitiveWriter::WriteU8(
            bytes, base + encoding::Gen1Layout::BoxMonType2Rel, mon.type2);
        encoding::PrimitiveWriter::WriteU8(
            bytes, base + encoding::Gen1Layout::BoxMonCatchRateRel, mon.catchRate);
        for (std::size_t i = 0; i < mon.moves.size(); ++i) {
            encoding::PrimitiveWriter::WriteU8(
                bytes, base + encoding::Gen1Layout::BoxMonMovesRel + i, mon.moves[i].moveId);
            encoding::PrimitiveWriter::WriteU8(
                bytes,
                base + encoding::Gen1Layout::BoxMonPpRel + i,
                PokemonStatCalculator::PackMovePp(
                    mon.moves[i].moveId, mon.moves[i].ppCurrent, mon.moves[i].ppUps));
        }
        encoding::PrimitiveWriter::WriteU16BigEndian(
            bytes, base + encoding::Gen1Layout::BoxMonTrainerIdRel, mon.originalTrainerId);
        encoding::PrimitiveWriter::WriteU24BigEndian(
            bytes, base + encoding::Gen1Layout::BoxMonExperienceRel, mon.experience);
        encoding::PrimitiveWriter::WriteU16BigEndian(
            bytes, base + encoding::Gen1Layout::BoxMonHpStatExpRel, mon.statExperience.hp);
        encoding::PrimitiveWriter::WriteU16BigEndian(
            bytes, base + encoding::Gen1Layout::BoxMonAttackStatExpRel, mon.statExperience.attack);
        encoding::PrimitiveWriter::WriteU16BigEndian(
            bytes, base + encoding::Gen1Layout::BoxMonDefenseStatExpRel, mon.statExperience.defense);
        encoding::PrimitiveWriter::WriteU16BigEndian(
            bytes, base + encoding::Gen1Layout::BoxMonSpeedStatExpRel, mon.statExperience.speed);
        encoding::PrimitiveWriter::WriteU16BigEndian(
            bytes, base + encoding::Gen1Layout::BoxMonSpecialStatExpRel, mon.statExperience.special);

        const std::uint16_t packedDvs = static_cast<std::uint16_t>(
            ((mon.dvs.attack & 0x0FU) << 12U) |
            ((mon.dvs.defense & 0x0FU) << 8U) |
            ((mon.dvs.speed & 0x0FU) << 4U) |
            (mon.dvs.special & 0x0FU));
        encoding::PrimitiveWriter::WriteU16BigEndian(
            bytes, base + encoding::Gen1Layout::BoxMonDvWordRel, packedDvs);
    }

    static void ZeroRange(std::vector<std::uint8_t>& bytes,
                          std::size_t offset,
                          std::size_t len) {
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
        report.ranges.push_back({
            start,
            endInclusive,
            classification,
            reason,
            "DaycareSerializer",
            reason
        });
    }
};

}  // namespace pkmn::savegen::generation
