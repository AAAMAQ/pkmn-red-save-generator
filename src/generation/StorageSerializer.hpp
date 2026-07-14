#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

#include "../encoding/Gen1Layout.hpp"
#include "../encoding/Gen1TextEncoder.hpp"
#include "../encoding/PrimitiveWriter.hpp"
#include "../integrity/BoxChecksumWriter.hpp"
#include "../pokemon/Gen1PokemonData.hpp"
#include "../reporting/GenerationReport.hpp"
#include "PokemonStatCalculator.hpp"
#include "RedSaveInitializer.hpp"

namespace pkmn::savegen::generation {

class StorageSerializer {
public:
    static void WriteStorage(WorkingSaveBuffer& working, const model::StorageState& storage) {
        for (std::size_t i = 0; i < storage.boxes.size(); ++i) {
            const std::size_t base = encoding::Gen1Layout::PermanentBoxOffsets[i];
            WriteBoxBlock(working.bytes, base, storage.boxes[i]);
        }

        const std::uint8_t rawCurrentBox = static_cast<std::uint8_t>(
            ((storage.boxChangedFlag ? 0x80U : 0x00U) |
             static_cast<std::uint8_t>(storage.selectedBoxNumber - 1)));
        encoding::PrimitiveWriter::WriteU8(
            working.bytes, encoding::Gen1Layout::CurrentBoxByteOff, rawCurrentBox);

        WriteBoxBlock(
            working.bytes, encoding::Gen1Layout::CurrentBoxCacheOff, storage.currentBoxCache);

        integrity::BoxChecksumWriter::WriteAll(working.bytes);

        MarkRange(working.report,
                  encoding::Gen1Layout::Bank2PayloadStart,
                  encoding::Gen1Layout::Bank2PayloadEndInclusive,
                  "overwritten-from-target",
                  "storage.boxes[1..6]");
        MarkRange(working.report,
                  encoding::Gen1Layout::Bank3PayloadStart,
                  encoding::Gen1Layout::Bank3PayloadEndInclusive,
                  "overwritten-from-target",
                  "storage.boxes[7..12]");
        MarkRange(working.report,
                  encoding::Gen1Layout::CurrentBoxByteOff,
                  encoding::Gen1Layout::CurrentBoxByteOff,
                  "derived-from-target",
                  "storage.selectedBoxNumber and boxChangedFlag");
        MarkRange(working.report,
                  encoding::Gen1Layout::CurrentBoxCacheOff,
                  encoding::Gen1Layout::CurrentBoxCacheOff + encoding::Gen1Layout::CurrentBoxCacheLen - 1U,
                  "overwritten-from-target",
                  "storage.currentBoxCache preserved as the player-visible Bank 1 working box");
        MarkRange(working.report,
                  encoding::Gen1Layout::Bank2BoxChecksumsOff,
                  encoding::Gen1Layout::Bank2BoxChecksumsOff + 5U,
                  "regenerated-checksum",
                  "per-box checksums for boxes 1-6");
        MarkRange(working.report,
                  encoding::Gen1Layout::Bank3BoxChecksumsOff,
                  encoding::Gen1Layout::Bank3BoxChecksumsOff + 5U,
                  "regenerated-checksum",
                  "per-box checksums for boxes 7-12");
        MarkRange(working.report,
                  encoding::Gen1Layout::Bank2AllChecksumOff,
                  encoding::Gen1Layout::Bank2AllChecksumOff,
                  "regenerated-checksum",
                  "bank 2 all-box checksum");
        MarkRange(working.report,
                  encoding::Gen1Layout::Bank3AllChecksumOff,
                  encoding::Gen1Layout::Bank3AllChecksumOff,
                  "regenerated-checksum",
                  "bank 3 all-box checksum");

        working.report.fieldsWritten.push_back("storage.boxes");
        working.report.fieldsWritten.push_back("storage.selectedBoxNumber");
        working.report.fieldsWritten.push_back("storage.currentBoxCache");
        working.report.fieldsWritten.push_back("integrity.boxChecksums");
        working.report.fieldsWritten.push_back("integrity.bank2AllChecksum");
        working.report.fieldsWritten.push_back("integrity.bank3AllChecksum");
    }

private:
    static void WriteBoxBlock(std::vector<std::uint8_t>& bytes,
                              std::size_t base,
                              const model::StorageBoxState& box) {
        ZeroRange(bytes, base, encoding::Gen1Layout::BoxBlockSize);
        encoding::PrimitiveWriter::WriteU8(
            bytes, base + encoding::Gen1Layout::BoxCountRel, static_cast<std::uint8_t>(box.count));

        for (int slot = 0; slot < encoding::Gen1Layout::BoxMaxMons; ++slot) {
            encoding::Gen1TextEncoder::WriteName(
                bytes,
                base + encoding::Gen1Layout::BoxOTNamesRel +
                    (static_cast<std::size_t>(slot) * encoding::Gen1Layout::Gen1NameLen),
                encoding::Gen1Layout::Gen1NameLen,
                "");
            encoding::Gen1TextEncoder::WriteName(
                bytes,
                base + encoding::Gen1Layout::BoxNicknamesRel +
                    (static_cast<std::size_t>(slot) * encoding::Gen1Layout::Gen1NameLen),
                encoding::Gen1Layout::Gen1NameLen,
                "");
        }

        for (std::size_t i = 0; i < box.pokemon.size(); ++i) {
            const auto& mon = box.pokemon[i];
            encoding::PrimitiveWriter::WriteU8(
                bytes, base + encoding::Gen1Layout::BoxSpeciesRel + i, mon.speciesId);
            WriteBoxPokemonStruct(bytes,
                                  base + encoding::Gen1Layout::BoxStructsRel +
                                      (i * encoding::Gen1Layout::BoxStructSize),
                                  mon);
            encoding::Gen1TextEncoder::WriteName(
                bytes,
                base + encoding::Gen1Layout::BoxOTNamesRel +
                    (i * encoding::Gen1Layout::Gen1NameLen),
                encoding::Gen1Layout::Gen1NameLen,
                mon.originalTrainerName);
            encoding::Gen1TextEncoder::WriteName(
                bytes,
                base + encoding::Gen1Layout::BoxNicknamesRel +
                    (i * encoding::Gen1Layout::Gen1NameLen),
                encoding::Gen1Layout::Gen1NameLen,
                mon.nickname);
        }

        const std::size_t terminatorIndex =
            static_cast<std::size_t>(std::min(box.count, encoding::Gen1Layout::BoxMaxMons));
        encoding::PrimitiveWriter::WriteU8(
            bytes, base + encoding::Gen1Layout::BoxSpeciesRel + terminatorIndex, 0xFF);
    }

    static void WriteBoxPokemonStruct(std::vector<std::uint8_t>& bytes,
                                      std::size_t base,
                                      const model::StoredPokemonState& mon) {
        const auto* speciesData = pokemon::FindSpeciesData(mon.speciesId);
        if (speciesData == nullptr) {
            throw std::runtime_error("Storage serializer encountered an unsupported species id.");
        }
        encoding::PrimitiveWriter::WriteU8(
            bytes, base + encoding::Gen1Layout::BoxMonSpeciesRel, mon.speciesId);
        encoding::PrimitiveWriter::WriteU16BigEndian(
            bytes, base + encoding::Gen1Layout::BoxMonCurrentHpRel, mon.currentHp);
        encoding::PrimitiveWriter::WriteU8(
            bytes, base + encoding::Gen1Layout::BoxMonLevelRel, mon.level);
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
            "StorageSerializer",
            reason
        });
    }
};

}  // namespace pkmn::savegen::generation
