#pragma once

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <stdexcept>
#include <sstream>
#include <string>
#include <vector>

#include "../input/PhysicalImageIsolationGuard.hpp"
#include "../input/RedJsonReader.hpp"
#include "../input/RedJsonValidator.hpp"
#include "../integrity/ChecksumAlgorithms.hpp"
#include "../integrity/IntegrityValidator.hpp"
#include "../model/RedSemanticState.hpp"
#include "../template/CanonicalTemplateLoader.hpp"
#include "../template/Sha256.hpp"
#include "../template/TemplateProfile.hpp"
#include "../template/TemplateValidator.hpp"
#include "CoreStateSerializer.hpp"
#include "DaycareSerializer.hpp"
#include "ExtendedWorldSerializer.hpp"
#include "HallOfFameSerializer.hpp"
#include "MinimalStateContract.hpp"
#include "PartySerializer.hpp"
#include "RedSaveInitializer.hpp"
#include "StorageSerializer.hpp"

namespace pkmn::savegen::generation {

struct GenerateRequest {
    std::filesystem::path inputJsonPath;
    std::filesystem::path templateSavePath;
    std::filesystem::path profilePath;
    std::filesystem::path outputSavePath;
    std::filesystem::path outputReportPath;
    bool dryRun = false;
};

struct GenerateResult {
    reporting::GenerationReport report;
    integrity::IntegrityValidationResult integrity;
    model::RedSemanticState expectedSemantic;
    std::vector<std::uint8_t> outputBytes;
};

class MinimalSaveGenerator {
public:
    static GenerateResult Generate(const GenerateRequest& request) {
        const auto parsed = input::RedJsonReader::ReadFromFile(request.inputJsonPath);
        const auto validation = input::RedJsonValidator::Validate(parsed.document);
        if (!validation.ok) {
            throw std::runtime_error("Input JSON failed validation.");
        }

        const auto sanitized = input::PhysicalImageIsolationGuard::Sanitize(parsed.document);
        const auto semantic = model::RedSemanticStateBuilder::Build(
            sanitized.document, sanitized.physicalImageRemoved);
        if (!semantic.ok) {
            throw std::runtime_error("Failed to build semantic state from target JSON.");
        }

        const auto profile = template_profile::TemplateProfileLoader::LoadFromFile(request.profilePath);
        const auto loadedTemplate = template_loader::CanonicalTemplateLoader::Load(request.templateSavePath);
        const auto templateValidation = template_validation::TemplateValidator::Validate(profile, loadedTemplate);
        if (!templateValidation.accepted) {
            throw std::runtime_error("Template failed validation against the active profile.");
        }

        const MinimalStateContract contract = MinimalStateContractBuilder::Build(semantic.state);
        ValidateOutputPaths(request, loadedTemplate.path, parsed.inputPath);

        WorkingSaveBuffer working =
            RedSaveInitializer::Initialize(loadedTemplate, profile, sanitized.physicalImageRemoved);
        working.report.generatorVersion = "milestone6-dev";
        working.report.targetJsonPath = request.inputJsonPath.lexically_normal().string();
        working.report.targetSourceSha256 = semantic.state.sourceSha256;
        working.report.dummyBoxPolicy = "Retired in Milestone 5 - permanent storage is generator-owned";
        working.report.warnings.insert(
            working.report.warnings.end(), templateValidation.warnings.begin(), templateValidation.warnings.end());
        working.report.warnings.insert(
            working.report.warnings.end(), contract.warnings.begin(), contract.warnings.end());

        CoreStateSerializer::ApplyMinimalState(contract, working);
        PartySerializer::WriteParty(working, contract.expectedSemantic.party);
        StorageSerializer::WriteStorage(working, contract.expectedSemantic.storage);
        DaycareSerializer::WriteDaycare(working, contract.expectedSemantic.daycare);
        HallOfFameSerializer::WriteHallOfFame(working, contract.expectedSemantic.hallOfFame);
        ExtendedWorldSerializer::WritePersistentWorldState(working, contract.expectedSemantic);
        working.bytes[encoding::Gen1Layout::MainChecksumOff] =
            integrity::ChecksumAlgorithms::ComputeMainChecksum(working.bytes);
        working.report.ranges.push_back({
            encoding::Gen1Layout::MainChecksumOff,
            encoding::Gen1Layout::MainChecksumOff,
            "regenerated-checksum",
            "Main checksum regenerated after semantic writes",
            "MinimalSaveGenerator",
            "integrity.mainChecksum"
        });
        working.report.fieldsWritten.push_back("integrity.mainChecksum");

        const auto integrity =
            integrity::IntegrityValidator::ValidateGeneratedSave(working.bytes);
        if (!integrity.ok) {
            std::string message = "Generation integrity validation failed.";
            for (const std::string& error : integrity.errors) {
                message += " ";
                message += error;
            }
            throw std::runtime_error(message);
        }

        if (!request.dryRun) {
            WriteFile(request.outputSavePath, working.bytes);
        }
        working.report.outputPath = request.dryRun
            ? ("dry-run:" + request.outputSavePath.lexically_normal().string())
            : request.outputSavePath.lexically_normal().string();
        working.report.outputSize = working.bytes.size();
        working.report.outputSha256 = template_support::Sha256::Hex(working.bytes);
        AttachByteProvenanceAndValidateRanges(
            working.report, loadedTemplate.bytes, working.bytes);
        if (!request.outputReportPath.empty()) {
            WriteReport(request.outputReportPath, working.report);
        }

        GenerateResult result;
        result.report = working.report;
        result.integrity = integrity;
        result.expectedSemantic = contract.expectedSemantic;
        result.outputBytes = std::move(working.bytes);
        return result;
    }

private:
    static void ValidateOutputPaths(const GenerateRequest& request,
                                    const std::filesystem::path& templatePath,
                                    const std::filesystem::path& inputPath) {
        const auto normalizedInput = std::filesystem::absolute(inputPath).lexically_normal();
        const auto normalizedTemplate = std::filesystem::absolute(templatePath).lexically_normal();
        const auto normalizedOutput = std::filesystem::absolute(request.outputSavePath).lexically_normal();
        const auto normalizedReport = std::filesystem::absolute(request.outputReportPath).lexically_normal();

        if (request.outputSavePath.empty()) {
            if (!request.dryRun) {
                throw std::runtime_error("Output save path is required unless --dry-run is used.");
            }
        } else if (normalizedOutput == normalizedInput) {
            throw std::runtime_error("Output save path must not collide with the input JSON path.");
        }
        if (!request.outputSavePath.empty() && normalizedOutput == normalizedTemplate) {
            throw std::runtime_error("Output save path must not collide with the canonical template path.");
        }
        if (!request.outputReportPath.empty() && normalizedReport == normalizedTemplate) {
            throw std::runtime_error("Output report path must not collide with the canonical template path.");
        }
        if (!request.outputReportPath.empty() && !request.outputSavePath.empty() && normalizedReport == normalizedOutput) {
            throw std::runtime_error("Output report path must be different from the output save path.");
        }
        if (!request.dryRun && !request.outputSavePath.empty() && std::filesystem::exists(request.outputSavePath)) {
            throw std::runtime_error("Output save path already exists; refusing to overwrite.");
        }
        if (!request.outputReportPath.empty() && std::filesystem::exists(request.outputReportPath)) {
            throw std::runtime_error("Output report path already exists; refusing to overwrite.");
        }
    }

    static void WriteFile(const std::filesystem::path& path, const std::vector<std::uint8_t>& bytes) {
        if (path.has_parent_path()) {
            std::filesystem::create_directories(path.parent_path());
        }
        std::ofstream output(path, std::ios::binary);
        if (!output) {
            throw std::runtime_error("Failed to open output save path for writing: " + path.string());
        }
        output.write(reinterpret_cast<const char*>(bytes.data()), static_cast<std::streamsize>(bytes.size()));
        if (!output) {
            throw std::runtime_error("Failed to write output save bytes.");
        }
    }

    static void WriteReport(const std::filesystem::path& path, const reporting::GenerationReport& report) {
        if (path.has_parent_path()) {
            std::filesystem::create_directories(path.parent_path());
        }
        std::ofstream output(path);
        if (!output) {
            throw std::runtime_error("Failed to open generation report path for writing: " + path.string());
        }
        output << report.ToJson().dump(2) << "\n";
        if (!output) {
            throw std::runtime_error("Failed to write generation report JSON.");
        }
    }

    static void AttachByteProvenanceAndValidateRanges(
        reporting::GenerationReport& report,
        const std::vector<std::uint8_t>& templateBytes,
        const std::vector<std::uint8_t>& outputBytes) {
        constexpr std::size_t kExactByteLimit = 32;
        report.overlappingWrites = FindUndeclaredOverlaps(report.ranges);
        if (!report.overlappingWrites.empty()) {
            std::ostringstream oss;
            oss << "Generation report contains undeclared overlapping write ranges:";
            for (const auto& overlap : report.overlappingWrites) {
                oss << " [0x" << std::hex << overlap.start
                    << "-0x" << overlap.endInclusive << std::dec
                    << " between '" << overlap.firstReason
                    << "' and '" << overlap.secondReason << "']";
            }
            throw std::runtime_error(oss.str());
        }

        for (auto& range : report.ranges) {
            if (range.classification == "template-inherited") {
                continue;
            }
            if (range.start > range.endInclusive ||
                range.endInclusive >= templateBytes.size() ||
                range.endInclusive >= outputBytes.size()) {
                throw std::runtime_error(
                    "Generation report contains an out-of-bounds range for " + range.reason + ".");
            }

            const std::size_t length = range.endInclusive - range.start + 1U;
            if (length <= kExactByteLimit) {
                range.previousBytesHex = HexBytes(templateBytes, range.start, length);
                range.newBytesHex = HexBytes(outputBytes, range.start, length);
            } else {
                range.byteSampleTruncated = true;
                range.previousSha256 = RangeSha256(templateBytes, range.start, length);
                range.newSha256 = RangeSha256(outputBytes, range.start, length);
                range.previousBytesHex = HexBytes(templateBytes, range.start, kExactByteLimit);
                range.newBytesHex = HexBytes(outputBytes, range.start, kExactByteLimit);
            }
        }
    }

    static std::vector<reporting::RangeOverlapEntry> FindUndeclaredOverlaps(
        const std::vector<reporting::RangeLedgerEntry>& ranges) {
        struct IndexedRange {
            std::size_t index = 0;
            const reporting::RangeLedgerEntry* range = nullptr;
        };

        std::vector<IndexedRange> written;
        for (std::size_t i = 0; i < ranges.size(); ++i) {
            if (ranges[i].classification == "template-inherited") {
                continue;
            }
            written.push_back({i, &ranges[i]});
        }
        std::sort(written.begin(), written.end(), [](const auto& lhs, const auto& rhs) {
            if (lhs.range->start != rhs.range->start) {
                return lhs.range->start < rhs.range->start;
            }
            return lhs.range->endInclusive < rhs.range->endInclusive;
        });

        std::vector<reporting::RangeOverlapEntry> overlaps;
        for (std::size_t i = 0; i < written.size(); ++i) {
            for (std::size_t j = i + 1; j < written.size(); ++j) {
                if (written[j].range->start > written[i].range->endInclusive) {
                    break;
                }
                const std::size_t start =
                    std::max(written[i].range->start, written[j].range->start);
                const std::size_t end =
                    std::min(written[i].range->endInclusive, written[j].range->endInclusive);
                if (start <= end) {
                    overlaps.push_back({
                        written[i].index,
                        written[j].index,
                        start,
                        end,
                        written[i].range->reason,
                        written[j].range->reason
                    });
                }
            }
        }
        return overlaps;
    }

    static std::string HexBytes(const std::vector<std::uint8_t>& bytes,
                                std::size_t offset,
                                std::size_t length) {
        std::ostringstream oss;
        oss << std::hex << std::uppercase << std::setfill('0');
        for (std::size_t i = 0; i < length; ++i) {
            oss << std::setw(2) << static_cast<int>(bytes[offset + i]);
        }
        return oss.str();
    }

    static std::string RangeSha256(const std::vector<std::uint8_t>& bytes,
                                   std::size_t offset,
                                   std::size_t length) {
        return template_support::Sha256::Hex(
            std::vector<std::uint8_t>(bytes.begin() + static_cast<std::ptrdiff_t>(offset),
                                      bytes.begin() + static_cast<std::ptrdiff_t>(offset + length)));
    }
};

}  // namespace pkmn::savegen::generation
