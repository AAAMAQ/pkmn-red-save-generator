#pragma once

#include <algorithm>
#include <array>
#include <cstdint>
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

struct PartyMoveState {
    std::uint8_t moveId = 0;
    std::string moveName;
    std::uint8_t ppCurrent = 0;
    std::uint8_t ppUps = 0;

    bool operator==(const PartyMoveState&) const = default;
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
    std::vector<PartyMoveState> moves;
    PokemonStatExperienceState statExperience;
    PokemonDVState dvs;

    bool operator==(const PartyPokemonState&) const = default;
};

struct PartyState {
    int count = 0;
    std::vector<PartyPokemonState> pokemon;

    bool operator==(const PartyState&) const = default;
};

struct DaycareState {
    bool inUse = false;

    bool operator==(const DaycareState&) const = default;
};

struct HallOfFameState {
    int entryCount = 0;

    bool operator==(const HallOfFameState&) const = default;
};

struct EventSubsetState {
    std::vector<bool> visitedTowns;
    std::vector<bool> hiddenItems;
    std::vector<bool> hiddenCoins;

    bool operator==(const EventSubsetState&) const = default;
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
    DaycareState daycare;
    HallOfFameState hallOfFame;
    EventSubsetState eventSubset;

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

        auto parse_hex_byte = [](const std::string& value) -> std::uint8_t {
            return static_cast<std::uint8_t>(std::stoul(value, nullptr, 16));
        };

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
                parse_hex_byte(decoded.at("badges").at("rawBitfield").get<std::string>().substr(2));
            state.core.mapId =
                static_cast<std::uint8_t>(decoded.at("location").at("map").at("id").get<int>());
            state.core.x =
                static_cast<std::uint8_t>(decoded.at("location").at("x").at("value").get<int>());
            state.core.y =
                static_cast<std::uint8_t>(decoded.at("location").at("y").at("value").get<int>());
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
                PartyPokemonState partyMon;
                partyMon.position = mon.at("position").get<int>();
                partyMon.speciesId =
                    static_cast<std::uint8_t>(mon.at("species").at("internalId").get<int>());
                partyMon.speciesName = mon.at("species").at("name").get<std::string>();
                partyMon.nationalDexNumber = static_cast<std::uint8_t>(
                    mon.at("species").at("nationalDexNumber").get<int>());
                partyMon.nickname = mon.at("nickname").at("value").get<std::string>();
                partyMon.originalTrainerName =
                    mon.at("originalTrainer").at("name").get<std::string>();
                partyMon.originalTrainerId = static_cast<std::uint16_t>(
                    mon.at("originalTrainer").at("idNo").get<int>());
                partyMon.level = static_cast<std::uint8_t>(mon.at("level").get<int>());
                partyMon.experience =
                    static_cast<std::uint32_t>(mon.at("experience").get<int>());
                partyMon.statusRaw =
                    parse_hex_byte(mon.at("status").at("rawByte").get<std::string>().substr(2));

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
                partyMon.statExperience.hp = static_cast<std::uint16_t>(
                    statExperience.at("hp").get<int>());
                partyMon.statExperience.attack = static_cast<std::uint16_t>(
                    statExperience.at("attack").get<int>());
                partyMon.statExperience.defense = static_cast<std::uint16_t>(
                    statExperience.at("defense").get<int>());
                partyMon.statExperience.speed = static_cast<std::uint16_t>(
                    statExperience.at("speed").get<int>());
                partyMon.statExperience.special = static_cast<std::uint16_t>(
                    statExperience.at("special").get<int>());

                for (const auto& move : mon.at("moves")) {
                    partyMon.moves.push_back(PartyMoveState{
                        static_cast<std::uint8_t>(move.at("move").at("id").get<int>()),
                        move.at("move").at("name").get<std::string>(),
                        static_cast<std::uint8_t>(move.at("pp").at("current").get<int>()),
                        static_cast<std::uint8_t>(move.at("pp").at("ppUps").get<int>())
                    });
                }

                state.party.pokemon.push_back(std::move(partyMon));
            }
            state.daycare.inUse = decoded.at("daycare").at("inUse").get<bool>();
            state.hallOfFame.entryCount = decoded.at("hallOfFame").at("entryCount").get<int>();

            state.eventSubset.visitedTowns.reserve(decoded.at("visitedTowns").size());
            for (const auto& entry : decoded.at("visitedTowns")) {
                state.eventSubset.visitedTowns.push_back(entry.at("visited").get<bool>());
            }
            state.eventSubset.hiddenItems.reserve(decoded.at("hiddenItems").size());
            for (const auto& entry : decoded.at("hiddenItems")) {
                state.eventSubset.hiddenItems.push_back(entry.at("collected").get<bool>());
            }
            state.eventSubset.hiddenCoins.reserve(decoded.at("hiddenCoins").size());
            for (const auto& entry : decoded.at("hiddenCoins")) {
                state.eventSubset.hiddenCoins.push_back(entry.at("collected").get<bool>());
            }

            result.ok = true;
            result.state = std::move(state);
        } catch (const std::exception& ex) {
            result.errors.push_back(ex.what());
        }
        return result;
    }
};

}  // namespace pkmn::savegen::model
