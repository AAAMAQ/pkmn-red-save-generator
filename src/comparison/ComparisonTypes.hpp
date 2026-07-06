#pragma once

#include <string>

namespace pkmn::savegen::comparison {

enum class DifferenceCategory {
    RequiredExactMismatch,
    DerivedMismatch,
    PermittedCanonicalDifference
};

struct Difference {
    DifferenceCategory category = DifferenceCategory::RequiredExactMismatch;
    std::string fieldPath;
    std::string expectedValue;
    std::string actualValue;
};

}  // namespace pkmn::savegen::comparison
