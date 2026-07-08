#pragma once

#include <algorithm>
#include <cstdint>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "../encoding/Gen1Layout.hpp"
#include "../model/RedSemanticState.hpp"
#include "HallOfFameSerializer.hpp"
#include "PartyValidator.hpp"
#include "StorageValidator.hpp"

namespace pkmn::savegen::generation {

struct SupportedLocation {
    const char* label = "Red's house second floor";
    std::uint8_t mapId = 38;
    std::uint8_t x = 3;
    std::uint8_t y = 6;
    std::uint8_t yBlock = 0;
    std::uint8_t xBlock = 1;
    std::uint8_t previousMap = 0;
};

struct MinimalStateContract {
    model::RedSemanticState expectedSemantic;
    SupportedLocation supportedLocation;
    bool locationStrictBaseline = true;
    std::vector<std::string> warnings;
};

class MinimalStateContractBuilder {
public:
    static MinimalStateContract Build(const model::RedSemanticState& target) {
        MinimalStateContract contract;
        contract.expectedSemantic = target;
        contract.supportedLocation = ResolveSupportedLocation(target);
        contract.locationStrictBaseline = (contract.supportedLocation.mapId == 38);

        if (target.core.mapId != contract.supportedLocation.mapId ||
            target.core.x != contract.supportedLocation.x ||
            target.core.y != contract.supportedLocation.y ||
            target.core.previousMapId != contract.supportedLocation.previousMap ||
            target.core.xBlockCoord != contract.supportedLocation.xBlock ||
            target.core.yBlockCoord != contract.supportedLocation.yBlock) {
            std::ostringstream oss;
            oss << "The current implemented location validator supports only explicitly verified safe map states; received map="
                << static_cast<int>(target.core.mapId)
                << " x=" << static_cast<int>(target.core.x)
                << " y=" << static_cast<int>(target.core.y)
                << " previousMap=" << static_cast<int>(target.core.previousMapId)
                << " xBlock=" << static_cast<int>(target.core.xBlockCoord)
                << " yBlock=" << static_cast<int>(target.core.yBlockCoord) << ".";
            throw std::runtime_error(oss.str());
        }

        PartyValidator::ValidateOrThrow(target.party);
        StorageValidator::ValidateOrThrow(target.storage);
        StorageValidator::ValidateDaycareOrThrow(target.daycare);
        HallOfFameSerializer::ValidateOrThrow(target.hallOfFame);

        RequireLength(
            target.pokedex.owned, 151, "decoded.pokedex.species owned");
        RequireLength(
            target.pokedex.seen, 151, "decoded.pokedex.species seen");
        RequireLength(
            target.visitedTowns,
            static_cast<std::size_t>(encoding::Gen1Layout::VisitedTownsUsedBits),
            "decoded.visitedTowns");
        RequireLength(
            target.hiddenItems,
            static_cast<std::size_t>(encoding::Gen1Layout::HiddenItemsUsedBits),
            "decoded.hiddenItems");
        RequireLength(
            target.hiddenCoins,
            static_cast<std::size_t>(encoding::Gen1Layout::HiddenCoinsUsedBits),
            "decoded.hiddenCoins");
        ValidateInventoryList(
            target.inventory.bagItems,
            static_cast<std::size_t>(encoding::Gen1Layout::BagItemsMaxPairs),
            "decoded.inventory.bag.items");
        ValidateInventoryList(
            target.inventory.pcItems,
            static_cast<std::size_t>(encoding::Gen1Layout::PCItemBoxMaxPairs),
            "decoded.inventory.pcItemStorage.items");

        contract.expectedSemantic.pokedex.ownedCount = CountTrue(contract.expectedSemantic.pokedex.owned);
        contract.expectedSemantic.pokedex.seenCount = CountTrue(contract.expectedSemantic.pokedex.seen);

        contract.warnings.push_back(
            "Permanent PC storage is now generator-owned; all 12 boxes, the selected-box cache, per-box checksums, and bank all-box checksums must be regenerated from semantic input.");
        contract.warnings.push_back(
            "The current location validator fails closed to the emulator-validated Red's house baseline. Non-baseline locations are paused after the Milestone 5-6 load-time corruption incident.");
        contract.warnings.push_back(
            "The generator currently owns trainer/core fields, party, permanent storage, selected-box cache, badges, Pokedex, bag inventory, PC item inventory, Daycare, Hall of Fame, and the conservative named event/world subset.");
        return contract;
    }

private:
    template <typename T>
    static void RequireLength(const std::vector<T>& values,
                              std::size_t expected,
                              const std::string& label) {
        if (values.size() != expected) {
            std::ostringstream oss;
            oss << label << " must contain exactly " << expected
                << " entries for the current generator stage; received " << values.size() << ".";
            throw std::runtime_error(oss.str());
        }
    }

    static std::size_t CountTrue(const std::vector<bool>& bits) {
        return static_cast<std::size_t>(std::count(bits.begin(), bits.end(), true));
    }

    static SupportedLocation ResolveSupportedLocation(const model::RedSemanticState& target) {
        static constexpr SupportedLocation kBaselineLocation{
            "Red's house second floor", 38, 3, 6, 0, 1, 0
        };

        if (target.core.mapId == kBaselineLocation.mapId &&
            target.core.x == kBaselineLocation.x &&
            target.core.y == kBaselineLocation.y &&
            target.core.previousMapId == kBaselineLocation.previousMap &&
            target.core.xBlockCoord == kBaselineLocation.xBlock &&
            target.core.yBlockCoord == kBaselineLocation.yBlock) {
            return kBaselineLocation;
        }

        std::ostringstream oss;
        oss << "No emulator-validated safe-location profile exists for map="
            << static_cast<int>(target.core.mapId)
            << " x=" << static_cast<int>(target.core.x)
            << " y=" << static_cast<int>(target.core.y)
            << " previousMap=" << static_cast<int>(target.core.previousMapId)
            << " xBlock=" << static_cast<int>(target.core.xBlockCoord)
            << " yBlock=" << static_cast<int>(target.core.yBlockCoord)
            << ". Only the Red's house second-floor baseline is enabled until the full map-runtime cluster is serialized and emulator-proven.";
        throw std::runtime_error(oss.str());
    }

    static void ValidateInventoryList(const std::vector<model::InventoryItem>& items,
                                      std::size_t capacity,
                                      const std::string& label) {
        if (items.size() > capacity) {
            std::ostringstream oss;
            oss << label << " exceeds capacity " << capacity
                << " with " << items.size() << " entries.";
            throw std::runtime_error(oss.str());
        }

        std::set<std::uint8_t> seenIds;
        for (std::size_t i = 0; i < items.size(); ++i) {
            const auto& item = items[i];
            if (item.id == 0x00 || item.id == 0xFF) {
                std::ostringstream oss;
                oss << label << "[" << i << "] contains unsupported item id "
                    << static_cast<int>(item.id) << ".";
                throw std::runtime_error(oss.str());
            }
            if (item.quantity == 0 || item.quantity > 99) {
                std::ostringstream oss;
                oss << label << "[" << i << "] quantity must be in 1..99; received "
                    << static_cast<int>(item.quantity) << ".";
                throw std::runtime_error(oss.str());
            }
            if (!seenIds.insert(item.id).second) {
                std::ostringstream oss;
                oss << label << " contains duplicate item id "
                    << static_cast<int>(item.id) << ".";
                throw std::runtime_error(oss.str());
            }
        }
    }
};

}  // namespace pkmn::savegen::generation
