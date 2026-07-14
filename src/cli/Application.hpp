#pragma once

#include <algorithm>
#include <array>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

#include "../comparison/SemanticComparator.hpp"
#include "../comparison/PhysicalComparator.hpp"
#include "../encoding/Gen1Layout.hpp"
#include "../encoding/PrimitiveWriter.hpp"
#include "../generation/MinimalSaveGenerator.hpp"
#include "../generation/MinimalStateContract.hpp"
#include "../generation/RedSaveInitializer.hpp"
#include "../generation/HallOfFameSerializer.hpp"
#include "../generation/StorageValidator.hpp"
#include "../input/PhysicalImageIsolationGuard.hpp"
#include "../input/RedJsonReader.hpp"
#include "../input/RedJsonValidator.hpp"
#include "../integrity/ChecksumAlgorithms.hpp"
#include "../integrity/IntegrityValidator.hpp"
#include "../model/RedSemanticState.hpp"
#include "../reporting/ComparisonReportWriter.hpp"
#include "../template/CanonicalTemplateLoader.hpp"
#include "../template/Sha256.hpp"
#include "../template/TemplateProfile.hpp"
#include "../template/TemplateValidator.hpp"
#include "../Version.hpp"

namespace pkmn::savegen::cli {

class Application {
public:
    static int Run(int argc, const char* argv[]) {
        try {
            if (argc < 2) {
                PrintUsage();
                return 0;
            }

            const std::string command = argv[1];
            const std::vector<std::string> args(argv + 2, argv + argc);
            if (command == "--help" || command == "-h" || command == "help") {
                PrintUsage();
                return 0;
            }
            if (command == "--version" || command == "-V" || command == "version") {
                PrintVersion();
                return 0;
            }
            if (command == "validate-input" || command == "validate") {
                return RunValidateInput(args);
            }
            if (command == "validate-template") {
                return RunValidateTemplate(args);
            }
            if (command == "validate-save") {
                return RunValidateSave(args);
            }
            if (command == "show-profile") {
                return RunShowProfile(args);
            }
            if (command == "inspect") {
                return RunInspect(args);
            }
            if (command == "generate") {
                return RunGenerate(args);
            }
            if (command == "check-determinism") {
                return RunCheckDeterminism(args);
            }
            if (command == "check-physical-image-isolation") {
                return RunCheckPhysicalImageIsolation(args);
            }
            if (command == "compare-semantics") {
                return RunCompareSemantics(args);
            }
            if (command == "validate-text") {
                return RunValidateText(args);
            }
            if (command == "validate-boxes") {
                return RunValidateBoxes(args);
            }
            if (command == "validate-pokemon-operability") {
                return RunValidatePokemonOperability(args);
            }
            if (command == "validate-hall-of-fame") {
                return RunValidateHallOfFame(args);
            }
            if (command == "inspect-box") {
                return RunInspectBox(args);
            }
            if (command == "inspect-pokemon") {
                return RunInspectPokemon(args);
            }
            if (command == "compare-physical") {
                return RunComparePhysical(args);
            }
            if (command == "analyze-post-emulator") {
                return RunAnalyzePostEmulator(args);
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
            << "  --help\n"
            << "  --version\n"
            << "  generate <input.red.json> <output.sav> [--report <report.json>] [--dry-run] [--summary] [--show-ranges]\n"
            << "  generate --input <target.red.json> --template <dummy.sav> --profile <profile.json> --output <generated.sav> --report <report.json> [--dry-run]\n"
            << "  validate-input|validate --input <path>\n"
            << "  validate-template --template <path> --profile <path>\n"
            << "  validate-save --input-save <path>\n"
            << "  inspect --input <path>\n"
            << "  validate-text --text <Gen-I-text-or-lossless-tokens>\n"
            << "  validate-boxes --input <path>\n"
            << "  validate-pokemon-operability --input <path>\n"
            << "  validate-hall-of-fame --input <path>\n"
            << "  inspect-box --input <path> --box <1..12> [--current-cache]\n"
            << "  inspect-pokemon --input <path> --box <1..12> --slot <1..20> [--current-cache]\n"
            << "  compare-physical --original <save> --generated <save> [--report <json>] [--markdown <md>]\n"
            << "  analyze-post-emulator --before <save> --after <save> [--report <json>] [--markdown <md>]\n"
            << "  show-profile --profile <path>\n"
            << "  check-determinism --input <path> --template <dummy.sav> --profile <profile.json> --work-dir <dir>\n"
            << "  check-physical-image-isolation --input <path> --template <dummy.sav> --profile <profile.json> --work-dir <dir>\n"
            << "  compare-semantics --target-json <target.red.json> --reparsed-json <savegenie-output.red.json> [--report <json>] [--markdown <md>]\n";
    }

    static void PrintVersion() {
        std::cout << kGeneratorName << " " << kGeneratorVersion << "\n";
    }

    static std::string GetRequiredFlag(const std::vector<std::string>& args, std::string_view name) {
        for (std::size_t i = 0; i + 1 < args.size(); ++i) {
            if (args[i] == name) {
                return args[i + 1];
            }
        }
        throw std::runtime_error("missing required flag: " + std::string(name));
    }

    static std::optional<std::string> GetOptionalFlag(
        const std::vector<std::string>& args,
        std::string_view name) {
        for (std::size_t i = 0; i < args.size(); ++i) {
            if (args[i] == name) {
                if (i + 1 >= args.size() || StartsWithDash(args[i + 1])) {
                    throw std::runtime_error("missing value for flag: " + std::string(name));
                }
                return args[i + 1];
            }
        }
        return std::nullopt;
    }

    static bool HasFlag(const std::vector<std::string>& args, std::string_view name) {
        return std::find(args.begin(), args.end(), name) != args.end();
    }

    static std::vector<std::string> PositionalArgs(const std::vector<std::string>& args) {
        std::vector<std::string> positional;
        for (std::size_t i = 0; i < args.size(); ++i) {
            if (StartsWithDash(args[i])) {
                if (FlagTakesValue(args[i]) && i + 1 < args.size()) {
                    ++i;
                }
                continue;
            }
            positional.push_back(args[i]);
        }
        return positional;
    }

    static bool StartsWithDash(const std::string& text) {
        return !text.empty() && text[0] == '-';
    }

    static bool FlagTakesValue(const std::string& flag) {
        return flag != "--dry-run" &&
               flag != "--summary" &&
               flag != "--show-ranges" &&
               flag != "--allow-dirty-current-box" &&
               flag != "--current-cache" &&
               flag != "--json";
    }

    static std::filesystem::path DefaultProfilePath() {
        return "profiles/pokemon-red-usa-europe-v1.json";
    }

    static std::filesystem::path DefaultTemplatePath() {
        return "Dummy Save/Pokemon - Red Version (USA, Europe) (SGB Enhanced).sav";
    }

    static std::filesystem::path DefaultReportPath(const std::filesystem::path& outputPath) {
        if (outputPath.empty()) {
            return {};
        }
        std::filesystem::path report = outputPath;
        report += ".generation-report.json";
        return report;
    }

    static std::filesystem::path InputPathFromArgs(const std::vector<std::string>& args) {
        if (const auto flagged = GetOptionalFlag(args, "--input")) {
            return *flagged;
        }
        const auto positional = PositionalArgs(args);
        if (!positional.empty()) {
            return positional[0];
        }
        throw std::runtime_error("missing input path");
    }

    static std::filesystem::path SavePathFromArgs(const std::vector<std::string>& args) {
        if (const auto flagged = GetOptionalFlag(args, "--input-save")) {
            return *flagged;
        }
        if (const auto flagged = GetOptionalFlag(args, "--save")) {
            return *flagged;
        }
        const auto positional = PositionalArgs(args);
        if (!positional.empty()) {
            return positional[0];
        }
        throw std::runtime_error("missing save path");
    }

    static std::vector<std::uint8_t> ReadBinaryFile(const std::filesystem::path& path) {
        std::ifstream input(path, std::ios::binary);
        if (!input) {
            throw std::runtime_error("failed to open binary file: " + path.string());
        }
        return std::vector<std::uint8_t>(
            std::istreambuf_iterator<char>(input),
            std::istreambuf_iterator<char>());
    }

    static void WriteJsonFile(const std::filesystem::path& path, const nlohmann::json& document) {
        if (path.has_parent_path()) {
            std::filesystem::create_directories(path.parent_path());
        }
        std::ofstream output(path);
        if (!output) {
            throw std::runtime_error("failed to open JSON output path: " + path.string());
        }
        output << document.dump(2) << "\n";
        if (!output) {
            throw std::runtime_error("failed to write JSON output path: " + path.string());
        }
    }

    static void PrintSemanticSummary(const model::RedSemanticState& semantic) {
        std::cout << "Semantic identity: " << semantic.identity.playerName
                  << " / " << semantic.identity.rivalName
                  << " trainerId=" << semantic.identity.trainerId << "\n";
        std::cout << "Location: map=" << static_cast<int>(semantic.core.mapId)
                  << " x=" << static_cast<int>(semantic.core.x)
                  << " y=" << static_cast<int>(semantic.core.y)
                  << " previousMap=" << static_cast<int>(semantic.core.previousMapId) << "\n";
        std::cout << "Party count: " << semantic.party.count << "\n";
        std::cout << "Storage box counts:";
        for (const auto& box : semantic.storage.boxes) {
            std::cout << " " << box.pokemon.size();
        }
        std::cout << "\n";
        std::cout << "Selected box: " << semantic.storage.selectedBoxNumber << "\n";
        if (semantic.storage.selectedBoxNumber >= 1 &&
            semantic.storage.selectedBoxNumber <= 12) {
            std::cout << "Selected permanent box count: "
                      << semantic.storage.boxes[
                             static_cast<std::size_t>(semantic.storage.selectedBoxNumber - 1)]
                             .pokemon.size()
                      << "\n";
        }
        std::cout << "Current working-box cache count: "
                  << semantic.storage.currentBoxCache.pokemon.size() << "\n";
        if (semantic.storage.selectedBoxNumber >= 1 &&
            semantic.storage.selectedBoxNumber <= 12) {
            std::cout << "Cache matches selected permanent box: "
                      << (semantic.storage.currentBoxCache ==
                                  semantic.storage.boxes[static_cast<std::size_t>(
                                      semantic.storage.selectedBoxNumber - 1)]
                              ? "yes"
                              : "no")
                      << "\n";
        }
        std::cout << "Bag items: " << semantic.inventory.bagItems.size()
                  << ", PC items: " << semantic.inventory.pcItems.size() << "\n";
        std::cout << "Pokedex owned/seen: " << semantic.pokedex.ownedCount
                  << "/" << semantic.pokedex.seenCount << "\n";
        std::cout << "Daycare in use: " << (semantic.daycare.inUse ? "yes" : "no") << "\n";
        std::cout << "Hall of Fame entries: " << semantic.hallOfFame.entries.size() << "\n";
        std::size_t hallOfFamePokemon = 0;
        for (const auto& entry : semantic.hallOfFame.entries) {
            hallOfFamePokemon += entry.pokemon.size();
        }
        std::cout << "Hall of Fame Pokemon: " << hallOfFamePokemon << "\n";
        std::cout << "Events: " << semantic.events.size()
                  << ", trainer battles: " << semantic.trainerBattles.size()
                  << ", static battles: " << semantic.staticBattles.size()
                  << ", story flags: " << semantic.storyProgress.size() << "\n";
        std::cout << "Scripts: " << semantic.scripts.size()
                  << ", missables: " << semantic.missableObjects.size()
                  << ", hidden items: " << semantic.hiddenItems.size()
                  << ", hidden coins: " << semantic.hiddenCoins.size()
                  << ", visited towns: " << semantic.visitedTowns.size() << "\n";
    }

    static int RunValidateInput(const std::vector<std::string>& args) {
        const std::filesystem::path inputPath = InputPathFromArgs(args);
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
        std::cout << "Party count: " << semantic.state.party.count << "\n";
        return validation.ok ? 0 : 1;
    }

    static int RunInspect(const std::vector<std::string>& args) {
        const std::filesystem::path inputPath = InputPathFromArgs(args);
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
        if (!semantic.ok) {
            for (const std::string& error : semantic.errors) {
                std::cerr << "error: " << error << "\n";
            }
            return 1;
        }
        PrintSemanticSummary(semantic.state);
        try {
            const auto contract = generation::MinimalStateContractBuilder::Build(semantic.state);
            std::cout << "Generation contract: supported\n";
            std::cout << "Safe location profile: " << contract.supportedLocation.label << "\n";
            std::cout << "Source location canonicalized: "
                      << (contract.locationWasCanonicalized ? "yes" : "no") << "\n";
            for (const std::string& warning : contract.warnings) {
                std::cout << "warning: " << warning << "\n";
            }
        } catch (const std::exception& ex) {
            std::cout << "Generation contract: rejected\n";
            std::cout << "reason: " << ex.what() << "\n";
            return 1;
        }
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

    static int RunValidateSave(const std::vector<std::string>& args) {
        const std::filesystem::path savePath = SavePathFromArgs(args);
        const std::vector<std::uint8_t> bytes = ReadBinaryFile(savePath);
        const bool sizeOk = bytes.size() == encoding::Gen1Layout::ExpectedSaveSize;
        std::cout << "Save: " << savePath.string() << "\n";
        std::cout << "Size: " << bytes.size() << " bytes\n";
        std::cout << "SHA-256: " << template_support::Sha256::Hex(bytes) << "\n";
        if (!sizeOk) {
            std::cerr << "error: save size is not 32768 bytes\n";
            return 1;
        }

        const auto integrity = integrity::IntegrityValidator::ValidateGeneratedSave(bytes);

        const std::uint8_t rawCurrentBox =
            encoding::PrimitiveWriter::ReadU8(bytes, encoding::Gen1Layout::CurrentBoxByteOff);
        const bool hasChangedBoxesBefore = (rawCurrentBox & 0x80U) != 0;
        const int selectedBox = static_cast<int>(rawCurrentBox & 0x7FU) + 1;

        std::cout << "Main checksum valid: " << (integrity.mainChecksumValid ? "yes" : "no") << "\n";
        std::cout << "Bank 2 all checksum valid: " << (integrity.bank2ChecksumValid ? "yes" : "no") << "\n";
        std::cout << "Bank 3 all checksum valid: " << (integrity.bank3ChecksumValid ? "yes" : "no") << "\n";
        std::cout << "Per-box checksums:";
        for (int box = 1; box <= 12; ++box) {
            std::cout << " box" << box << "="
                      << (integrity.boxChecksumsValid[
                              static_cast<std::size_t>(box - 1)] ? "valid" : "invalid");
        }
        std::cout << "\n";
        std::cout << "Per-box structures:";
        for (int box = 1; box <= 12; ++box) {
            std::cout << " box" << box << "="
                      << (integrity.boxStructuresValid[
                              static_cast<std::size_t>(box - 1)] ? "valid" : "invalid");
        }
        std::cout << "\n";
        std::cout << "Selected box: " << selectedBox
                  << " raw=0x" << std::hex << static_cast<int>(rawCurrentBox) << std::dec
                  << " hasChangedBoxesBefore=" << (hasChangedBoxesBefore ? "yes" : "no") << "\n";
        std::cout << "Current-box cache structure valid: "
                  << (integrity.currentBoxCacheValid ? "yes" : "no") << "\n";
        std::cout << "Current-box cache matches permanent selected box: "
                  << (integrity.currentBoxCacheMatchesSelectedPermanent ? "yes" : "no") << "\n";
        if (!integrity.currentBoxCacheMatchesSelectedPermanent && integrity.currentBoxCacheValid) {
            std::cout << "note: divergence is valid Gen I current working-box state; both representations were validated independently.\n";
        }
        if (HasFlag(args, "--allow-dirty-current-box")) {
            std::cout << "warning: --allow-dirty-current-box is deprecated; valid cache divergence no longer requires an opt-in.\n";
        }
        for (const auto& warning : integrity.warnings) {
            std::cout << "warning: " << warning << "\n";
        }
        for (const auto& error : integrity.errors) {
            std::cerr << "error: " << error << "\n";
        }
        if (!integrity.ok) {
            std::cerr << "error: save validation failed\n";
        }
        return integrity.ok ? 0 : 1;
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
        const auto positional = PositionalArgs(args);
        request.inputJsonPath = GetOptionalFlag(args, "--input").value_or(
            positional.empty() ? "" : positional[0]);
        if (request.inputJsonPath.empty()) {
            throw std::runtime_error("missing input JSON path");
        }
        if (const auto output = GetOptionalFlag(args, "--output")) {
            request.outputSavePath = *output;
        } else if (positional.size() >= 2) {
            request.outputSavePath = positional[1];
        }
        request.templateSavePath = GetOptionalFlag(args, "--template").value_or(
            DefaultTemplatePath().string());
        request.profilePath = GetOptionalFlag(args, "--profile").value_or(
            DefaultProfilePath().string());
        request.dryRun = HasFlag(args, "--dry-run");
        if (request.outputSavePath.empty() && request.dryRun) {
            request.outputSavePath = "dry-run.generated.sav";
        }
        request.outputReportPath = GetOptionalFlag(args, "--report").value_or(
            request.dryRun ? std::string{} : DefaultReportPath(request.outputSavePath).string());

        const generation::GenerateResult result = generation::MinimalSaveGenerator::Generate(request);
        std::cout << (request.dryRun ? "Dry-run generated save bytes for: " : "Generated save: ")
                  << request.outputSavePath.string() << "\n";
        if (!request.outputReportPath.empty()) {
            std::cout << "Generation report: " << request.outputReportPath.string() << "\n";
        }
        std::cout << "Output SHA-256: " << result.report.outputSha256 << "\n";
        std::cout << "Output size: " << result.report.outputSize << "\n";
        std::cout << "Dummy box policy: " << result.report.dummyBoxPolicy << "\n";
        std::cout << "Main checksum valid: " << (result.integrity.mainChecksumValid ? "yes" : "no") << "\n";
        std::cout << "Bank 2 all checksum valid: " << (result.integrity.bank2ChecksumValid ? "yes" : "no") << "\n";
        std::cout << "Bank 3 all checksum valid: " << (result.integrity.bank3ChecksumValid ? "yes" : "no") << "\n";
        std::cout << "Current-box cache structurally valid: "
                  << (result.integrity.currentBoxCacheValid ? "yes" : "no") << "\n";
        std::cout << "Current-box cache matches permanent selected box: "
                  << (result.integrity.currentBoxCacheMatchesSelectedPermanent ? "yes" : "no") << "\n";
        for (const std::string& warning : result.report.warnings) {
            std::cout << "warning: " << warning << "\n";
        }
        if (HasFlag(args, "--summary")) {
            PrintSemanticSummary(result.expectedSemantic);
            std::cout << "Write ranges: " << result.report.ranges.size() << "\n";
            std::cout << "Overlapping writes: " << result.report.overlappingWrites.size() << "\n";
        }
        if (HasFlag(args, "--show-ranges")) {
            for (const auto& range : result.report.ranges) {
                std::cout << "range 0x" << std::hex << range.start
                          << "-0x" << range.endInclusive << std::dec
                          << " [" << range.classification << "] "
                          << range.serializerName << " " << range.semanticPath
                          << " :: " << range.reason << "\n";
            }
        }
        return 0;
    }

    static int RunCheckDeterminism(const std::vector<std::string>& args) {
        const std::filesystem::path workDir = GetRequiredFlag(args, "--work-dir");
        std::filesystem::create_directories(workDir);

        generation::GenerateRequest first;
        first.inputJsonPath = InputPathFromArgs(args);
        first.templateSavePath = GetOptionalFlag(args, "--template").value_or(
            DefaultTemplatePath().string());
        first.profilePath = GetOptionalFlag(args, "--profile").value_or(
            DefaultProfilePath().string());
        first.outputSavePath = workDir / "determinism-a.sav";
        first.outputReportPath = workDir / "determinism-a.report.json";

        generation::GenerateRequest second = first;
        second.outputSavePath = workDir / "determinism-b.sav";
        second.outputReportPath = workDir / "determinism-b.report.json";

        generation::GenerateRequest third = first;
        third.outputSavePath = workDir / "determinism-c.sav";
        third.outputReportPath = workDir / "determinism-c.report.json";

        const auto a = generation::MinimalSaveGenerator::Generate(first);
        const auto b = generation::MinimalSaveGenerator::Generate(second);
        const auto c = generation::MinimalSaveGenerator::Generate(third);
        const bool same = a.outputBytes == b.outputBytes && a.outputBytes == c.outputBytes;
        std::cout << "First SHA-256: " << a.report.outputSha256 << "\n";
        std::cout << "Second SHA-256: " << b.report.outputSha256 << "\n";
        std::cout << "Third SHA-256: " << c.report.outputSha256 << "\n";
        std::cout << "Deterministic output: " << (same ? "yes" : "no") << "\n";
        return same ? 0 : 1;
    }

    static int RunCheckPhysicalImageIsolation(const std::vector<std::string>& args) {
        const std::filesystem::path workDir = GetRequiredFlag(args, "--work-dir");
        std::filesystem::create_directories(workDir);
        const std::filesystem::path inputPath = InputPathFromArgs(args);
        const auto parsed = input::RedJsonReader::ReadFromFile(inputPath);

        auto noPhysical = parsed.document;
        noPhysical.erase("physicalImage");
        if (noPhysical.contains("decoded")) {
            noPhysical["decoded"].erase("physicalImage");
        }
        auto replacedPhysical = parsed.document;
        replacedPhysical["physicalImage"] = "this physical image must not affect generation";
        if (replacedPhysical.contains("decoded")) {
            replacedPhysical["decoded"]["physicalImage"] = "also ignored";
        }

        const std::filesystem::path noPhysicalPath = workDir / "physical-image-removed.red.json";
        const std::filesystem::path replacedPath = workDir / "physical-image-replaced.red.json";
        WriteJsonFile(noPhysicalPath, noPhysical);
        WriteJsonFile(replacedPath, replacedPhysical);

        generation::GenerateRequest original;
        original.inputJsonPath = inputPath;
        original.templateSavePath = GetOptionalFlag(args, "--template").value_or(
            DefaultTemplatePath().string());
        original.profilePath = GetOptionalFlag(args, "--profile").value_or(
            DefaultProfilePath().string());
        original.outputSavePath = workDir / "physical-image-original.sav";
        original.outputReportPath = workDir / "physical-image-original.report.json";

        generation::GenerateRequest removed = original;
        removed.inputJsonPath = noPhysicalPath;
        removed.outputSavePath = workDir / "physical-image-removed.sav";
        removed.outputReportPath = workDir / "physical-image-removed.report.json";

        generation::GenerateRequest replaced = original;
        replaced.inputJsonPath = replacedPath;
        replaced.outputSavePath = workDir / "physical-image-replaced.sav";
        replaced.outputReportPath = workDir / "physical-image-replaced.report.json";

        const auto a = generation::MinimalSaveGenerator::Generate(original);
        const auto b = generation::MinimalSaveGenerator::Generate(removed);
        const auto c = generation::MinimalSaveGenerator::Generate(replaced);
        const bool same = a.outputBytes == b.outputBytes && a.outputBytes == c.outputBytes;
        std::cout << "Original SHA-256: " << a.report.outputSha256 << "\n";
        std::cout << "Removed physicalImage SHA-256: " << b.report.outputSha256 << "\n";
        std::cout << "Replaced physicalImage SHA-256: " << c.report.outputSha256 << "\n";
        std::cout << "Physical-image isolated: " << (same ? "yes" : "no") << "\n";
        return same ? 0 : 1;
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
        options.compareDaycare = true;
        options.compareHallOfFame = true;
        options.compareWorldSubset = true;
        options.compareMissableObjects = true;
        options.compareEvents = true;
        options.compareTrainerBattles = true;
        options.compareStaticBattles = true;
        options.compareStoryProgress = true;
        options.compareScripts = true;
        const auto differences = comparison::SemanticComparator::CompareOwnedFields(
            contract.expectedSemantic, actualSemantic.state, options);
        const bool blocking = comparison::SemanticComparator::HasBlockingDifferences(differences);
        nlohmann::json differenceJson = nlohmann::json::array();
        std::ostringstream markdown;
        markdown << "# Semantic Comparison\n\n"
                 << "- Status: **" << (blocking ? "FAIL" : "PASS") << "**\n"
                 << "- Differences: " << differences.size() << "\n\n"
                 << "| Path | Category | Expected | Actual |\n"
                 << "|---|---|---|---|\n";
        for (const auto& difference : differences) {
            const std::string category =
                comparison::SemanticComparator::CategoryLabel(difference.category);
            differenceJson.push_back({
                {"path", difference.fieldPath},
                {"category", category},
                {"expected", difference.expectedValue},
                {"actual", difference.actualValue},
                {"blocking", difference.category !=
                    comparison::DifferenceCategory::PermittedCanonicalDifference}
            });
            markdown << "| `" << difference.fieldPath << "` | " << category
                     << " | `" << difference.expectedValue << "` | `"
                     << difference.actualValue << "` |\n";
        }
        if (const auto reportPath = GetOptionalFlag(args, "--report")) {
            reporting::ComparisonReportWriter::WriteJson(
                *reportPath,
                {
                    {"reportFormatVersion", "1.0.0"},
                    {"status", blocking ? "fail" : "pass"},
                    {"differenceCount", differences.size()},
                    {"differences", std::move(differenceJson)}
                });
        }
        if (const auto markdownPath = GetOptionalFlag(args, "--markdown")) {
            reporting::ComparisonReportWriter::WriteMarkdown(
                *markdownPath, markdown.str());
        }
        if (differences.empty()) {
            std::cout << "Semantic comparison: PASS\n";
            return 0;
        }

        if (!blocking) {
            std::cout << "Semantic comparison: PASS with permitted canonical differences\n";
            for (const auto& difference : differences) {
                std::cout << difference.fieldPath
                          << " [" << comparison::SemanticComparator::CategoryLabel(difference.category) << "]"
                          << ": expected=" << difference.expectedValue
                          << " actual=" << difference.actualValue << "\n";
            }
            return 0;
        }

        std::cout << "Semantic comparison: FAIL\n";
        for (const auto& difference : differences) {
            std::cout << difference.fieldPath
                      << " [" << comparison::SemanticComparator::CategoryLabel(difference.category) << "]"
                      << ": expected=" << difference.expectedValue
                      << " actual=" << difference.actualValue << "\n";
        }
        return 1;
    }

    static model::RedSemanticState LoadSemanticInput(
        const std::filesystem::path& inputPath) {
        const auto parsed = input::RedJsonReader::ReadFromFile(inputPath);
        input::RedJsonValidator::Validate(parsed.document);
        const auto sanitized = input::PhysicalImageIsolationGuard::Sanitize(parsed.document);
        const auto semantic = model::RedSemanticStateBuilder::Build(
            sanitized.document, sanitized.physicalImageRemoved);
        if (!semantic.ok) {
            throw std::runtime_error(
                semantic.errors.empty() ? "failed to build semantic input" : semantic.errors.front());
        }
        return semantic.state;
    }

    static int RunValidateText(const std::vector<std::string>& args) {
        const std::string text = GetRequiredFlag(args, "--text");
        const auto encoded = encoding::Gen1TextEncoder::EncodeName(text, 11U);
        std::cout << "Encoded bytes:";
        for (const auto byte : encoded) {
            std::cout << " " << std::uppercase << std::hex
                      << static_cast<int>(byte) << std::dec;
        }
        std::cout << "\nDisplay text: "
                  << encoding::Gen1TextEncoder::DecodeName(encoded, 0U, encoded.size())
                  << "\nLossless text: "
                  << encoding::Gen1TextEncoder::DecodeNameLossless(
                         encoded, 0U, encoded.size())
                  << "\nText validation: PASS\n";
        return 0;
    }

    static int RunValidateBoxes(const std::vector<std::string>& args) {
        const auto semantic = LoadSemanticInput(InputPathFromArgs(args));
        generation::StorageValidator::ValidateOrThrow(semantic.storage);
        std::size_t total = 0;
        for (const auto& box : semantic.storage.boxes) total += box.pokemon.size();
        std::cout << "Permanent boxes: 12\n"
                  << "Permanent Pokemon: " << total << "\n"
                  << "Selected box: " << semantic.storage.selectedBoxNumber << "\n"
                  << "Selected permanent count: "
                  << semantic.storage.boxes[
                         static_cast<std::size_t>(semantic.storage.selectedBoxNumber - 1)]
                         .pokemon.size()
                  << "\nCurrent working-box cache count: "
                  << semantic.storage.currentBoxCache.pokemon.size() << "\n"
                  << "Storage validation: PASS\n";
        return 0;
    }

    static int RunValidatePokemonOperability(const std::vector<std::string>& args) {
        const auto semantic = LoadSemanticInput(InputPathFromArgs(args));
        std::size_t validated = 0;
        std::size_t currentHpAboveDerivedMax = 0;
        const auto validate = [&](const model::StoredPokemonState& mon,
                                  const std::string& path) {
            generation::StorageValidator::ValidatePokemonOperabilityOrThrow(mon, path);
            const auto* species = pokemon::FindSpeciesData(mon.speciesId);
            const auto stats = generation::PokemonStatCalculator::CalculateStoredPokemonStats(
                *species, mon);
            if (mon.currentHp > stats.maxHp) {
                ++currentHpAboveDerivedMax;
                std::cout << "warning: " << path << " stores HP " << mon.currentHp
                          << " above derived maximum " << stats.maxHp
                          << "; the source value will be preserved.\n";
            }
            ++validated;
        };
        for (std::size_t box = 0; box < semantic.storage.boxes.size(); ++box) {
            for (std::size_t slot = 0; slot < semantic.storage.boxes[box].pokemon.size(); ++slot) {
                validate(
                    semantic.storage.boxes[box].pokemon[slot],
                    "storage.boxes[" + std::to_string(box) + "].pokemon[" +
                        std::to_string(slot) + "]");
            }
        }
        for (std::size_t slot = 0; slot < semantic.storage.currentBoxCache.pokemon.size(); ++slot) {
            validate(
                semantic.storage.currentBoxCache.pokemon[slot],
                "storage.currentBoxCache.pokemon[" + std::to_string(slot) + "]");
        }
        if (semantic.daycare.pokemon.has_value()) {
            validate(
                *semantic.daycare.pokemon, "daycare.pokemon");
        }
        std::cout << "Operationally validated stored Pokemon: " << validated << "\n"
                  << "Stored HP values above derived maximum: "
                  << currentHpAboveDerivedMax << " (preserved, not silently normalized)\n"
                  << "Pokemon operability validation: PASS\n";
        return 0;
    }

    static int RunValidateHallOfFame(const std::vector<std::string>& args) {
        const auto semantic = LoadSemanticInput(InputPathFromArgs(args));
        generation::HallOfFameSerializer::ValidateOrThrow(semantic.hallOfFame);
        std::size_t pokemonCount = 0;
        for (const auto& entry : semantic.hallOfFame.entries) {
            pokemonCount += entry.pokemon.size();
        }
        std::cout << "Hall of Fame entries: " << semantic.hallOfFame.entries.size() << "\n"
                  << "Hall of Fame Pokemon: " << pokemonCount << "\n"
                  << "Hall of Fame validation: PASS\n";
        return 0;
    }

    static void PrintStoredPokemon(const model::StoredPokemonState& mon) {
        const auto* species = pokemon::FindSpeciesData(mon.speciesId);
        const auto stats = generation::PokemonStatCalculator::CalculateStoredPokemonStats(
            *species, mon);
        std::cout << "Position: " << mon.position << "\n"
                  << "Species: " << mon.speciesName
                  << " (internal=" << static_cast<int>(mon.speciesId) << ")\n"
                  << "Nickname: " << mon.nickname << "\n"
                  << "OT: " << mon.originalTrainerName
                  << " / " << mon.originalTrainerId << "\n"
                  << "Level: " << static_cast<int>(mon.level)
                  << " EXP: " << mon.experience << "\n"
                  << "Stored HP: " << mon.currentHp
                  << " Derived max HP on withdrawal: " << stats.maxHp << "\n";
    }

    static int RunInspectBox(const std::vector<std::string>& args) {
        const auto semantic = LoadSemanticInput(InputPathFromArgs(args));
        const int boxNumber = std::stoi(GetRequiredFlag(args, "--box"));
        if (boxNumber < 1 || boxNumber > 12) {
            throw std::runtime_error("--box must be in 1..12");
        }
        const bool current = HasFlag(args, "--current-cache");
        const auto& box = current
            ? semantic.storage.currentBoxCache
            : semantic.storage.boxes[static_cast<std::size_t>(boxNumber - 1)];
        if (current && boxNumber != semantic.storage.selectedBoxNumber) {
            throw std::runtime_error(
                "--current-cache must use the selected box number " +
                std::to_string(semantic.storage.selectedBoxNumber));
        }
        std::cout << (current ? "Current working-box cache" : "Permanent box")
                  << " " << boxNumber << " count: " << box.pokemon.size() << "\n";
        for (const auto& mon : box.pokemon) {
            std::cout << "[" << mon.position << "] " << mon.nickname
                      << " species=" << mon.speciesName
                      << " level=" << static_cast<int>(mon.level)
                      << " hp=" << mon.currentHp << "\n";
        }
        return 0;
    }

    static int RunInspectPokemon(const std::vector<std::string>& args) {
        const auto semantic = LoadSemanticInput(InputPathFromArgs(args));
        const int boxNumber = std::stoi(GetRequiredFlag(args, "--box"));
        const int slot = std::stoi(GetRequiredFlag(args, "--slot"));
        if (boxNumber < 1 || boxNumber > 12 || slot < 1 || slot > 20) {
            throw std::runtime_error("--box must be in 1..12 and --slot in 1..20");
        }
        const bool current = HasFlag(args, "--current-cache");
        const auto& box = current
            ? semantic.storage.currentBoxCache
            : semantic.storage.boxes[static_cast<std::size_t>(boxNumber - 1)];
        if (current && boxNumber != semantic.storage.selectedBoxNumber) {
            throw std::runtime_error(
                "--current-cache must use the selected box number " +
                std::to_string(semantic.storage.selectedBoxNumber));
        }
        if (static_cast<std::size_t>(slot) > box.pokemon.size()) {
            throw std::runtime_error("requested slot is empty");
        }
        PrintStoredPokemon(box.pokemon[static_cast<std::size_t>(slot - 1)]);
        return 0;
    }

    static int RunComparePhysical(const std::vector<std::string>& args) {
        const std::filesystem::path originalPath = GetRequiredFlag(args, "--original");
        const std::filesystem::path generatedPath = GetRequiredFlag(args, "--generated");
        const auto original = ReadBinaryFile(originalPath);
        const auto generated = ReadBinaryFile(generatedPath);
        const auto result = comparison::PhysicalComparator::Compare(original, generated);
        const std::string originalHash = template_support::Sha256::Hex(original);
        const std::string generatedHash = template_support::Sha256::Hex(generated);
        if (const auto report = GetOptionalFlag(args, "--report")) {
            reporting::ComparisonReportWriter::WriteJson(
                *report,
                reporting::ComparisonReportWriter::PhysicalJson(
                    result, originalHash, generatedHash));
        }
        if (const auto markdown = GetOptionalFlag(args, "--markdown")) {
            reporting::ComparisonReportWriter::WriteMarkdown(
                *markdown,
                reporting::ComparisonReportWriter::PhysicalMarkdown(
                    result, originalHash, generatedHash));
        }
        std::cout << "Original SHA-256: " << originalHash << "\n"
                  << "Compared SHA-256: " << generatedHash << "\n"
                  << "Equal bytes: " << result.equalBytes << "\n"
                  << "Differing bytes: " << result.differingBytes << "\n"
                  << "Byte-identical: " << (result.byteIdentical() ? "yes" : "no") << "\n";
        return 0;
    }

    static int RunAnalyzePostEmulator(const std::vector<std::string>& args) {
        const std::filesystem::path beforePath = GetRequiredFlag(args, "--before");
        const std::filesystem::path afterPath = GetRequiredFlag(args, "--after");
        const auto before = ReadBinaryFile(beforePath);
        const auto after = ReadBinaryFile(afterPath);
        const auto physical = comparison::PhysicalComparator::Compare(before, after);
        const auto beforeIntegrity = integrity::IntegrityValidator::ValidateGeneratedSave(before);
        const auto afterIntegrity = integrity::IntegrityValidator::ValidateGeneratedSave(after);
        const std::string beforeHash = template_support::Sha256::Hex(before);
        const std::string afterHash = template_support::Sha256::Hex(after);

        nlohmann::json report = {
            {"reportFormatVersion", "1.0.0"},
            {"status", afterIntegrity.ok ? "integrity-passed" : "integrity-failed"},
            {"physical", reporting::ComparisonReportWriter::PhysicalJson(
                physical, beforeHash, afterHash)},
            {"beforeIntegrity", reporting::ComparisonReportWriter::IntegrityJson(beforeIntegrity)},
            {"afterIntegrity", reporting::ComparisonReportWriter::IntegrityJson(afterIntegrity)}
        };
        if (const auto reportPath = GetOptionalFlag(args, "--report")) {
            reporting::ComparisonReportWriter::WriteJson(*reportPath, report);
        }
        if (const auto markdownPath = GetOptionalFlag(args, "--markdown")) {
            std::ostringstream markdown;
            markdown << reporting::ComparisonReportWriter::PhysicalMarkdown(
                physical, beforeHash, afterHash, "Post-Emulator Physical Comparison")
                << "\n## Integrity\n\n"
                << "| State | Main | Bank 2 | Bank 3 | Cache structure | Overall |\n"
                << "|---|---|---|---|---|---|\n"
                << "| Before | " << (beforeIntegrity.mainChecksumValid ? "valid" : "invalid")
                << " | " << (beforeIntegrity.bank2ChecksumValid ? "valid" : "invalid")
                << " | " << (beforeIntegrity.bank3ChecksumValid ? "valid" : "invalid")
                << " | " << (beforeIntegrity.currentBoxCacheValid ? "valid" : "invalid")
                << " | " << (beforeIntegrity.ok ? "PASS" : "FAIL") << " |\n"
                << "| After | " << (afterIntegrity.mainChecksumValid ? "valid" : "invalid")
                << " | " << (afterIntegrity.bank2ChecksumValid ? "valid" : "invalid")
                << " | " << (afterIntegrity.bank3ChecksumValid ? "valid" : "invalid")
                << " | " << (afterIntegrity.currentBoxCacheValid ? "valid" : "invalid")
                << " | " << (afterIntegrity.ok ? "PASS" : "FAIL") << " |\n";
            reporting::ComparisonReportWriter::WriteMarkdown(
                *markdownPath, markdown.str());
        }
        std::cout << "Before SHA-256: " << beforeHash << "\n"
                  << "After SHA-256: " << afterHash << "\n"
                  << "Changed bytes: " << physical.differingBytes << "\n"
                  << "Before integrity: " << (beforeIntegrity.ok ? "PASS" : "FAIL") << "\n"
                  << "After integrity: " << (afterIntegrity.ok ? "PASS" : "FAIL") << "\n";
        return afterIntegrity.ok ? 0 : 1;
    }
};

inline int Run(int argc, const char* argv[]) {
    return Application::Run(argc, argv);
}

}  // namespace pkmn::savegen::cli
