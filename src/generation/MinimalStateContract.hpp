#pragma once

#include <algorithm>
#include <cstdint>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "../model/RedSemanticState.hpp"

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
            oss << "Milestone 2 currently supports only the proven baseline location map="
                << static_cast<int>(contract.supportedLocation.mapId)
                << " x=" << static_cast<int>(contract.supportedLocation.x)
                << " y=" << static_cast<int>(contract.supportedLocation.y)
                << "; received map=" << static_cast<int>(target.core.mapId)
                << " x=" << static_cast<int>(target.core.x)
                << " y=" << static_cast<int>(target.core.y) << ".";
            throw std::runtime_error(oss.str());
        }

        contract.expectedSemantic.core.badgesBitfield = 0;
        contract.expectedSemantic.pokedex.owned.assign(
            std::max<std::size_t>(target.pokedex.owned.size(), 151), false);
        contract.expectedSemantic.pokedex.seen.assign(
            std::max<std::size_t>(target.pokedex.seen.size(), 151), false);
        contract.expectedSemantic.pokedex.ownedCount = 0;
        contract.expectedSemantic.pokedex.seenCount = 0;
        contract.expectedSemantic.inventory.bagItems.clear();
        contract.expectedSemantic.inventory.pcItems.clear();
        contract.expectedSemantic.party.count = 0;
        contract.expectedSemantic.daycare.inUse = false;
        contract.expectedSemantic.hallOfFame.entryCount = 0;

        contract.warnings.push_back(
            "Milestone 2 preserves the committed dummy's permanent box banks and invalid bank all-box checksums as canonical unused state.");
        contract.warnings.push_back(
            "Milestone 2 supports only the proven baseline safe location from the canonical dummy template.");
        return contract;
    }
};

}  // namespace pkmn::savegen::generation
