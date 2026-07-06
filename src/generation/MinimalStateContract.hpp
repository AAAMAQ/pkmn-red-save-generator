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
#include "PartyValidator.hpp"

namespace pkmn::savegen::generation {

struct SupportedLocation {
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
    std::string dummyBoxPolicy = "Policy A - preserve canonical unused state";
    std::vector<std::string> warnings;
};

class MinimalStateContractBuilder {
public:
    static MinimalStateContract Build(const model::RedSemanticState& target) {
        MinimalStateContract contract;
        contract.expectedSemantic = target;

        if (target.core.mapId != contract.supportedLocation.mapId ||
            target.core.x != contract.supportedLocation.x ||
            target.core.y != contract.supportedLocation.y) {
            std::ostringstream oss;
            oss << "Milestone 4 currently supports only the proven baseline location map="
                << static_cast<int>(contract.supportedLocation.mapId)
                << " x=" << static_cast<int>(contract.supportedLocation.x)
                << " y=" << static_cast<int>(contract.supportedLocation.y)
                << "; received map=" << static_cast<int>(target.core.mapId)
                << " x=" << static_cast<int>(target.core.x)
                << " y=" << static_cast<int>(target.core.y) << ".";
            throw std::runtime_error(oss.str());
        }

        PartyValidator::ValidateOrThrow(target.party);
        if (target.daycare.inUse) {
            throw std::runtime_error(
                "Milestone 4 does not yet support an occupied daycare state.");
        }
        if (target.hallOfFame.entryCount != 0) {
            throw std::runtime_error(
                "Milestone 4 does not yet support non-empty Hall of Fame reconstruction.");
        }

        RequireBitfieldLength(
            target.pokedex.owned, 151, "decoded.pokedex.species owned");
        RequireBitfieldLength(
            target.pokedex.seen, 151, "decoded.pokedex.species seen");
        RequireBitfieldLength(
            target.eventSubset.visitedTowns,
            static_cast<std::size_t>(encoding::Gen1Layout::VisitedTownsUsedBits),
            "decoded.visitedTowns");
        RequireBitfieldLength(
            target.eventSubset.hiddenItems,
            static_cast<std::size_t>(encoding::Gen1Layout::HiddenItemsUsedBits),
            "decoded.hiddenItems");
        RequireBitfieldLength(
            target.eventSubset.hiddenCoins,
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
            "Milestone 4 preserves the committed dummy's permanent box banks and invalid bank all-box checksums as canonical unused state.");
        contract.warnings.push_back(
            "Milestone 4 still supports only the proven baseline safe location from the canonical dummy template.");
        contract.warnings.push_back(
            "Milestone 4 now owns full party serialization in addition to trainer/core fields, badges, Pokedex, bag inventory, PC item inventory, and the conservative event subset.");
        return contract;
    }

private:
    static void RequireBitfieldLength(const std::vector<bool>& bits,
                                      std::size_t expected,
                                      const std::string& label) {
        if (bits.size() != expected) {
            std::ostringstream oss;
            oss << label << " must contain exactly " << expected
                << " entries for Milestone 4; received " << bits.size() << ".";
            throw std::runtime_error(oss.str());
        }
    }

    static std::size_t CountTrue(const std::vector<bool>& bits) {
        return static_cast<std::size_t>(std::count(bits.begin(), bits.end(), true));
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
