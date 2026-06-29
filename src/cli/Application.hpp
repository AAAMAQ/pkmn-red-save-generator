#pragma once

#include <filesystem>
#include <iostream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

#include "../comparison/SemanticComparator.hpp"
#include "../generation/MinimalSaveGenerator.hpp"
#include "../generation/MinimalStateContract.hpp"
#include "../generation/RedSaveInitializer.hpp"
#include "../input/PhysicalImageIsolationGuard.hpp"
#include "../input/RedJsonReader.hpp"
#include "../input/RedJsonValidator.hpp"
#include "../model/RedSemanticState.hpp"
#include "../template/CanonicalTemplateLoader.hpp"
#include "../template/TemplateProfile.hpp"
#include "../template/TemplateValidator.hpp"

namespace pkmn::savegen::cli {

class Application {
public:
    static int Run(int argc, const char* argv[]) {
        try {
            if (argc < 2) {
                PrintUsage();
                return 1;
            }

            const std::string command = argv[1];
            const std::vector<std::string> args(argv + 2, argv + argc);
            if (command == "validate-input") {
                return RunValidateInput(args);
            }
            if (command == "validate-template") {
                return RunValidateTemplate(args);
            }
            if (command == "show-profile") {
                return RunShowProfile(args);
            }
            if (command == "generate") {
                return RunGenerate(args);
            }
            if (command == "compare-semantics") {
                return RunCompareSemantics(args);
            }

            std::cerr << "Unknown command: " << command << "\n";
            PrintUsage();
            return 1;
        } catch (const std::exception& ex) {
            std::cerr << "error: " << ex.what() << "\n";
            return 1;
        }
    }

private:
    static void PrintUsage() {
        std::cout
            << "pkmn-red-save-generator\n"
            << "  validate-input --input <path>\n"
            << "  validate-template --template <path> --profile <path>\n"
            << "  show-profile --profile <path>\n"
            << "  generate --input <target.red.json> --template <dummy.sav> --profile <profile.json> --output <generated.sav> --report <generated.generation-report.json>\n"
            << "  compare-semantics --target-json <target.red.json> --reparsed-json <savegenie-output.red.json>\n";
    }

    static std::string GetRequiredFlag(const std::vector<std::string>& args, std::string_view name) {
        for (std::size_t i = 0; i + 1 < args.size(); ++i) {
            if (args[i] == name) {
                return args[i + 1];
            }
        }
        throw std::runtime_error("missing required flag: " + std::string(name));
    }

    static int RunValidateInput(const std::vector<std::string>& args) {
        const std::filesystem::path inputPath = GetRequiredFlag(args, "--input");
        const input::ParsedRedJson parsed = input::RedJsonReader::ReadFromFile(inputPath);
        const input::RedJsonValidationResult validation = input::RedJsonValidator::Validate(parsed.document);
        const input::SanitizedRedJson sanitized = input::PhysicalImageIsolationGuard::Sanitize(parsed.document);
        const model::BuildSemanticStateResult semantic = model::RedSemanticStateBuilder::Build(
            sanitized.document, sanitized.physicalImageRemoved);

        std::cout << "Input: " << inputPath.string() << "\n";
        std::cout << "Schema version: " << validation.schemaVersion << "\n";
        std::cout << "Target game: " << validation.targetGame << " (" << validation.regionAssumption << ")\n";
        std::cout << "physicalImage present: " << (validation.physicalImagePresent ? "yes" : "no") << "\n";
        std::cout << "physicalImage ignored: " << (sanitized.physicalImageRemoved ? "yes" : "no") << "\n";
        for (const std::string& warning : validation.warnings) {
            std::cout << "warning: " << warning << "\n";
        }
        if (!semantic.ok) {
            for (const std::string& error : semantic.errors) {
                std::cerr << "error: " << error << "\n";
            }
            return 1;
        }
        std::cout << "Semantic identity: " << semantic.state.identity.playerName
                  << " / " << semantic.state.identity.rivalName << "\n";
        std::cout << "Bag items: " << semantic.state.inventory.bagItems.size()
                  << ", PC items: " << semantic.state.inventory.pcItems.size() << "\n";
        return validation.ok ? 0 : 1;
    }

    static int RunValidateTemplate(const std::vector<std::string>& args) {
        const std::filesystem::path templatePath = GetRequiredFlag(args, "--template");
        const std::filesystem::path profilePath = GetRequiredFlag(args, "--profile");

        const template_profile::TemplateProfile profile =
            template_profile::TemplateProfileLoader::LoadFromFile(profilePath);
        const template_loader::LoadedTemplate loaded =
            template_loader::CanonicalTemplateLoader::Load(templatePath);
        const template_validation::TemplateValidationResult result =
            template_validation::TemplateValidator::Validate(profile, loaded);
        const generation::WorkingSaveBuffer working =
            generation::RedSaveInitializer::Initialize(loaded, profile, true);

        std::cout << "Profile: " << profile.profileId << "\n";
        std::cout << "Template: " << templatePath.string() << "\n";
        std::cout << "Template SHA-256: " << loaded.sha256 << "\n";
        std::cout << "Accepted: " << (result.accepted ? "yes" : "no") << "\n";
        std::cout << "Main checksum valid: " << (result.baseline.mainChecksumValid ? "yes" : "no") << "\n";
        std::cout << "Bank 2 all checksum valid: " << (result.baseline.bank2AllChecksumValid ? "yes" : "no") << "\n";
        std::cout << "Bank 3 all checksum valid: " << (result.baseline.bank3AllChecksumValid ? "yes" : "no") << "\n";
        std::cout << "Current box cache count: " << static_cast<int>(result.baseline.currentBoxCacheCount) << "\n";
        std::cout << "Selected box number: " << result.baseline.selectedBoxNumber << "\n";
        std::cout << "Initializer copied bytes: " << working.bytes.size() << "\n";
        for (const std::string& warning : result.warnings) {
            std::cout << "warning: " << warning << "\n";
        }
        for (const std::string& error : result.errors) {
            std::cerr << "error: " << error << "\n";
        }
        return result.accepted ? 0 : 1;
    }

    static int RunShowProfile(const std::vector<std::string>& args) {
        const std::filesystem::path profilePath = GetRequiredFlag(args, "--profile");
        const template_profile::TemplateProfile profile =
            template_profile::TemplateProfileLoader::LoadFromFile(profilePath);
        std::cout << "Profile ID: " << profile.profileId << "\n";
        std::cout << "Target: " << profile.targetGame << " (" << profile.regionAssumption << ")\n";
        std::cout << "Template: " << profile.templateRelativePath.string() << "\n";
        std::cout << "Template SHA-256: " << profile.templateSha256 << "\n";
        std::cout << "Supported schema versions:";
        for (const std::string& version : profile.supportedSchemaVersions) {
            std::cout << " " << version;
        }
        std::cout << "\n";
        return 0;
    }

    static int RunGenerate(const std::vector<std::string>& args) {
        generation::GenerateRequest request;
        request.inputJsonPath = GetRequiredFlag(args, "--input");
        request.templateSavePath = GetRequiredFlag(args, "--template");
        request.profilePath = GetRequiredFlag(args, "--profile");
        request.outputSavePath = GetRequiredFlag(args, "--output");
        request.outputReportPath = GetRequiredFlag(args, "--report");

        const generation::GenerateResult result = generation::MinimalSaveGenerator::Generate(request);
        std::cout << "Generated save: " << request.outputSavePath.string() << "\n";
        std::cout << "Generation report: " << request.outputReportPath.string() << "\n";
        std::cout << "Output SHA-256: " << result.report.outputSha256 << "\n";
        std::cout << "Dummy box policy: " << result.report.dummyBoxPolicy << "\n";
        std::cout << "Main checksum valid: " << (result.integrity.mainChecksumValid ? "yes" : "no") << "\n";
        std::cout << "Bank 2 all checksum valid: " << (result.integrity.bank2ChecksumValid ? "yes" : "no") << "\n";
        std::cout << "Bank 3 all checksum valid: " << (result.integrity.bank3ChecksumValid ? "yes" : "no") << "\n";
        std::cout << "Bank storage unchanged from template: "
                  << (result.integrity.bankStorageUnchanged ? "yes" : "no") << "\n";
        for (const std::string& warning : result.report.warnings) {
            std::cout << "warning: " << warning << "\n";
        }
        return 0;
    }

    static int RunCompareSemantics(const std::vector<std::string>& args) {
        const std::filesystem::path targetPath = GetRequiredFlag(args, "--target-json");
        const std::filesystem::path reparsedPath = GetRequiredFlag(args, "--reparsed-json");

        const auto targetParsed = input::RedJsonReader::ReadFromFile(targetPath);
        const auto targetSanitized = input::PhysicalImageIsolationGuard::Sanitize(targetParsed.document);
        const auto targetSemantic =
            model::RedSemanticStateBuilder::Build(targetSanitized.document, targetSanitized.physicalImageRemoved);
        if (!targetSemantic.ok) {
            std::cerr << "error: failed to build semantic state from target JSON\n";
            return 1;
        }

        const auto contract = generation::MinimalStateContractBuilder::Build(targetSemantic.state);
        const auto actualParsed = input::RedJsonReader::ReadFromFile(reparsedPath);
        const auto actualSanitized = input::PhysicalImageIsolationGuard::Sanitize(actualParsed.document);
        const auto actualSemantic =
            model::RedSemanticStateBuilder::Build(actualSanitized.document, actualSanitized.physicalImageRemoved);
        if (!actualSemantic.ok) {
            std::cerr << "error: failed to build semantic state from reparsed JSON\n";
            return 1;
        }

        comparison::ComparisonOptions options;
        options.compareEventSubset = false;
        const auto differences = comparison::SemanticComparator::CompareOwnedFields(
            contract.expectedSemantic, actualSemantic.state, options);
        if (differences.empty()) {
            std::cout << "Milestone 2 semantic comparison: PASS\n";
            return 0;
        }

        std::cout << "Milestone 2 semantic comparison: FAIL\n";
        for (const auto& difference : differences) {
            std::cout << difference.fieldPath << ": expected=" << difference.expectedValue
                      << " actual=" << difference.actualValue << "\n";
        }
        return 1;
    }
};

inline int Run(int argc, const char* argv[]) {
    return Application::Run(argc, argv);
}

}  // namespace pkmn::savegen::cli
