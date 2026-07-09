#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <map>
#include <set>
#include <stdexcept>
#include <string>
#include <vector>

#include "../encoding/BitfieldWriter.hpp"
#include "../encoding/Gen1Layout.hpp"
#include "../encoding/PrimitiveWriter.hpp"
#include "../reporting/GenerationReport.hpp"
#include "RedSaveInitializer.hpp"

namespace pkmn::savegen::generation {

class ExtendedWorldSerializer {
public:
    static void ValidateOrThrow(const model::RedSemanticState& semantic) {
        ValidateMissableObjects(semantic.missableObjects);
        ValidateScripts(semantic.scripts);
        ValidateNamedFlagCollection(semantic.events, "decoded.events.flags");
        ValidateNamedFlagCollection(semantic.trainerBattles, "decoded.trainerBattles.records");
        ValidateNamedFlagCollection(semantic.staticBattles, "decoded.staticBattles.records");
        ValidateNamedFlagCollection(semantic.storyProgress, "decoded.storyProgress.storyFlags");
        (void)MergeNamedFlags(semantic);
    }

    static void WritePersistentWorldState(WorkingSaveBuffer& working,
                                          const model::RedSemanticState& semantic) {
        ValidateOrThrow(semantic);
        WriteMissableObjects(working, semantic.missableObjects);
        WriteNamedEventFlags(working, semantic);
        WriteScripts(working, semantic.scripts);
        WriteRuntimeState(working, semantic.core);
    }

private:
    static void WriteMissableObjects(WorkingSaveBuffer& working,
                                     const std::vector<model::MissableObjectState>& entries) {
        ZeroRange(
            working.bytes,
            encoding::Gen1Layout::MissableObjectsOff,
            encoding::Gen1Layout::MissableObjectsLen);
        for (const auto& entry : entries) {
            WriteBit(working.bytes,
                     encoding::Gen1Layout::MissableObjectsOff,
                     entry.index,
                     entry.toggledOff);
        }
        MarkRange(working.report,
                  encoding::Gen1Layout::MissableObjectsOff,
                  encoding::Gen1Layout::MissableObjectsOff + encoding::Gen1Layout::MissableObjectsLen - 1U,
                  "overwritten-from-target",
                  "missableObjects");
        working.report.fieldsWritten.push_back("missableObjects");
    }

    static void WriteNamedEventFlags(WorkingSaveBuffer& working,
                                     const model::RedSemanticState& semantic) {
        const auto merged = MergeNamedFlags(semantic);
        ZeroRange(
            working.bytes,
            encoding::Gen1Layout::EventFlagsOff,
            encoding::Gen1Layout::EventFlagsLen);
        for (const auto& [flagIndex, value] : merged) {
            if (flagIndex < 0 ||
                flagIndex >= static_cast<int>(encoding::Gen1Layout::EventFlagsUsedBits)) {
                throw std::runtime_error("decoded event-state contains an out-of-range flag index.");
            }
            WriteBit(working.bytes,
                     encoding::Gen1Layout::EventFlagsOff,
                     flagIndex,
                     value);
        }
        MarkRange(working.report,
                  encoding::Gen1Layout::EventFlagsOff,
                  encoding::Gen1Layout::EventFlagsOff + encoding::Gen1Layout::EventFlagsLen - 1U,
                  "overwritten-from-target",
                  "named event flags");
        working.report.fieldsWritten.push_back("events");
        working.report.fieldsWritten.push_back("trainerBattles");
        working.report.fieldsWritten.push_back("staticBattles");
        working.report.fieldsWritten.push_back("storyProgress");
    }

    static std::map<int, bool> MergeNamedFlags(const model::RedSemanticState& semantic) {
        std::map<int, bool> merged;
        AppendNamedFlags(merged, semantic.events, "decoded.events");
        AppendNamedFlags(merged, semantic.trainerBattles, "decoded.trainerBattles");
        AppendNamedFlags(merged, semantic.staticBattles, "decoded.staticBattles");
        AppendNamedFlags(merged, semantic.storyProgress, "decoded.storyProgress");
        return merged;
    }

    static void AppendNamedFlags(std::map<int, bool>& merged,
                                 const std::vector<model::NamedFlagState>& flags,
                                 const std::string& sourceName) {
        for (const auto& flag : flags) {
            if (flag.flagIndex < 0 ||
                flag.flagIndex >= static_cast<int>(encoding::Gen1Layout::EventFlagsUsedBits)) {
                throw std::runtime_error(sourceName + " contains an out-of-range flag index.");
            }
            const auto [it, inserted] = merged.emplace(flag.flagIndex, flag.set);
            if (!inserted && it->second != flag.set) {
                throw std::runtime_error(
                    sourceName + " disagrees with another named-flag source for flag index " +
                    std::to_string(flag.flagIndex) + ".");
            }
        }
    }

    static void WriteScripts(WorkingSaveBuffer& working,
                             const std::vector<model::ScriptState>& scripts) {
        ZeroRange(
            working.bytes,
            encoding::Gen1Layout::CurrentScriptsOff,
            encoding::Gen1Layout::CurrentScriptsLen);
        for (const auto& script : scripts) {
            if (script.relativeOffset < 0 ||
                script.relativeOffset + script.size >
                    static_cast<int>(encoding::Gen1Layout::CurrentScriptsLen)) {
                throw std::runtime_error("decoded.scripts contains an out-of-range relativeOffset/size.");
            }
            const std::size_t base =
                encoding::Gen1Layout::CurrentScriptsOff + static_cast<std::size_t>(script.relativeOffset);
            if (script.size == 2) {
                if (script.value < 0 || script.value > 0xFFFF) {
                    throw std::runtime_error("decoded.scripts 2-byte value exceeds 16 bits.");
                }
                encoding::PrimitiveWriter::WriteU16BigEndian(
                    working.bytes, base, static_cast<std::uint16_t>(script.value));
            } else if (script.size == 1) {
                if (script.value < 0 || script.value > 0xFF) {
                    throw std::runtime_error("decoded.scripts 1-byte value exceeds 8 bits.");
                }
                encoding::PrimitiveWriter::WriteU8(
                    working.bytes, base, static_cast<std::uint8_t>(script.value));
            } else {
                throw std::runtime_error("decoded.scripts contains an unsupported size.");
            }
        }
        MarkRange(working.report,
                  encoding::Gen1Layout::CurrentScriptsOff,
                  encoding::Gen1Layout::CurrentScriptsOff + encoding::Gen1Layout::CurrentScriptsLen - 1U,
                  "overwritten-from-target",
                  "scripts");
        working.report.fieldsWritten.push_back("scripts");
    }

    static void WriteRuntimeState(WorkingSaveBuffer& working,
                                  const model::CoreState& core) {
        encoding::PrimitiveWriter::WriteU8(
            working.bytes,
            encoding::Gen1Layout::WalkBikeSurfOff,
            EncodeMovementMode(core.movementMode));
        encoding::PrimitiveWriter::WriteU8(
            working.bytes,
            encoding::Gen1Layout::PlayerMoveDirOff,
            EncodeDirection(core.playerMoveDirection));
        encoding::PrimitiveWriter::WriteU8(
            working.bytes,
            encoding::Gen1Layout::PlayerCurDirOff,
            EncodeDirection(core.playerCurrentDirection));
        encoding::PrimitiveWriter::WriteU8(
            working.bytes,
            encoding::Gen1Layout::PlayerLastStopDirOff,
            EncodeDirection(core.playerCurrentDirection));
        encoding::PrimitiveWriter::WriteU16BigEndian(
            working.bytes,
            encoding::Gen1Layout::SafariStepsOff,
            core.safariSteps);
        encoding::PrimitiveWriter::WriteU8(
            working.bytes,
            encoding::Gen1Layout::SafariGameOverOff,
            core.safariGameOver ? 1U : 0U);
        encoding::PrimitiveWriter::WriteU8(
            working.bytes,
            encoding::Gen1Layout::SafariBallCountOff,
            core.safariBallCount);

        WriteIndexedBit(working.bytes, encoding::Gen1Layout::WorldFlags1Off, 0, core.strengthOutsideBattle);
        WriteIndexedBit(working.bytes, encoding::Gen1Layout::WorldFlags1Off, 1, core.surfingAllowed);
        WriteIndexedBit(working.bytes, encoding::Gen1Layout::WorldFlags1Off, 3, core.gotOldRod);
        WriteIndexedBit(working.bytes, encoding::Gen1Layout::WorldFlags1Off, 4, core.gotGoodRod);
        WriteIndexedBit(working.bytes, encoding::Gen1Layout::WorldFlags1Off, 5, core.gotSuperRod);
        WriteIndexedBit(working.bytes, encoding::Gen1Layout::WorldFlags1Off, 6, core.satisfiedSaffronGuards);
        WriteIndexedBit(working.bytes, encoding::Gen1Layout::BattleFlagsOff, 6, core.isBattle);
        WriteIndexedBit(working.bytes, encoding::Gen1Layout::BattleFlagsOff, 7, core.isTrainerBattle);
        WriteIndexedBit(working.bytes, encoding::Gen1Layout::WorldFlags2Off, 0, core.gotLapras);
        WriteIndexedBit(working.bytes, encoding::Gen1Layout::WorldFlags2Off, 2, core.everHealedPokemon);
        WriteIndexedBit(working.bytes, encoding::Gen1Layout::WorldFlags2Off, 3, core.gotStarter);
        WriteIndexedBit(working.bytes, encoding::Gen1Layout::FlyFlagsOff, 7, core.flyOutOfBattle);
        WriteIndexedBit(working.bytes, encoding::Gen1Layout::EliteFlagsOff, 1, core.defeatedLoreleiRoomState);
        WriteIndexedBit(working.bytes, encoding::Gen1Layout::PlaytimeFlagsOff, 0, core.countPlaytime);

        MarkRange(working.report,
                  encoding::Gen1Layout::WalkBikeSurfOff,
                  encoding::Gen1Layout::WalkBikeSurfOff,
                  "overwritten-from-target",
                  "runtimeState.movementMode");
        MarkRange(working.report,
                  encoding::Gen1Layout::PlayerMoveDirOff,
                  encoding::Gen1Layout::PlayerCurDirOff,
                  "overwritten-from-target",
                  "runtimeState.playerDirections");
        MarkRange(working.report,
                  encoding::Gen1Layout::SafariStepsOff,
                  encoding::Gen1Layout::SafariStepsOff + 1U,
                  "overwritten-from-target",
                  "runtimeState.safari.steps");
        MarkRange(working.report,
                  encoding::Gen1Layout::SafariGameOverOff,
                  encoding::Gen1Layout::SafariGameOverOff,
                  "overwritten-from-target",
                  "runtimeState.safari.gameOver");
        MarkRange(working.report,
                  encoding::Gen1Layout::SafariBallCountOff,
                  encoding::Gen1Layout::SafariBallCountOff,
                  "overwritten-from-target",
                  "runtimeState.safari.ballCount");
        MarkRange(working.report,
                  encoding::Gen1Layout::WorldFlags1Off,
                  encoding::Gen1Layout::WorldFlags1Off,
                  "overwritten-from-target",
                  "runtimeState.worldFlags1");
        MarkRange(working.report,
                  encoding::Gen1Layout::BattleFlagsOff,
                  encoding::Gen1Layout::BattleFlagsOff,
                  "overwritten-from-target",
                  "runtimeState.battleFlags");
        MarkRange(working.report,
                  encoding::Gen1Layout::WorldFlags2Off,
                  encoding::Gen1Layout::WorldFlags2Off,
                  "overwritten-from-target",
                  "worldState.storyEvidence.worldFlags2");
        MarkRange(working.report,
                  encoding::Gen1Layout::FlyFlagsOff,
                  encoding::Gen1Layout::FlyFlagsOff,
                  "overwritten-from-target",
                  "runtimeState.flyFlags");
        MarkRange(working.report,
                  encoding::Gen1Layout::EliteFlagsOff,
                  encoding::Gen1Layout::EliteFlagsOff,
                  "overwritten-from-target",
                  "worldState.storyEvidence.eliteFlags");
        MarkRange(working.report,
                  encoding::Gen1Layout::PlaytimeFlagsOff,
                  encoding::Gen1Layout::PlaytimeFlagsOff,
                  "overwritten-from-target",
                  "runtimeState.playtimeFlags");
        working.report.fieldsWritten.push_back("runtimeState");
    }

    static void ValidateMissableObjects(
        const std::vector<model::MissableObjectState>& entries) {
        if (entries.size() != static_cast<std::size_t>(encoding::Gen1Layout::MissableObjectsUsedBits)) {
            throw std::runtime_error(
                "decoded.missableObjects must contain the complete 228-entry named list.");
        }
        std::set<int> seen;
        for (std::size_t i = 0; i < entries.size(); ++i) {
            const auto& entry = entries[i];
            if (entry.index < 0 ||
                entry.index >= static_cast<int>(encoding::Gen1Layout::MissableObjectsUsedBits)) {
                throw std::runtime_error("decoded.missableObjects contains an out-of-range index.");
            }
            if (entry.index != static_cast<int>(i)) {
                throw std::runtime_error("decoded.missableObjects must be complete and sequential.");
            }
            if (!seen.insert(entry.index).second) {
                throw std::runtime_error("decoded.missableObjects contains a duplicate index.");
            }
        }
    }

    static void ValidateNamedFlagCollection(
        const std::vector<model::NamedFlagState>& flags,
        const std::string& label) {
        std::set<int> seen;
        for (const auto& flag : flags) {
            if (flag.flagIndex < 0 ||
                flag.flagIndex >= static_cast<int>(encoding::Gen1Layout::EventFlagsUsedBits)) {
                throw std::runtime_error(label + " contains an out-of-range flag index.");
            }
            if (!seen.insert(flag.flagIndex).second) {
                throw std::runtime_error(label + " contains a duplicate flag index.");
            }
        }
    }

    static void ValidateScripts(const std::vector<model::ScriptState>& scripts) {
        if (scripts.size() != static_cast<std::size_t>(encoding::Gen1Layout::CurrentScriptCount)) {
            throw std::runtime_error(
                "decoded.scripts.scripts must contain the complete 97-entry named script list.");
        }

        std::set<int> seenIndices;
        std::set<int> touchedBytes;
        for (const auto& script : scripts) {
            if (script.index < 0 ||
                script.index >= encoding::Gen1Layout::CurrentScriptCount) {
                throw std::runtime_error("decoded.scripts contains an out-of-range script index.");
            }
            if (!seenIndices.insert(script.index).second) {
                throw std::runtime_error("decoded.scripts contains a duplicate script index.");
            }
            if (script.size != 1 && script.size != 2) {
                throw std::runtime_error("decoded.scripts contains an unsupported size.");
            }
            if (script.relativeOffset < 0 ||
                script.relativeOffset + script.size >
                    static_cast<int>(encoding::Gen1Layout::CurrentScriptsLen)) {
                throw std::runtime_error("decoded.scripts contains an out-of-range relativeOffset/size.");
            }
            if ((script.size == 1 && (script.value < 0 || script.value > 0xFF)) ||
                (script.size == 2 && (script.value < 0 || script.value > 0xFFFF))) {
                throw std::runtime_error("decoded.scripts contains a value outside its declared size.");
            }
            for (int byteIndex = 0; byteIndex < script.size; ++byteIndex) {
                if (!touchedBytes.insert(script.relativeOffset + byteIndex).second) {
                    throw std::runtime_error("decoded.scripts contains overlapping script byte ranges.");
                }
            }
        }
    }

    static std::uint8_t EncodeMovementMode(const std::string& movementMode) {
        if (movementMode == "Walking") return 0x00;
        if (movementMode == "Biking") return 0x01;
        if (movementMode == "Surfing") return 0x02;
        throw std::runtime_error("Unsupported movementMode: " + movementMode);
    }

    static std::uint8_t EncodeDirection(const std::string& direction) {
        if (direction == "None") return 0x00;
        if (direction == "Right") return 0x01;
        if (direction == "Left") return 0x02;
        if (direction == "Down") return 0x04;
        if (direction == "Up") return 0x08;
        throw std::runtime_error("Unsupported direction value: " + direction);
    }

    static void WriteBit(std::vector<std::uint8_t>& bytes,
                         std::size_t byteBase,
                         int bitIndex,
                         bool value) {
        encoding::BitfieldWriter::WriteBit(
            bytes,
            byteBase + static_cast<std::size_t>(bitIndex / 8),
            static_cast<std::uint8_t>(bitIndex % 8),
            value);
    }

    static void WriteIndexedBit(std::vector<std::uint8_t>& bytes,
                                std::size_t byteOffset,
                                std::uint8_t bitIndex,
                                bool value) {
        encoding::BitfieldWriter::WriteBit(bytes, byteOffset, bitIndex, value);
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
            "ExtendedWorldSerializer",
            reason
        });
    }
};

}  // namespace pkmn::savegen::generation
