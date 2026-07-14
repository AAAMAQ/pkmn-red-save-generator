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
#include "ExtendedWorldSerializer.hpp"
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
    bool locationWasCanonicalized = false;
    std::vector<std::string> warnings;
};

class MinimalStateContractBuilder {
public:
    static MinimalStateContract Build(const model::RedSemanticState& target) {
        MinimalStateContract contract;
        contract.expectedSemantic = target;
        contract.supportedLocation = ResolveSupportedLocation(target);

        if (target.core.mapId != contract.supportedLocation.mapId ||
            target.core.x != contract.supportedLocation.x ||
            target.core.y != contract.supportedLocation.y ||
            target.core.previousMapId != contract.supportedLocation.previousMap ||
            target.core.xBlockCoord != contract.supportedLocation.xBlock ||
            target.core.yBlockCoord != contract.supportedLocation.yBlock) {
            std::ostringstream oss;
            oss << "Source location map="
                << static_cast<int>(target.core.mapId)
                << " x=" << static_cast<int>(target.core.x)
                << " y=" << static_cast<int>(target.core.y)
                << " previousMap=" << static_cast<int>(target.core.previousMapId)
                << " xBlock=" << static_cast<int>(target.core.xBlockCoord)
                << " yBlock=" << static_cast<int>(target.core.yBlockCoord)
                << " is outside the emulator-validated safe-location profile. Generated output will canonicalize the start location to "
                << contract.supportedLocation.label << ".";
            contract.warnings.push_back(oss.str());
            contract.locationWasCanonicalized = true;
            contract.expectedSemantic.core.mapId = contract.supportedLocation.mapId;
            contract.expectedSemantic.core.x = contract.supportedLocation.x;
            contract.expectedSemantic.core.y = contract.supportedLocation.y;
            contract.expectedSemantic.core.previousMapId = contract.supportedLocation.previousMap;
            contract.expectedSemantic.core.xBlockCoord = contract.supportedLocation.xBlock;
            contract.expectedSemantic.core.yBlockCoord = contract.supportedLocation.yBlock;
        }

        PartyValidator::ValidateOrThrow(target.party);
        StorageValidator::ValidateOrThrow(target.storage);
        if (HasDirtyCurrentBoxCacheDisagreement(target.storage)) {
            contract.warnings.push_back(
                "Input current-box cache differs from the selected permanent box. This is a valid Gen I working-box state: generation preserves the Bank 1 cache independently, and the game commits it to permanent storage when the player changes boxes.");
        }
        StorageValidator::ValidateDaycareOrThrow(target.daycare);
        HallOfFameSerializer::ValidateOrThrow(target.hallOfFame);
        ExtendedWorldSerializer::ValidateOrThrow(target);

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
            "Unsupported source locations are canonicalized to the emulator-validated Red's house baseline rather than guessed. Broader location preservation remains paused after the Milestone 5-6 load-time corruption incident.");
        contract.warnings.push_back(
            "The generator currently owns trainer/core fields, party, permanent storage, selected-box cache, badges, Pokedex, bag inventory, PC item inventory, Daycare, Hall of Fame, named event/story/trainer/static flags, scripts, missables, hidden objects, visited towns, and the Red's-house world-evidence subset.");
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

        return kBaselineLocation;
    }

    static bool HasDirtyCurrentBoxCacheDisagreement(const model::StorageState& storage) {
        if (!storage.hasCurrentBoxCache ||
            storage.selectedBoxNumber < 1 ||
            storage.selectedBoxNumber > static_cast<int>(storage.boxes.size())) {
            return false;
        }

        const auto& selected =
            storage.boxes[static_cast<std::size_t>(storage.selectedBoxNumber - 1)];
        return storage.currentBoxCache != selected;
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
