#pragma once

#include <algorithm>
#include <array>
#include <cstdint>
#include <optional>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "../../third_party/nlohmann/json.hpp"
#include "../pokemon/Gen1PokemonData.hpp"

namespace pkmn::savegen::model {

struct InventoryItem {
    std::uint8_t id = 0;
    std::string name;
    std::uint8_t quantity = 0;

    bool operator==(const InventoryItem&) const = default;
};

struct IdentityState {
    std::string playerName;
    std::string rivalName;
    std::uint16_t trainerId = 0;

    bool operator==(const IdentityState&) const = default;
};

struct CoreState {
    std::uint8_t optionsByte = 0;
    std::uint8_t letterDelayByte = 0;
    std::uint8_t contrast = 0;
    std::uint8_t badgesBitfield = 0;
    std::uint8_t mapId = 0;
    std::uint8_t x = 0;
    std::uint8_t y = 0;
    std::uint8_t previousMapId = 0;
    std::uint8_t xBlockCoord = 0;
    std::uint8_t yBlockCoord = 0;
    std::string movementMode;
    std::string playerMoveDirection;
    std::string playerCurrentDirection;
    bool strengthOutsideBattle = false;
    bool surfingAllowed = false;
    bool flyOutOfBattle = false;
    bool isBattle = false;
    bool isTrainerBattle = false;
    bool countPlaytime = false;
    bool gotOldRod = false;
    bool gotGoodRod = false;
    bool gotSuperRod = false;
    bool satisfiedSaffronGuards = false;
    bool gotLapras = false;
    bool everHealedPokemon = false;
    bool gotStarter = false;
    bool defeatedLoreleiRoomState = false;
    bool safariGameOver = false;
    std::uint8_t safariBallCount = 0;
    std::uint16_t safariSteps = 0;
    std::uint32_t money = 0;
    std::uint16_t coins = 0;
    int playHours = 0;
    int playMinutes = 0;
    int playSeconds = 0;

    bool operator==(const CoreState&) const = default;
};

struct PokedexState {
    std::vector<bool> owned;
    std::vector<bool> seen;
    std::size_t ownedCount = 0;
    std::size_t seenCount = 0;

    bool operator==(const PokedexState&) const = default;
};

struct InventoryState {
    std::vector<InventoryItem> bagItems;
    std::vector<InventoryItem> pcItems;

    bool operator==(const InventoryState&) const = default;
};

struct PokemonMoveState {
    std::uint8_t moveId = 0;
    std::string moveName;
    std::uint8_t ppCurrent = 0;
    std::uint8_t ppUps = 0;

    bool operator==(const PokemonMoveState&) const = default;
};

struct PokemonStatExperienceState {
    std::uint16_t hp = 0;
    std::uint16_t attack = 0;
    std::uint16_t defense = 0;
    std::uint16_t speed = 0;
    std::uint16_t special = 0;

    bool operator==(const PokemonStatExperienceState&) const = default;
};

struct PokemonDVState {
    std::uint8_t hp = 0;
    std::uint8_t attack = 0;
    std::uint8_t defense = 0;
    std::uint8_t speed = 0;
    std::uint8_t special = 0;

    bool operator==(const PokemonDVState&) const = default;
};

struct PartyPokemonState {
    int position = 0;
    std::uint8_t speciesId = 0;
    std::string speciesName;
    std::uint8_t nationalDexNumber = 0;
    std::string nickname;
    std::string originalTrainerName;
    std::uint16_t originalTrainerId = 0;
    std::uint8_t level = 0;
    std::uint32_t experience = 0;
    std::uint8_t statusRaw = 0;
    std::uint8_t type1 = 0;
    std::uint8_t type2 = 0;
    std::uint8_t catchRate = 0;
    std::uint16_t currentHp = 0;
    std::uint16_t maxHp = 0;
    std::uint16_t attack = 0;
    std::uint16_t defense = 0;
    std::uint16_t speed = 0;
    std::uint16_t special = 0;
    std::vector<PokemonMoveState> moves;
    PokemonStatExperienceState statExperience;
    PokemonDVState dvs;

    bool operator==(const PartyPokemonState&) const = default;
};

struct PartyState {
    int count = 0;
    std::vector<PartyPokemonState> pokemon;

    bool operator==(const PartyState&) const = default;
};

struct StoredPokemonState {
    int position = 0;
    std::uint8_t speciesId = 0;
    std::string speciesName;
    std::uint8_t nationalDexNumber = 0;
    std::string nickname;
    std::string originalTrainerName;
    std::uint16_t originalTrainerId = 0;
    std::uint8_t level = 0;
    std::uint32_t experience = 0;
    std::uint8_t statusRaw = 0;
    std::uint8_t type1 = 0;
    std::uint8_t type2 = 0;
    std::uint8_t catchRate = 0;
    std::uint16_t currentHp = 0;
    std::vector<PokemonMoveState> moves;
    PokemonStatExperienceState statExperience;
    PokemonDVState dvs;

    bool operator==(const StoredPokemonState&) const = default;
};

struct StorageBoxState {
    int boxNumber = 0;
    int count = 0;
    std::vector<StoredPokemonState> pokemon;

    bool operator==(const StorageBoxState&) const = default;
};

struct StorageState {
    std::vector<StorageBoxState> boxes;
    int selectedBoxNumber = 1;
    bool boxChangedFlag = false;
    bool hasCurrentBoxCache = false;
    StorageBoxState currentBoxCache;
    std::string inputSynchronizationStatus;

    bool operator==(const StorageState&) const = default;
};

struct DaycareState {
    bool inUse = false;
    std::optional<StoredPokemonState> pokemon;

    bool operator==(const DaycareState&) const = default;
};

struct HallOfFamePokemonState {
    int partyOrder = 0;
    std::uint8_t speciesId = 0;
    std::string speciesName;
    std::uint8_t nationalDexNumber = 0;
    std::uint8_t level = 0;
    std::string nickname;

    bool operator==(const HallOfFamePokemonState&) const = default;
};

struct HallOfFameEntryState {
    int entryNumber = 0;
    std::vector<HallOfFamePokemonState> pokemon;

    bool operator==(const HallOfFameEntryState&) const = default;
};

struct HallOfFameState {
    int entryCount = 0;
    std::vector<HallOfFameEntryState> entries;

    bool operator==(const HallOfFameState&) const = default;
};

struct NamedFlagState {
    int flagIndex = 0;
    std::string name;
    std::string description;
    bool set = false;
    std::string category;
    std::string location;
    int trainerNumber = 0;

    bool operator==(const NamedFlagState&) const = default;
};

struct ScriptState {
    int index = 0;
    int relativeOffset = 0;
    int size = 0;
    int value = 0;
    std::string mapOrScriptName;

    bool operator==(const ScriptState&) const = default;
};

struct MissableObjectState {
    int index = 0;
    std::string name;
    std::string location;
    bool toggledOff = false;

    bool operator==(const MissableObjectState&) const = default;
};

struct HiddenObjectState {
    int index = 0;
    std::string name;
    std::string location;
    int x = 0;
    int y = 0;
    bool collected = false;

    bool operator==(const HiddenObjectState&) const = default;
};

struct VisitedTownState {
    int index = 0;
    std::string name;
    bool visited = false;

    bool operator==(const VisitedTownState&) const = default;
};

struct RedSemanticState {
    std::string schemaVersion;
    std::string targetGame;
    std::string regionAssumption;
    std::string sourceSha256;
    bool physicalImageIgnored = false;
    IdentityState identity;
    CoreState core;
    PokedexState pokedex;
    InventoryState inventory;
    PartyState party;
    StorageState storage;
    DaycareState daycare;
    HallOfFameState hallOfFame;
    std::vector<NamedFlagState> events;
    std::vector<NamedFlagState> trainerBattles;
    std::vector<NamedFlagState> staticBattles;
    std::vector<NamedFlagState> storyProgress;
    std::vector<ScriptState> scripts;
    std::vector<MissableObjectState> missableObjects;
    std::vector<HiddenObjectState> hiddenItems;
    std::vector<HiddenObjectState> hiddenCoins;
    std::vector<VisitedTownState> visitedTowns;

    bool operator==(const RedSemanticState&) const = default;
};

struct BuildSemanticStateResult {
    bool ok = false;
    RedSemanticState state;
    std::vector<std::string> errors;
};

class RedSemanticStateBuilder {
public:
    static BuildSemanticStateResult Build(const nlohmann::json& sanitizedDocument,
                                          bool physicalImageIgnored) {
        BuildSemanticStateResult result;

        try {
            const nlohmann::json& schema = sanitizedDocument.at("schema");
            const nlohmann::json& source = sanitizedDocument.at("source");
            const nlohmann::json& decoded = sanitizedDocument.at("decoded");

            RedSemanticState state;
            state.schemaVersion = schema.at("schemaVersion").get<std::string>();
            state.targetGame = schema.at("game").get<std::string>();
            state.regionAssumption = schema.at("regionAssumption").get<std::string>();
            state.sourceSha256 = source.at("hashes").at("wholeFileSha256").get<std::string>();
            state.physicalImageIgnored = physicalImageIgnored;

            state.identity.playerName = decoded.at("trainer").at("name").at("value").get<std::string>();
            state.identity.rivalName = decoded.at("rival").at("name").at("value").get<std::string>();
            state.identity.trainerId =
                static_cast<std::uint16_t>(decoded.at("trainer").at("trainerId").at("value").get<int>());

            state.core.optionsByte =
                static_cast<std::uint8_t>(decoded.at("options").at("optionsByte").at("value").get<int>());
            state.core.letterDelayByte =
                static_cast<std::uint8_t>(decoded.at("options").at("letterDelayByte").at("value").get<int>());
            state.core.contrast =
                static_cast<std::uint8_t>(decoded.at("options").at("contrast").at("value").get<int>());
            state.core.badgesBitfield =
                ParseHexByte(decoded.at("badges").at("rawBitfield").get<std::string>());
            state.core.mapId =
                static_cast<std::uint8_t>(decoded.at("location").at("map").at("id").get<int>());
            state.core.x =
                static_cast<std::uint8_t>(decoded.at("location").at("x").at("value").get<int>());
            state.core.y =
                static_cast<std::uint8_t>(decoded.at("location").at("y").at("value").get<int>());
            state.core.previousMapId = static_cast<std::uint8_t>(
                decoded.at("location").at("previousMap").at("id").get<int>());
            state.core.xBlockCoord =
                static_cast<std::uint8_t>(decoded.at("runtimeState").at("xBlockCoord").get<int>());
            state.core.yBlockCoord =
                static_cast<std::uint8_t>(decoded.at("runtimeState").at("yBlockCoord").get<int>());
            state.core.movementMode =
                decoded.at("runtimeState").at("movementMode").get<std::string>();
            state.core.playerMoveDirection =
                decoded.at("runtimeState").at("playerMoveDirection").get<std::string>();
            state.core.playerCurrentDirection =
                decoded.at("runtimeState").at("playerCurrentDirection").get<std::string>();
            state.core.safariGameOver =
                decoded.at("runtimeState").at("safari").at("gameOver").get<bool>();
            state.core.safariBallCount = static_cast<std::uint8_t>(
                decoded.at("runtimeState").at("safari").at("ballCount").get<int>());
            state.core.safariSteps = static_cast<std::uint16_t>(
                decoded.at("runtimeState").at("safari").at("steps").get<int>());
            state.core.strengthOutsideBattle =
                decoded.at("runtimeState").at("flags").at("strengthOutsideBattle").get<bool>();
            state.core.surfingAllowed =
                decoded.at("runtimeState").at("flags").at("surfingAllowed").get<bool>();
            state.core.flyOutOfBattle =
                decoded.at("runtimeState").at("flags").at("flyOutOfBattle").get<bool>();
            state.core.isBattle =
                decoded.at("runtimeState").at("flags").at("isBattle").get<bool>();
            state.core.isTrainerBattle =
                decoded.at("runtimeState").at("flags").at("isTrainerBattle").get<bool>();
            state.core.countPlaytime =
                decoded.at("runtimeState").at("flags").at("countPlaytime").get<bool>();
            const nlohmann::json& storyEvidence = decoded.at("worldState").at("storyEvidence");
            state.core.gotOldRod = storyEvidence.at("gotOldRod").get<bool>();
            state.core.gotGoodRod = storyEvidence.at("gotGoodRod").get<bool>();
            state.core.gotSuperRod = storyEvidence.at("gotSuperRod").get<bool>();
            state.core.satisfiedSaffronGuards =
                storyEvidence.at("satisfiedSaffronGuards").get<bool>();
            state.core.gotLapras = storyEvidence.at("gotLapras").get<bool>();
            state.core.everHealedPokemon = storyEvidence.at("everHealedPokemon").get<bool>();
            state.core.gotStarter = storyEvidence.at("gotStarter").get<bool>();
            state.core.defeatedLoreleiRoomState =
                storyEvidence.at("defeatedLoreleiRoomState").get<bool>();
            state.core.money =
                static_cast<std::uint32_t>(decoded.at("moneyAndCoins").at("money").at("value").get<int>());
            state.core.coins =
                static_cast<std::uint16_t>(decoded.at("moneyAndCoins").at("coins").at("value").get<int>());
            state.core.playHours = decoded.at("playtime").at("hours").get<int>();
            state.core.playMinutes = decoded.at("playtime").at("minutes").get<int>();
            state.core.playSeconds = decoded.at("playtime").at("seconds").get<int>();

            state.pokedex.owned.reserve(decoded.at("pokedex").at("species").size());
            state.pokedex.seen.reserve(decoded.at("pokedex").at("species").size());
            for (const auto& species : decoded.at("pokedex").at("species")) {
                state.pokedex.owned.push_back(species.at("owned").get<bool>());
                state.pokedex.seen.push_back(species.at("seen").get<bool>());
            }
            state.pokedex.ownedCount =
                static_cast<std::size_t>(decoded.at("pokedex").at("ownedCount").get<int>());
            state.pokedex.seenCount =
                static_cast<std::size_t>(decoded.at("pokedex").at("seenCount").get<int>());

            for (const auto& item : decoded.at("inventory").at("bag").at("items")) {
                state.inventory.bagItems.push_back(InventoryItem{
                    static_cast<std::uint8_t>(item.at("item").at("id").get<int>()),
                    item.at("item").at("name").get<std::string>(),
                    static_cast<std::uint8_t>(item.at("quantity").get<int>())
                });
            }
            for (const auto& item : decoded.at("inventory").at("pcItemStorage").at("items")) {
                state.inventory.pcItems.push_back(InventoryItem{
                    static_cast<std::uint8_t>(item.at("item").at("id").get<int>()),
                    item.at("item").at("name").get<std::string>(),
                    static_cast<std::uint8_t>(item.at("quantity").get<int>())
                });
            }

            state.party.count = decoded.at("party").at("count").get<int>();
            for (const auto& mon : decoded.at("party").at("pokemon")) {
                state.party.pokemon.push_back(ParsePartyPokemon(mon));
            }

            state.storage.selectedBoxNumber =
                decoded.at("currentBoxCache").at("selectedBoxNumber").get<int>();
            state.storage.boxChangedFlag =
                decoded.at("currentBoxCache").at("boxChangedFlag").get<bool>();
            if (decoded.at("currentBoxCache").contains("cache") &&
                !decoded.at("currentBoxCache").at("cache").is_null()) {
                state.storage.hasCurrentBoxCache = true;
                state.storage.currentBoxCache = ParseCurrentBoxCache(
                    decoded.at("currentBoxCache").at("cache"),
                    state.storage.selectedBoxNumber);
            }
            state.storage.inputSynchronizationStatus =
                decoded.at("currentBoxCache").at("synchronizationStatus").get<std::string>();
            state.storage.boxes.reserve(decoded.at("pcStorage").at("boxes").size());
            for (const auto& box : decoded.at("pcStorage").at("boxes")) {
                state.storage.boxes.push_back(ParseStorageBox(box));
            }

            state.daycare.inUse = decoded.at("daycare").at("inUse").get<bool>();
            if (state.daycare.inUse && !decoded.at("daycare").at("pokemon").is_null()) {
                state.daycare.pokemon = ParseStoredPokemon(decoded.at("daycare").at("pokemon"));
                state.daycare.pokemon->position = 1;
            }

            state.hallOfFame.entryCount = decoded.at("hallOfFame").at("entryCount").get<int>();
            for (const auto& entry : decoded.at("hallOfFame").at("entries")) {
                state.hallOfFame.entries.push_back(ParseHallOfFameEntry(entry));
            }

            for (const auto& flag : decoded.at("events").at("flags")) {
                state.events.push_back(ParseNamedFlag(flag, "value"));
            }
            for (const auto& flag : decoded.at("trainerBattles").at("records")) {
                state.trainerBattles.push_back(ParseNamedFlag(flag, "completed"));
            }
            for (const auto& flag : decoded.at("staticBattles").at("records")) {
                state.staticBattles.push_back(ParseNamedFlag(flag, "completed"));
            }
            for (const auto& flag : decoded.at("storyProgress").at("storyFlags")) {
                state.storyProgress.push_back(ParseNamedFlag(flag, "completed"));
            }
            for (const auto& script : decoded.at("scripts").at("scripts")) {
                state.scripts.push_back(ParseScript(script));
            }
            for (const auto& entry : decoded.at("missableObjects")) {
                state.missableObjects.push_back(ParseMissableObject(entry));
            }
            for (const auto& entry : decoded.at("hiddenItems")) {
                state.hiddenItems.push_back(ParseHiddenObject(entry, "collected"));
            }
            for (const auto& entry : decoded.at("hiddenCoins")) {
                state.hiddenCoins.push_back(ParseHiddenObject(entry, "collected"));
            }
            for (const auto& entry : decoded.at("visitedTowns")) {
                state.visitedTowns.push_back(ParseVisitedTown(entry));
            }

            result.ok = true;
            result.state = std::move(state);
        } catch (const std::exception& ex) {
            result.errors.push_back(ex.what());
        }
        return result;
    }

private:
    static std::uint8_t ParseHexByte(const std::string& value) {
        if (value.size() < 3U || value[0] != '0' || (value[1] != 'x' && value[1] != 'X')) {
            throw std::runtime_error("Expected 0x-prefixed hex byte, received: " + value);
        }
        return static_cast<std::uint8_t>(std::stoul(value.substr(2), nullptr, 16));
    }

    static PartyPokemonState ParsePartyPokemon(const nlohmann::json& mon) {
        PartyPokemonState partyMon;
        partyMon.position = mon.at("position").get<int>();
        partyMon.speciesId =
            static_cast<std::uint8_t>(mon.at("species").at("internalId").get<int>());
        partyMon.speciesName = mon.at("species").at("name").get<std::string>();
        partyMon.nationalDexNumber = static_cast<std::uint8_t>(
            mon.at("species").at("nationalDexNumber").get<int>());
        partyMon.nickname = mon.at("nickname").at("value").get<std::string>();
        partyMon.originalTrainerName = mon.at("originalTrainer").at("name").get<std::string>();
        partyMon.originalTrainerId = static_cast<std::uint16_t>(
            mon.at("originalTrainer").at("idNo").get<int>());
        partyMon.level = static_cast<std::uint8_t>(mon.at("level").get<int>());
        partyMon.experience = static_cast<std::uint32_t>(mon.at("experience").get<int>());
        partyMon.statusRaw = ParseHexByte(mon.at("status").at("rawByte").get<std::string>());

        const auto* speciesData = pokemon::FindSpeciesData(partyMon.speciesId);
        if (speciesData == nullptr) {
            throw std::runtime_error(
                "decoded.party contains unsupported species id " +
                std::to_string(partyMon.speciesId) + ".");
        }
        partyMon.type1 = speciesData->type1;
        partyMon.type2 = speciesData->type2;
        partyMon.catchRate = speciesData->catchRate;

        const nlohmann::json& stats = mon.at("stats");
        partyMon.currentHp = static_cast<std::uint16_t>(stats.at("hpCurrent").get<int>());
        partyMon.maxHp = static_cast<std::uint16_t>(stats.at("hpMax").get<int>());
        partyMon.attack = static_cast<std::uint16_t>(stats.at("attack").get<int>());
        partyMon.defense = static_cast<std::uint16_t>(stats.at("defense").get<int>());
        partyMon.speed = static_cast<std::uint16_t>(stats.at("speed").get<int>());
        partyMon.special = static_cast<std::uint16_t>(stats.at("special").get<int>());

        const nlohmann::json& dvs = mon.at("dvs");
        partyMon.dvs.hp = static_cast<std::uint8_t>(dvs.at("hp").get<int>());
        partyMon.dvs.attack = static_cast<std::uint8_t>(dvs.at("attack").get<int>());
        partyMon.dvs.defense = static_cast<std::uint8_t>(dvs.at("defense").get<int>());
        partyMon.dvs.speed = static_cast<std::uint8_t>(dvs.at("speed").get<int>());
        partyMon.dvs.special = static_cast<std::uint8_t>(dvs.at("special").get<int>());

        const nlohmann::json& statExperience = mon.at("statExperience");
        partyMon.statExperience.hp =
            static_cast<std::uint16_t>(statExperience.at("hp").get<int>());
        partyMon.statExperience.attack =
            static_cast<std::uint16_t>(statExperience.at("attack").get<int>());
        partyMon.statExperience.defense =
            static_cast<std::uint16_t>(statExperience.at("defense").get<int>());
        partyMon.statExperience.speed =
            static_cast<std::uint16_t>(statExperience.at("speed").get<int>());
        partyMon.statExperience.special =
            static_cast<std::uint16_t>(statExperience.at("special").get<int>());

        for (const auto& move : mon.at("moves")) {
            partyMon.moves.push_back(ParseMove(move));
        }
        return partyMon;
    }

    static StoredPokemonState ParseStoredPokemon(const nlohmann::json& mon) {
        StoredPokemonState stored;
        stored.position = mon.value("position", 1);
        stored.speciesId =
            static_cast<std::uint8_t>(mon.at("species").at("internalId").get<int>());
        stored.speciesName = mon.at("species").at("name").get<std::string>();
        stored.nationalDexNumber = static_cast<std::uint8_t>(
            mon.at("species").at("nationalDexNumber").get<int>());
        stored.nickname = mon.at("nickname").at("value").get<std::string>();
        stored.originalTrainerName = mon.at("originalTrainer").at("name").get<std::string>();
        stored.originalTrainerId = static_cast<std::uint16_t>(
            mon.at("originalTrainer").at("idNo").get<int>());
        stored.level = static_cast<std::uint8_t>(mon.at("level").get<int>());
        stored.experience = static_cast<std::uint32_t>(mon.at("experience").get<int>());
        stored.statusRaw = ParseHexByte(mon.at("status").at("rawByte").get<std::string>());

        if (const auto* speciesData = pokemon::FindSpeciesData(stored.speciesId);
            speciesData != nullptr) {
            stored.type1 = speciesData->type1;
            stored.type2 = speciesData->type2;
            stored.catchRate = speciesData->catchRate;
        }

        const auto& stats = mon.at("stats");
        stored.currentHp = static_cast<std::uint16_t>(stats.at("hpCurrent").get<int>());

        const auto& dvs = mon.at("dvs");
        stored.dvs.hp = static_cast<std::uint8_t>(dvs.at("hp").get<int>());
        stored.dvs.attack = static_cast<std::uint8_t>(dvs.at("attack").get<int>());
        stored.dvs.defense = static_cast<std::uint8_t>(dvs.at("defense").get<int>());
        stored.dvs.speed = static_cast<std::uint8_t>(dvs.at("speed").get<int>());
        stored.dvs.special = static_cast<std::uint8_t>(dvs.at("special").get<int>());

        const auto& statExperience = mon.at("statExperience");
        stored.statExperience.hp =
            static_cast<std::uint16_t>(statExperience.at("hp").get<int>());
        stored.statExperience.attack =
            static_cast<std::uint16_t>(statExperience.at("attack").get<int>());
        stored.statExperience.defense =
            static_cast<std::uint16_t>(statExperience.at("defense").get<int>());
        stored.statExperience.speed =
            static_cast<std::uint16_t>(statExperience.at("speed").get<int>());
        stored.statExperience.special =
            static_cast<std::uint16_t>(statExperience.at("special").get<int>());

        for (const auto& move : mon.at("moves")) {
            stored.moves.push_back(ParseMove(move));
        }
        return stored;
    }

    static PokemonMoveState ParseMove(const nlohmann::json& move) {
        return PokemonMoveState{
            static_cast<std::uint8_t>(move.at("move").at("id").get<int>()),
            move.at("move").at("name").get<std::string>(),
            static_cast<std::uint8_t>(move.at("pp").at("current").get<int>()),
            static_cast<std::uint8_t>(move.at("pp").at("ppUps").get<int>())
        };
    }

    static StorageBoxState ParseStorageBox(const nlohmann::json& box) {
        StorageBoxState parsed;
        parsed.boxNumber = box.at("boxNumber").get<int>();
        parsed.count = box.at("count").get<int>();
        for (const auto& mon : box.at("pokemon")) {
            parsed.pokemon.push_back(ParseStoredPokemon(mon));
        }
        return parsed;
    }

    static StorageBoxState ParseCurrentBoxCache(const nlohmann::json& cache, int selectedBoxNumber) {
        StorageBoxState parsed;
        parsed.boxNumber = selectedBoxNumber;
        if (cache.contains("boxNumber") && !cache.at("boxNumber").is_null()) {
            parsed.boxNumber = cache.at("boxNumber").get<int>();
        }
        parsed.count = cache.at("count").get<int>();
        for (const auto& mon : cache.at("pokemon")) {
            parsed.pokemon.push_back(ParseStoredPokemon(mon));
        }
        return parsed;
    }

    static HallOfFameEntryState ParseHallOfFameEntry(const nlohmann::json& entry) {
        HallOfFameEntryState parsed;
        parsed.entryNumber = entry.at("entryNumber").get<int>();
        for (const auto& mon : entry.at("pokemon")) {
            HallOfFamePokemonState pokemonState;
            pokemonState.partyOrder = mon.at("partyOrder").get<int>();
            pokemonState.speciesId =
                static_cast<std::uint8_t>(mon.at("species").at("internalId").get<int>());
            pokemonState.speciesName = mon.at("species").at("name").get<std::string>();
            pokemonState.nationalDexNumber = static_cast<std::uint8_t>(
                mon.at("species").at("nationalDexNumber").get<int>());
            pokemonState.level = static_cast<std::uint8_t>(mon.at("level").get<int>());
            pokemonState.nickname = mon.at("nickname").get<std::string>();
            parsed.pokemon.push_back(std::move(pokemonState));
        }
        return parsed;
    }

    static NamedFlagState ParseNamedFlag(const nlohmann::json& flag, const char* boolField) {
        NamedFlagState parsed;
        parsed.flagIndex = flag.at("flagIndex").get<int>();
        parsed.name = flag.at("name").get<std::string>();
        parsed.description = flag.value("description", "");
        parsed.set = flag.at(boolField).get<bool>();
        parsed.category = flag.value("category", "");
        parsed.location = flag.value("location", "");
        parsed.trainerNumber = flag.value("trainerNumber", 0);
        return parsed;
    }

    static ScriptState ParseScript(const nlohmann::json& script) {
        ScriptState parsed;
        parsed.index = script.at("index").get<int>();
        parsed.relativeOffset = script.at("relativeOffset").get<int>();
        parsed.size = script.at("size").get<int>();
        parsed.value = script.at("rawValue").get<int>();
        parsed.mapOrScriptName = script.value("mapOrScriptName", "");
        return parsed;
    }

    static MissableObjectState ParseMissableObject(const nlohmann::json& entry) {
        MissableObjectState parsed;
        parsed.index = entry.at("index").get<int>();
        parsed.name = entry.at("name").get<std::string>();
        parsed.location = entry.value("location", "");
        parsed.toggledOff = entry.at("toggledOff").get<bool>();
        return parsed;
    }

    static HiddenObjectState ParseHiddenObject(const nlohmann::json& entry, const char* boolField) {
        HiddenObjectState parsed;
        parsed.index = entry.at("index").get<int>();
        parsed.name = entry.at("name").get<std::string>();
        parsed.location = entry.value("location", "");
        parsed.x = entry.value("x", 0);
        parsed.y = entry.value("y", 0);
        parsed.collected = entry.at(boolField).get<bool>();
        return parsed;
    }

    static VisitedTownState ParseVisitedTown(const nlohmann::json& entry) {
        VisitedTownState parsed;
        parsed.index = entry.at("index").get<int>();
        parsed.name = entry.at("name").get<std::string>();
        parsed.visited = entry.at("visited").get<bool>();
        return parsed;
    }
};

}  // namespace pkmn::savegen::model
