#pragma once

#include <algorithm>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <vector>

#include "../encoding/BcdEncoder.hpp"
#include "../encoding/BitfieldWriter.hpp"
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
        encoding::PrimitiveWriter::WriteU8(
            working.bytes, encoding::Gen1Layout::PreviousMapOff, contract.supportedLocation.previousMap);
        MarkRange(working.report,
                  encoding::Gen1Layout::MapIdOff,
                  encoding::Gen1Layout::PreviousMapOff,
                  contract.locationWasCanonicalized ? "canonicalized" : "overwritten-from-target",
                  contract.locationWasCanonicalized
                      ? "core.safeLocation canonicalized to Red's house second floor"
                      : "core.safeLocation using supported baseline mapping");

        WriteBadges(working, contract.expectedSemantic.core.badgesBitfield);
        WritePokedex(working, contract.expectedSemantic.pokedex);
        WriteItemList(working,
                      encoding::Gen1Layout::BagItemsCountOff,
                      encoding::Gen1Layout::BagItemsPairsOff,
                      encoding::Gen1Layout::BagItemsMaxPairs,
                      contract.expectedSemantic.inventory.bagItems,
                      "inventory.bagItems");
        WriteItemList(working,
                      encoding::Gen1Layout::PCItemBoxCountOff,
                      encoding::Gen1Layout::PCItemBoxPairsOff,
                      encoding::Gen1Layout::PCItemBoxMaxPairs,
                      contract.expectedSemantic.inventory.pcItems,
                      "inventory.pcItems");
        WriteEventSubset(working,
                         contract.expectedSemantic.visitedTowns,
                         contract.expectedSemantic.hiddenItems,
                         contract.expectedSemantic.hiddenCoins);
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

    static void WriteBadges(WorkingSaveBuffer& working, std::uint8_t badgesBitfield) {
        encoding::PrimitiveWriter::WriteU8(working.bytes, encoding::Gen1Layout::BadgesOff, badgesBitfield);
        encoding::PrimitiveWriter::WriteU8(
            working.bytes, encoding::Gen1Layout::BadgesMirrorOff, badgesBitfield);
        MarkRange(working.report,
                  encoding::Gen1Layout::BadgesOff,
                  encoding::Gen1Layout::BadgesOff,
                  "overwritten-from-target",
                  "core.badgesBitfield");
        MarkRange(working.report,
                  encoding::Gen1Layout::BadgesMirrorOff,
                  encoding::Gen1Layout::BadgesMirrorOff,
                  "synchronized-duplicate-cache",
                  "core.badges mirror synchronized");
    }

    static void WritePokedex(WorkingSaveBuffer& working, const model::PokedexState& pokedex) {
        ZeroRange(working.bytes, encoding::Gen1Layout::PokedexOwnedOff, encoding::Gen1Layout::PokedexBitsLen);
        ZeroRange(working.bytes, encoding::Gen1Layout::PokedexSeenOff, encoding::Gen1Layout::PokedexBitsLen);
        WriteBits(working.bytes, encoding::Gen1Layout::PokedexOwnedOff, pokedex.owned);
        WriteBits(working.bytes, encoding::Gen1Layout::PokedexSeenOff, pokedex.seen);
        MarkRange(working.report,
                  encoding::Gen1Layout::PokedexOwnedOff,
                  encoding::Gen1Layout::PokedexOwnedOff + encoding::Gen1Layout::PokedexBitsLen - 1,
                  "overwritten-from-target",
                  "pokedex.owned bitfield");
        MarkRange(working.report,
                  encoding::Gen1Layout::PokedexSeenOff,
                  encoding::Gen1Layout::PokedexSeenOff + encoding::Gen1Layout::PokedexBitsLen - 1,
                  "overwritten-from-target",
                  "pokedex.seen bitfield");
    }

    static void WriteItemList(WorkingSaveBuffer& working,
                              std::size_t countOffset,
                              std::size_t pairsOffset,
                              int maxPairs,
                              const std::vector<model::InventoryItem>& items,
                              const std::string& fieldName) {
        const std::size_t serializedLen = 1 + (static_cast<std::size_t>(maxPairs) * 2U) + 1U;
        ZeroRange(working.bytes, countOffset, serializedLen);
        encoding::PrimitiveWriter::WriteU8(
            working.bytes, countOffset, static_cast<std::uint8_t>(items.size()));

        std::size_t cursor = pairsOffset;
        for (const auto& item : items) {
            encoding::PrimitiveWriter::WriteU8(working.bytes, cursor, item.id);
            encoding::PrimitiveWriter::WriteU8(working.bytes, cursor + 1U, item.quantity);
            cursor += 2U;
        }
        encoding::PrimitiveWriter::WriteU8(working.bytes, cursor, 0xFF);

        MarkRange(working.report,
                  countOffset,
                  countOffset + serializedLen - 1U,
                  "overwritten-from-target",
                  fieldName);
    }

    static void WriteEventSubset(
        WorkingSaveBuffer& working,
        const std::vector<model::VisitedTownState>& visitedTowns,
        const std::vector<model::HiddenObjectState>& hiddenItems,
        const std::vector<model::HiddenObjectState>& hiddenCoins) {
        std::vector<bool> visitedTownBits;
        visitedTownBits.reserve(visitedTowns.size());
        for (const auto& entry : visitedTowns) {
            visitedTownBits.push_back(entry.visited);
        }
        std::vector<bool> hiddenItemBits;
        hiddenItemBits.reserve(hiddenItems.size());
        for (const auto& entry : hiddenItems) {
            hiddenItemBits.push_back(entry.collected);
        }
        std::vector<bool> hiddenCoinBits;
        hiddenCoinBits.reserve(hiddenCoins.size());
        for (const auto& entry : hiddenCoins) {
            hiddenCoinBits.push_back(entry.collected);
        }
        WriteBooleanBitfieldRange(working,
                                  encoding::Gen1Layout::VisitedTownsOff,
                                  encoding::Gen1Layout::VisitedTownsLen,
                                  visitedTownBits,
                                  "visitedTowns");
        WriteBooleanBitfieldRange(working,
                                  encoding::Gen1Layout::HiddenItemsOff,
                                  encoding::Gen1Layout::HiddenItemsLen,
                                  hiddenItemBits,
                                  "hiddenItems");
        WriteBooleanBitfieldRange(working,
                                  encoding::Gen1Layout::HiddenCoinsOff,
                                  encoding::Gen1Layout::HiddenCoinsLen,
                                  hiddenCoinBits,
                                  "hiddenCoins");
    }

    static void WriteBits(std::vector<std::uint8_t>& bytes,
                          std::size_t byteOffset,
                          const std::vector<bool>& bits) {
        for (std::size_t i = 0; i < bits.size(); ++i) {
            encoding::BitfieldWriter::WriteBit(
                bytes,
                byteOffset + (i / 8U),
                static_cast<std::uint8_t>(i % 8U),
                bits[i]);
        }
    }

    static void WriteBooleanBitfieldRange(WorkingSaveBuffer& working,
                                          std::size_t byteOffset,
                                          std::size_t byteLen,
                                          const std::vector<bool>& bits,
                                          const std::string& fieldName) {
        ZeroRange(working.bytes, byteOffset, byteLen);
        WriteBits(working.bytes, byteOffset, bits);
        MarkRange(working.report,
                  byteOffset,
                  byteOffset + byteLen - 1U,
                  "overwritten-from-target",
                  fieldName);
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
            "CoreStateSerializer",
            reason
        });
        report.fieldsWritten.push_back(reason);
    }
};

}  // namespace pkmn::savegen::generation
