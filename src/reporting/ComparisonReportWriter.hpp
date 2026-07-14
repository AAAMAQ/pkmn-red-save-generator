#pragma once

#include <filesystem>
#include <fstream>
#include <iomanip>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <string>

#include <nlohmann/json.hpp>

#include "../comparison/PhysicalComparator.hpp"
#include "../integrity/IntegrityValidator.hpp"
#include "../template/Sha256.hpp"

namespace pkmn::savegen::reporting {

class ComparisonReportWriter {
public:
    static nlohmann::json PhysicalJson(
        const comparison::PhysicalComparisonResult& result,
        const std::string& originalSha256,
        const std::string& generatedSha256) {
        nlohmann::json regions = nlohmann::json::array();
        for (const auto& region : result.regions) {
            regions.push_back({
                {"start", region.start},
                {"endInclusive", region.endInclusive},
                {"length", region.endInclusive - region.start + 1U},
                {"classification", region.equal ? "equal" : "different"},
                {"differingBytes", region.differingBytes}
            });
        }
        return {
            {"reportFormatVersion", "1.0.0"},
            {"status", result.byteIdentical() ? "byte-identical" : "physically-different"},
            {"offsetConvention", "zero-based inclusive ranges"},
            {"original", {{"size", result.originalSize}, {"sha256", originalSha256}}},
            {"generated", {{"size", result.generatedSize}, {"sha256", generatedSha256}}},
            {"comparedBytes", result.comparedBytes},
            {"equalBytes", result.equalBytes},
            {"differingBytes", result.differingBytes},
            {"equalPercent", Percentage(result.equalBytes, result.comparedBytes)},
            {"differingPercent", Percentage(result.differingBytes, result.comparedBytes)},
            {"firstDifference", OptionalOffset(result.firstDifference)},
            {"lastDifference", OptionalOffset(result.lastDifference)},
            {"regions", std::move(regions)}
        };
    }

    static std::string PhysicalMarkdown(
        const comparison::PhysicalComparisonResult& result,
        const std::string& originalSha256,
        const std::string& generatedSha256,
        std::string_view title = "Physical Comparison") {
        std::ostringstream out;
        out << "# " << title << "\n\n"
            << "- Status: **"
            << (result.byteIdentical() ? "byte-identical" : "physically different") << "**\n"
            << "- Original: " << result.originalSize << " bytes, `" << originalSha256 << "`\n"
            << "- Compared: " << result.generatedSize << " bytes, `" << generatedSha256 << "`\n"
            << "- Equal bytes: " << result.equalBytes << " ("
            << std::fixed << std::setprecision(4)
            << Percentage(result.equalBytes, result.comparedBytes) << "%)\n"
            << "- Differing bytes: " << result.differingBytes << " ("
            << Percentage(result.differingBytes, result.comparedBytes) << "%)\n";
        if (result.firstDifference != std::numeric_limits<std::size_t>::max()) {
            out << "- First/last difference: `0x" << std::hex << result.firstDifference
                << "` / `0x" << result.lastDifference << std::dec << "`\n";
        }
        out << "\n## Contiguous Regions\n\n"
            << "Offsets are zero-based and inclusive.\n\n"
            << "| Start | End | Length | Result | Differing bytes |\n"
            << "|---:|---:|---:|---|---:|\n";
        for (const auto& region : result.regions) {
            out << "| `0x" << std::hex << region.start << "` | `0x"
                << region.endInclusive << "` | " << std::dec
                << (region.endInclusive - region.start + 1U) << " | "
                << (region.equal ? "equal" : "different") << " | "
                << region.differingBytes << " |\n";
        }
        return out.str();
    }

    static nlohmann::json IntegrityJson(
        const integrity::IntegrityValidationResult& value) {
        nlohmann::json boxChecksums = nlohmann::json::array();
        nlohmann::json boxStructures = nlohmann::json::array();
        for (std::size_t i = 0; i < 12U; ++i) {
            boxChecksums.push_back(value.boxChecksumsValid[i]);
            boxStructures.push_back(value.boxStructuresValid[i]);
        }
        return {
            {"ok", value.ok},
            {"sizeOk", value.sizeOk},
            {"mainChecksumValid", value.mainChecksumValid},
            {"bank2ChecksumValid", value.bank2ChecksumValid},
            {"bank3ChecksumValid", value.bank3ChecksumValid},
            {"boxChecksumsValid", std::move(boxChecksums)},
            {"boxStructuresValid", std::move(boxStructures)},
            {"selectedBoxValid", value.selectedBoxValid},
            {"currentBoxCacheValid", value.currentBoxCacheValid},
            {"currentBoxCacheMatchesSelectedPermanent",
             value.currentBoxCacheMatchesSelectedPermanent},
            {"warnings", value.warnings},
            {"errors", value.errors}
        };
    }

    static void WriteJson(const std::filesystem::path& path,
                          const nlohmann::json& document) {
        EnsureParent(path);
        std::ofstream out(path);
        if (!out) throw std::runtime_error("failed to open report path: " + path.string());
        out << document.dump(2) << "\n";
        if (!out) throw std::runtime_error("failed to write report path: " + path.string());
    }

    static void WriteMarkdown(const std::filesystem::path& path,
                              const std::string& document) {
        EnsureParent(path);
        std::ofstream out(path);
        if (!out) throw std::runtime_error("failed to open report path: " + path.string());
        out << document;
        if (!out) throw std::runtime_error("failed to write report path: " + path.string());
    }

private:
    static double Percentage(std::size_t part, std::size_t total) {
        return total == 0U ? 100.0 : (100.0 * static_cast<double>(part) /
                                      static_cast<double>(total));
    }

    static nlohmann::json OptionalOffset(std::size_t value) {
        if (value == std::numeric_limits<std::size_t>::max()) return nullptr;
        return value;
    }

    static void EnsureParent(const std::filesystem::path& path) {
        if (path.has_parent_path()) {
            std::filesystem::create_directories(path.parent_path());
        }
    }
};

}  // namespace pkmn::savegen::reporting
