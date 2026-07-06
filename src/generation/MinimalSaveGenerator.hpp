#pragma once

#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <string>

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
#include "MinimalStateContract.hpp"
#include "PartySerializer.hpp"
#include "RedSaveInitializer.hpp"

namespace pkmn::savegen::generation {

struct GenerateRequest {
    std::filesystem::path inputJsonPath;
    std::filesystem::path templateSavePath;
    std::filesystem::path profilePath;
    std::filesystem::path outputSavePath;
    std::filesystem::path outputReportPath;
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
        working.report.generatorVersion = "milestone4-dev";
        working.report.targetJsonPath = request.inputJsonPath.lexically_normal().string();
        working.report.targetSourceSha256 = semantic.state.sourceSha256;
        working.report.dummyBoxPolicy = contract.dummyBoxPolicy;
        working.report.warnings.insert(
            working.report.warnings.end(), templateValidation.warnings.begin(), templateValidation.warnings.end());
        working.report.warnings.insert(
            working.report.warnings.end(), contract.warnings.begin(), contract.warnings.end());

        CoreStateSerializer::ApplyMinimalState(contract, working);
        PartySerializer::WriteParty(working, contract.expectedSemantic.party);
        working.bytes[encoding::Gen1Layout::MainChecksumOff] =
            integrity::ChecksumAlgorithms::ComputeMainChecksum(working.bytes);
        working.report.ranges.push_back({
            encoding::Gen1Layout::MainChecksumOff,
            encoding::Gen1Layout::MainChecksumOff,
            "regenerated-checksum",
            "Main checksum regenerated after semantic writes"
        });
        working.report.fieldsWritten.push_back("integrity.mainChecksum");

        const auto integrity =
            integrity::IntegrityValidator::ValidateMilestone2(working.bytes, loadedTemplate.bytes, false);
        if (!integrity.ok) {
            std::string message = "Generation integrity validation failed.";
            for (const std::string& error : integrity.errors) {
                message += " ";
                message += error;
            }
            throw std::runtime_error(message);
        }

        WriteFile(request.outputSavePath, working.bytes);
        working.report.outputPath = request.outputSavePath.lexically_normal().string();
        working.report.outputSize = working.bytes.size();
        working.report.outputSha256 = template_support::Sha256::Hex(working.bytes);
        WriteReport(request.outputReportPath, working.report);

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

        if (normalizedOutput == normalizedInput) {
            throw std::runtime_error("Output save path must not collide with the input JSON path.");
        }
        if (normalizedOutput == normalizedTemplate) {
            throw std::runtime_error("Output save path must not collide with the canonical template path.");
        }
        if (normalizedReport == normalizedTemplate) {
            throw std::runtime_error("Output report path must not collide with the canonical template path.");
        }
        if (normalizedReport == normalizedOutput) {
            throw std::runtime_error("Output report path must be different from the output save path.");
        }
        if (std::filesystem::exists(request.outputSavePath)) {
            throw std::runtime_error("Output save path already exists; refusing to overwrite.");
        }
        if (std::filesystem::exists(request.outputReportPath)) {
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
};

}  // namespace pkmn::savegen::generation
