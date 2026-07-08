#pragma once

#include <cstddef>
#include <cstdint>
#include <map>
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
    static void WritePersistentWorldState(WorkingSaveBuffer& working,
                                          const model::RedSemanticState& semantic) {
        WriteMissableObjects(working, semantic.missableObjects);
        WriteNamedEventFlags(working, semantic);
        WriteScripts(working, semantic.scripts);
        WriteRuntimeState(working, semantic.core);
    }

private:
    static void WriteMissableObjects(WorkingSaveBuffer& working,
                                     const std::vector<model::MissableObjectState>& entries) {
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
        WriteIndexedBit(working.bytes, encoding::Gen1Layout::BattleFlagsOff, 6, core.isBattle);
        WriteIndexedBit(working.bytes, encoding::Gen1Layout::BattleFlagsOff, 7, core.isTrainerBattle);
        WriteIndexedBit(working.bytes, encoding::Gen1Layout::FlyFlagsOff, 7, core.flyOutOfBattle);
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
                  encoding::Gen1Layout::FlyFlagsOff,
                  encoding::Gen1Layout::FlyFlagsOff,
                  "overwritten-from-target",
                  "runtimeState.flyFlags");
        MarkRange(working.report,
                  encoding::Gen1Layout::PlaytimeFlagsOff,
                  encoding::Gen1Layout::PlaytimeFlagsOff,
                  "overwritten-from-target",
                  "runtimeState.playtimeFlags");
        working.report.fieldsWritten.push_back("runtimeState");
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
