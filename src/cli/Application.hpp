#pragma once

#include <algorithm>
#include <array>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

#include "../comparison/SemanticComparator.hpp"
#include "../encoding/Gen1Layout.hpp"
#include "../encoding/PrimitiveWriter.hpp"
#include "../generation/MinimalSaveGenerator.hpp"
#include "../generation/MinimalStateContract.hpp"
#include "../generation/RedSaveInitializer.hpp"
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
            if (command == "--help" || command == "-h" || command == "help") {
                PrintUsage();
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
            << "  generate <input.red.json> <output.sav> [--report <report.json>] [--dry-run] [--summary] [--show-ranges]\n"
            << "  generate --input <target.red.json> --template <dummy.sav> --profile <profile.json> --output <generated.sav> --report <report.json> [--dry-run]\n"
            << "  validate-input|validate --input <path>\n"
            << "  validate-template --template <path> --profile <path>\n"
            << "  validate-save --input-save <path> [--allow-dirty-current-box]\n"
            << "  inspect --input <path>\n"
            << "  show-profile --profile <path>\n"
            << "  check-determinism --input <path> --template <dummy.sav> --profile <profile.json> --work-dir <dir>\n"
            << "  check-physical-image-isolation --input <path> --template <dummy.sav> --profile <profile.json> --work-dir <dir>\n"
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
        std::cout << "Bag items: " << semantic.inventory.bagItems.size()
                  << ", PC items: " << semantic.inventory.pcItems.size() << "\n";
        std::cout << "Pokedex owned/seen: " << semantic.pokedex.ownedCount
                  << "/" << semantic.pokedex.seenCount << "\n";
        std::cout << "Daycare in use: " << (semantic.daycare.inUse ? "yes" : "no") << "\n";
        std::cout << "Hall of Fame entries: " << semantic.hallOfFame.entries.size() << "\n";
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
        const bool allowDirtyCurrentBox = HasFlag(args, "--allow-dirty-current-box");
        const std::vector<std::uint8_t> bytes = ReadBinaryFile(savePath);
        const bool sizeOk = bytes.size() == encoding::Gen1Layout::ExpectedSaveSize;
        std::cout << "Save: " << savePath.string() << "\n";
        std::cout << "Size: " << bytes.size() << " bytes\n";
        std::cout << "SHA-256: " << template_support::Sha256::Hex(bytes) << "\n";
        if (!sizeOk) {
            std::cerr << "error: save size is not 32768 bytes\n";
            return 1;
        }

        const bool mainOk = integrity::ChecksumAlgorithms::ValidateMainChecksum(bytes);
        const bool bank2Ok = integrity::ChecksumAlgorithms::ValidateBank2AllChecksum(bytes);
        const bool bank3Ok = integrity::ChecksumAlgorithms::ValidateBank3AllChecksum(bytes);
        std::array<bool, 12> boxOk{};
        bool allBoxesOk = true;
        for (int box = 1; box <= 12; ++box) {
            boxOk[static_cast<std::size_t>(box - 1)] =
                integrity::ChecksumAlgorithms::ValidateBoxChecksum(bytes, box);
            allBoxesOk = allBoxesOk && boxOk[static_cast<std::size_t>(box - 1)];
        }

        const std::uint8_t rawCurrentBox =
            encoding::PrimitiveWriter::ReadU8(bytes, encoding::Gen1Layout::CurrentBoxByteOff);
        const bool dirty = (rawCurrentBox & 0x80U) != 0;
        const int selectedBox = static_cast<int>(rawCurrentBox & 0x7FU) + 1;
        bool cacheMatchesPermanent = false;
        if (selectedBox >= 1 && selectedBox <= 12) {
            const std::size_t permanentBase =
                encoding::Gen1Layout::PermanentBoxOffsets[static_cast<std::size_t>(selectedBox - 1)];
            cacheMatchesPermanent = true;
            for (std::size_t i = 0; i < encoding::Gen1Layout::BoxBlockSize; ++i) {
                if (bytes[permanentBase + i] != bytes[encoding::Gen1Layout::CurrentBoxCacheOff + i]) {
                    cacheMatchesPermanent = false;
                    break;
                }
            }
        }

        std::cout << "Main checksum valid: " << (mainOk ? "yes" : "no") << "\n";
        std::cout << "Bank 2 all checksum valid: " << (bank2Ok ? "yes" : "no") << "\n";
        std::cout << "Bank 3 all checksum valid: " << (bank3Ok ? "yes" : "no") << "\n";
        std::cout << "Per-box checksums:";
        for (int box = 1; box <= 12; ++box) {
            std::cout << " box" << box << "="
                      << (boxOk[static_cast<std::size_t>(box - 1)] ? "valid" : "invalid");
        }
        std::cout << "\n";
        std::cout << "Selected box: " << selectedBox
                  << " raw=0x" << std::hex << static_cast<int>(rawCurrentBox) << std::dec
                  << " dirty=" << (dirty ? "yes" : "no") << "\n";
        std::cout << "Current-box cache matches permanent selected box: "
                  << (cacheMatchesPermanent ? "yes" : "no") << "\n";
        if (!cacheMatchesPermanent && dirty && allowDirtyCurrentBox) {
            std::cout << "Current-box cache divergence accepted because --allow-dirty-current-box was supplied.\n";
        }

        const bool cacheOk = cacheMatchesPermanent || (allowDirtyCurrentBox && dirty);
        const bool ok = mainOk && bank2Ok && bank3Ok && allBoxesOk && cacheOk;
        if (!ok) {
            std::cerr << "error: save validation failed\n";
        }
        return ok ? 0 : 1;
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
        std::cout << "Current-box cache synchronized: "
                  << (result.integrity.currentBoxCacheSynchronized ? "yes" : "no") << "\n";
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

        const auto a = generation::MinimalSaveGenerator::Generate(first);
        const auto b = generation::MinimalSaveGenerator::Generate(second);
        const bool same = a.outputBytes == b.outputBytes;
        std::cout << "First SHA-256: " << a.report.outputSha256 << "\n";
        std::cout << "Second SHA-256: " << b.report.outputSha256 << "\n";
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
        if (differences.empty()) {
            std::cout << "Semantic comparison: PASS\n";
            return 0;
        }

        if (!comparison::SemanticComparator::HasBlockingDifferences(differences)) {
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
};

inline int Run(int argc, const char* argv[]) {
    return Application::Run(argc, argv);
}

}  // namespace pkmn::savegen::cli
