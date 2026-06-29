#pragma once

#include <algorithm>
#include <cstdint>
#include <string>
#include <vector>

#include "../encoding/BcdEncoder.hpp"
#include "../encoding/Gen1Layout.hpp"
#include "../encoding/Gen1TextEncoder.hpp"
#include "../encoding/PrimitiveWriter.hpp"
#include "../reporting/GenerationReport.hpp"
#include "MinimalStateContract.hpp"
#include "RedSaveInitializer.hpp"

namespace pkmn::savegen::generation {

class CoreStateSerializer {
public:
    static void ApplyMinimalState(const MinimalStateContract& contract, WorkingSaveBuffer& working) {
        WriteName(working, encoding::Gen1Layout::TrainerNameOff, encoding::Gen1Layout::TrainerNameLen,
                  contract.expectedSemantic.identity.playerName, "identity.playerName");
        WriteName(working, encoding::Gen1Layout::RivalNameOff, encoding::Gen1Layout::RivalNameLen,
                  contract.expectedSemantic.identity.rivalName, "identity.rivalName");

        encoding::PrimitiveWriter::WriteU16BigEndian(
            working.bytes, encoding::Gen1Layout::TrainerIdOff, contract.expectedSemantic.identity.trainerId);
        MarkRange(working.report,
                  encoding::Gen1Layout::TrainerIdOff,
                  encoding::Gen1Layout::TrainerIdOff + 1,
                  "overwritten-from-target",
                  "identity.trainerId");

        encoding::PrimitiveWriter::WriteU8(
            working.bytes, encoding::Gen1Layout::OptionsOff, contract.expectedSemantic.core.optionsByte);
        encoding::PrimitiveWriter::WriteU8(
            working.bytes, encoding::Gen1Layout::LetterDelayOff, contract.expectedSemantic.core.letterDelayByte);
        encoding::PrimitiveWriter::WriteU8(
            working.bytes, encoding::Gen1Layout::ContrastOff, contract.expectedSemantic.core.contrast);
        MarkRange(working.report,
                  encoding::Gen1Layout::OptionsOff,
                  encoding::Gen1Layout::OptionsOff,
                  "overwritten-from-target",
                  "core.optionsByte");
        MarkRange(working.report,
                  encoding::Gen1Layout::LetterDelayOff,
                  encoding::Gen1Layout::LetterDelayOff,
                  "overwritten-from-target",
                  "core.letterDelayByte");
        MarkRange(working.report,
                  encoding::Gen1Layout::ContrastOff,
                  encoding::Gen1Layout::ContrastOff,
                  "overwritten-from-target",
                  "core.contrast");

        encoding::BcdEncoder::Write3(
            working.bytes, encoding::Gen1Layout::MoneyOff, contract.expectedSemantic.core.money);
        encoding::BcdEncoder::Write2(
            working.bytes, encoding::Gen1Layout::CoinsOff, contract.expectedSemantic.core.coins);
        MarkRange(working.report,
                  encoding::Gen1Layout::MoneyOff,
                  encoding::Gen1Layout::MoneyOff + 2,
                  "overwritten-from-target",
                  "core.money");
        MarkRange(working.report,
                  encoding::Gen1Layout::CoinsOff,
                  encoding::Gen1Layout::CoinsOff + 1,
                  "overwritten-from-target",
                  "core.coins");

        encoding::PrimitiveWriter::WriteU8(
            working.bytes, encoding::Gen1Layout::PlayHoursOff,
            ClampToByte(contract.expectedSemantic.core.playHours, "playHours"));
        encoding::PrimitiveWriter::WriteU8(working.bytes, encoding::Gen1Layout::PlayMaxedOff, 0);
        encoding::PrimitiveWriter::WriteU8(
            working.bytes, encoding::Gen1Layout::PlayMinutesOff,
            ClampToByte(contract.expectedSemantic.core.playMinutes, "playMinutes"));
        encoding::PrimitiveWriter::WriteU8(
            working.bytes, encoding::Gen1Layout::PlaySecondsOff,
            ClampToByte(contract.expectedSemantic.core.playSeconds, "playSeconds"));
        encoding::PrimitiveWriter::WriteU8(working.bytes, encoding::Gen1Layout::PlayFramesOff, 0);
        MarkRange(working.report,
                  encoding::Gen1Layout::PlayHoursOff,
                  encoding::Gen1Layout::PlayFramesOff,
                  "overwritten-from-target",
                  "core.playtime with canonical frame/maxed defaults");

        encoding::PrimitiveWriter::WriteU8(
            working.bytes, encoding::Gen1Layout::MapIdOff, contract.supportedLocation.mapId);
        encoding::PrimitiveWriter::WriteU8(
            working.bytes, encoding::Gen1Layout::YCoordOff, contract.supportedLocation.y);
        encoding::PrimitiveWriter::WriteU8(
            working.bytes, encoding::Gen1Layout::XCoordOff, contract.supportedLocation.x);
        encoding::PrimitiveWriter::WriteU8(
            working.bytes, encoding::Gen1Layout::YBlockCoordOff, contract.supportedLocation.yBlock);
        encoding::PrimitiveWriter::WriteU8(
            working.bytes, encoding::Gen1Layout::XBlockCoordOff, contract.supportedLocation.xBlock);
        MarkRange(working.report,
                  encoding::Gen1Layout::MapIdOff,
                  encoding::Gen1Layout::XBlockCoordOff,
                  "overwritten-from-target",
                  "core.safeLocation using Milestone 2 supported baseline mapping");

        WriteZeroBadges(working);
        ClearPokedex(working);
        ClearBagItems(working);
        ClearPcItems(working);
        ClearParty(working);
        ClearDaycare(working);
        ClearHallOfFame(working);
    }

private:
    static void WriteName(WorkingSaveBuffer& working,
                          std::size_t offset,
                          std::size_t len,
                          const std::string& value,
                          const std::string& fieldName) {
        encoding::Gen1TextEncoder::WriteName(working.bytes, offset, len, value);
        MarkRange(working.report,
                  offset,
                  offset + len - 1,
                  "overwritten-from-target",
                  fieldName);
    }

    static std::uint8_t ClampToByte(int value, const std::string& fieldName) {
        if (value < 0 || value > 255) {
            throw std::runtime_error(fieldName + " is outside the supported 0..255 range");
        }
        return static_cast<std::uint8_t>(value);
    }

    static void ZeroRange(std::vector<std::uint8_t>& bytes, std::size_t offset, std::size_t len) {
        encoding::PrimitiveWriter::EnsureRange(bytes, offset, len);
        std::fill(bytes.begin() + static_cast<std::ptrdiff_t>(offset),
                  bytes.begin() + static_cast<std::ptrdiff_t>(offset + len),
                  static_cast<std::uint8_t>(0));
    }

    static void WriteZeroBadges(WorkingSaveBuffer& working) {
        encoding::PrimitiveWriter::WriteU8(working.bytes, encoding::Gen1Layout::BadgesOff, 0);
        encoding::PrimitiveWriter::WriteU8(working.bytes, encoding::Gen1Layout::BadgesMirrorOff, 0);
        MarkRange(working.report,
                  encoding::Gen1Layout::BadgesOff,
                  encoding::Gen1Layout::BadgesOff,
                  "intentionally-cleared",
                  "core.badgesBitfield canonical default");
        MarkRange(working.report,
                  encoding::Gen1Layout::BadgesMirrorOff,
                  encoding::Gen1Layout::BadgesMirrorOff,
                  "synchronized-duplicate-cache",
                  "core.badges mirror synchronized to canonical default");
    }

    static void ClearPokedex(WorkingSaveBuffer& working) {
        ZeroRange(working.bytes, encoding::Gen1Layout::PokedexOwnedOff, encoding::Gen1Layout::PokedexBitsLen);
        ZeroRange(working.bytes, encoding::Gen1Layout::PokedexSeenOff, encoding::Gen1Layout::PokedexBitsLen);
        MarkRange(working.report,
                  encoding::Gen1Layout::PokedexOwnedOff,
                  encoding::Gen1Layout::PokedexOwnedOff + encoding::Gen1Layout::PokedexBitsLen - 1,
                  "intentionally-cleared",
                  "empty Pokedex owned bitfield canonical default");
        MarkRange(working.report,
                  encoding::Gen1Layout::PokedexSeenOff,
                  encoding::Gen1Layout::PokedexSeenOff + encoding::Gen1Layout::PokedexBitsLen - 1,
                  "intentionally-cleared",
                  "empty Pokedex seen bitfield canonical default");
    }

    static void ClearBagItems(WorkingSaveBuffer& working) {
        ZeroRange(working.bytes, encoding::Gen1Layout::BagItemsCountOff, 1 + (encoding::Gen1Layout::BagItemsMaxPairs * 2) + 1);
        encoding::PrimitiveWriter::WriteU8(working.bytes, encoding::Gen1Layout::BagItemsPairsOff, 0xFF);
        MarkRange(working.report,
                  encoding::Gen1Layout::BagItemsCountOff,
                  encoding::Gen1Layout::BagItemsCountOff + (1 + (encoding::Gen1Layout::BagItemsMaxPairs * 2)),
                  "intentionally-cleared",
                  "empty bag inventory canonical default");
    }

    static void ClearPcItems(WorkingSaveBuffer& working) {
        ZeroRange(working.bytes, encoding::Gen1Layout::PCItemBoxCountOff, encoding::Gen1Layout::PCItemBoxSerializedLen);
        encoding::PrimitiveWriter::WriteU8(working.bytes, encoding::Gen1Layout::PCItemBoxPairsOff, 0xFF);
        MarkRange(working.report,
                  encoding::Gen1Layout::PCItemBoxCountOff,
                  encoding::Gen1Layout::PCItemBoxCountOff + encoding::Gen1Layout::PCItemBoxSerializedLen - 1,
                  "intentionally-cleared",
                  "empty PC item inventory canonical default");
    }

    static void ClearParty(WorkingSaveBuffer& working) {
        ZeroRange(working.bytes, encoding::Gen1Layout::PartyBase, encoding::Gen1Layout::PartyBlockLen);
        encoding::PrimitiveWriter::WriteU8(working.bytes, encoding::Gen1Layout::PartySpeciesOff, 0xFF);
        MarkRange(working.report,
                  encoding::Gen1Layout::PartyBase,
                  encoding::Gen1Layout::PartyBase + encoding::Gen1Layout::PartyBlockLen - 1,
                  "intentionally-cleared",
                  "empty party canonical default");
    }

    static void ClearDaycare(WorkingSaveBuffer& working) {
        ZeroRange(working.bytes, encoding::Gen1Layout::DaycareInUseOff, encoding::Gen1Layout::DaycareLen);
        MarkRange(working.report,
                  encoding::Gen1Layout::DaycareInUseOff,
                  encoding::Gen1Layout::DaycareInUseOff + encoding::Gen1Layout::DaycareLen - 1,
                  "intentionally-cleared",
                  "empty daycare canonical default");
    }

    static void ClearHallOfFame(WorkingSaveBuffer& working) {
        ZeroRange(working.bytes, encoding::Gen1Layout::HallOfFameOff, encoding::Gen1Layout::HallOfFameLen);
        encoding::PrimitiveWriter::WriteU8(working.bytes, encoding::Gen1Layout::HallOfFameRecordCountOff, 0);
        MarkRange(working.report,
                  encoding::Gen1Layout::HallOfFameOff,
                  encoding::Gen1Layout::HallOfFameOff + encoding::Gen1Layout::HallOfFameLen - 1,
                  "intentionally-cleared",
                  "empty Hall of Fame canonical default");
        MarkRange(working.report,
                  encoding::Gen1Layout::HallOfFameRecordCountOff,
                  encoding::Gen1Layout::HallOfFameRecordCountOff,
                  "intentionally-cleared",
                  "Hall of Fame entry count canonical default");
    }

    static void MarkRange(reporting::GenerationReport& report,
                          std::size_t start,
                          std::size_t endInclusive,
                          const std::string& classification,
                          const std::string& reason) {
        report.ranges.push_back({start, endInclusive, classification, reason});
        report.fieldsWritten.push_back(reason);
    }
};

}  // namespace pkmn::savegen::generation
