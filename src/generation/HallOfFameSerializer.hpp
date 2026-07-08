#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "../encoding/Gen1Layout.hpp"
#include "../encoding/Gen1TextEncoder.hpp"
#include "../encoding/PrimitiveWriter.hpp"
#include "../reporting/GenerationReport.hpp"
#include "../pokemon/Gen1PokemonData.hpp"
#include "RedSaveInitializer.hpp"

namespace pkmn::savegen::generation {

class HallOfFameSerializer {
public:
    static void ValidateOrThrow(const model::HallOfFameState& hallOfFame) {
        if (hallOfFame.entryCount < 0 ||
            hallOfFame.entryCount > static_cast<int>(encoding::Gen1Layout::HallOfFameLen / 0x60U)) {
            throw std::runtime_error("decoded.hallOfFame.entryCount exceeds supported capacity.");
        }
        if (static_cast<int>(hallOfFame.entries.size()) != hallOfFame.entryCount) {
            throw std::runtime_error(
                "decoded.hallOfFame.entryCount must match decoded.hallOfFame.entries length.");
        }
        for (std::size_t entryIndex = 0; entryIndex < hallOfFame.entries.size(); ++entryIndex) {
            const auto& entry = hallOfFame.entries[entryIndex];
            if (entry.entryNumber != static_cast<int>(entryIndex + 1U)) {
                throw std::runtime_error("decoded.hallOfFame entry numbers must be sequential and 1-based.");
            }
            if (entry.pokemon.size() > 6U) {
                throw std::runtime_error("decoded.hallOfFame entries may contain at most 6 Pokemon.");
            }
            for (std::size_t monIndex = 0; monIndex < entry.pokemon.size(); ++monIndex) {
                const auto& mon = entry.pokemon[monIndex];
                if (mon.partyOrder != static_cast<int>(monIndex + 1U)) {
                    throw std::runtime_error("decoded.hallOfFame partyOrder must be sequential and 1-based.");
                }
                if (mon.level == 0U || mon.level > 100U) {
                    throw std::runtime_error("decoded.hallOfFame Pokemon levels must be in 1..100.");
                }
                if (pokemon::FindSpeciesData(mon.speciesId) == nullptr) {
                    throw std::runtime_error("decoded.hallOfFame contains an unsupported species id.");
                }
                encoding::Gen1TextEncoder::EncodeName(
                    mon.nickname, encoding::Gen1Layout::Gen1NameLen);
            }
        }
    }

    static void WriteHallOfFame(WorkingSaveBuffer& working,
                                const model::HallOfFameState& hallOfFame) {
        ZeroRange(working.bytes, encoding::Gen1Layout::HallOfFameOff, encoding::Gen1Layout::HallOfFameLen);
        encoding::PrimitiveWriter::WriteU8(
            working.bytes,
            encoding::Gen1Layout::HallOfFameRecordCountOff,
            static_cast<std::uint8_t>(hallOfFame.entryCount));

        for (std::size_t entryIndex = 0; entryIndex < hallOfFame.entries.size(); ++entryIndex) {
            const std::size_t entryBase =
                encoding::Gen1Layout::HallOfFameOff + (entryIndex * 0x60U);
            const auto& entry = hallOfFame.entries[entryIndex];
            for (std::size_t monIndex = 0; monIndex < entry.pokemon.size(); ++monIndex) {
                const auto& mon = entry.pokemon[monIndex];
                const std::size_t monBase = entryBase + (monIndex * 0x10U);
                encoding::PrimitiveWriter::WriteU8(working.bytes, monBase + 0x00U, mon.speciesId);
                encoding::PrimitiveWriter::WriteU8(working.bytes, monBase + 0x01U, mon.level);
                encoding::Gen1TextEncoder::WriteName(
                    working.bytes, monBase + 0x02U, encoding::Gen1Layout::Gen1NameLen, mon.nickname);
            }
        }

        MarkRange(working.report,
                  encoding::Gen1Layout::HallOfFameOff,
                  encoding::Gen1Layout::HallOfFameOff + encoding::Gen1Layout::HallOfFameLen - 1U,
                  hallOfFame.entryCount == 0 ? "intentionally-cleared" : "overwritten-from-target",
                  hallOfFame.entryCount == 0 ? "empty Hall of Fame canonical default" : "hallOfFame");
        MarkRange(working.report,
                  encoding::Gen1Layout::HallOfFameRecordCountOff,
                  encoding::Gen1Layout::HallOfFameRecordCountOff,
                  hallOfFame.entryCount == 0 ? "intentionally-cleared" : "overwritten-from-target",
                  "hallOfFame.entryCount");
        working.report.fieldsWritten.push_back("hallOfFame");
    }

private:
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
            "HallOfFameSerializer",
            reason
        });
    }
};

}  // namespace pkmn::savegen::generation
