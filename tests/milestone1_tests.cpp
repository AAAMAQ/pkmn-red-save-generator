#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

#include <doctest/doctest.h>

#include "../src/comparison/SemanticComparator.hpp"
#include "../src/encoding/BcdEncoder.hpp"
#include "../src/encoding/Gen1Layout.hpp"
#include "../src/encoding/BitfieldWriter.hpp"
#include "../src/encoding/Gen1TextEncoder.hpp"
#include "../src/encoding/PrimitiveWriter.hpp"
#include "../src/generation/MinimalSaveGenerator.hpp"
#include "../src/generation/MinimalStateContract.hpp"
#include "../src/generation/RedSaveInitializer.hpp"
#include "../src/input/PhysicalImageIsolationGuard.hpp"
#include "../src/input/RedJsonReader.hpp"
#include "../src/input/RedJsonValidator.hpp"
#include "../src/integrity/IntegrityValidator.hpp"
#include "../src/model/RedSemanticState.hpp"
#include "../src/template/CanonicalTemplateLoader.hpp"
#include "../src/template/TemplateProfile.hpp"
#include "../src/template/TemplateValidator.hpp"

namespace {

std::filesystem::path RepoRoot() {
    std::filesystem::path current = std::filesystem::current_path();
    while (!current.empty()) {
        if (std::filesystem::exists(current / "Dummy Save") &&
            std::filesystem::exists(current / "profiles" / "pokemon-red-usa-europe-v1.json")) {
            return current;
        }
        current = current.parent_path();
    }
    throw std::runtime_error("failed to locate repository root");
}

std::filesystem::path DummyJsonPath() {
    return RepoRoot() / "Dummy Save" / "Pokemon - Red Version (USA, Europe) (SGB Enhanced).red.json";
}

std::filesystem::path DummySavPath() {
    return RepoRoot() / "Dummy Save" / "Pokemon - Red Version (USA, Europe) (SGB Enhanced).sav";
}

std::filesystem::path ProfilePath() {
    return RepoRoot() / "profiles" / "pokemon-red-usa-europe-v1.json";
}

std::filesystem::path MakeTempDir(const std::string& name) {
    const auto dir = std::filesystem::temp_directory_path() / ("pkmn-red-save-generator-" + name);
    std::filesystem::remove_all(dir);
    std::filesystem::create_directories(dir);
    return dir;
}

void WriteJson(const std::filesystem::path& path, const nlohmann::json& json) {
    std::ofstream out(path);
    REQUIRE(out.good());
    out << json.dump(2) << "\n";
}

std::vector<bool> ReadBits(const std::vector<std::uint8_t>& buffer,
                           std::size_t byteOffset,
                           std::size_t bitCount) {
    std::vector<bool> bits;
    bits.reserve(bitCount);
    for (std::size_t i = 0; i < bitCount; ++i) {
        bits.push_back(pkmn::savegen::encoding::BitfieldWriter::ReadBit(
            buffer, byteOffset + (i / 8U), static_cast<std::uint8_t>(i % 8U)));
    }
    return bits;
}

std::vector<pkmn::savegen::model::InventoryItem> ReadItemList(const std::vector<std::uint8_t>& buffer,
                                                              std::size_t countOffset,
                                                              std::size_t pairsOffset,
                                                              int maxPairs) {
    std::vector<pkmn::savegen::model::InventoryItem> items;
    const auto count =
        pkmn::savegen::encoding::PrimitiveWriter::ReadU8(buffer, countOffset);
    REQUIRE(count <= maxPairs);
    std::size_t cursor = pairsOffset;
    for (int i = 0; i < count; ++i) {
        const auto id = pkmn::savegen::encoding::PrimitiveWriter::ReadU8(buffer, cursor);
        const auto quantity = pkmn::savegen::encoding::PrimitiveWriter::ReadU8(buffer, cursor + 1U);
        items.push_back({id, "", quantity});
        cursor += 2U;
    }
    CHECK(pkmn::savegen::encoding::PrimitiveWriter::ReadU8(buffer, cursor) == 0xFF);
    return items;
}

}  // namespace

TEST_CASE("Primitive writers encode and validate core formats") {
    std::vector<std::uint8_t> buffer(8, 0);

    pkmn::savegen::encoding::PrimitiveWriter::WriteU24BigEndian(buffer, 0, 3000U);
    CHECK(pkmn::savegen::encoding::PrimitiveWriter::ReadU24BigEndian(buffer, 0) == 3000U);

    pkmn::savegen::encoding::BcdEncoder::Write3(buffer, 0, 123456U);
    CHECK(pkmn::savegen::encoding::BcdEncoder::Decode3(buffer, 0) == 123456U);

    pkmn::savegen::encoding::BcdEncoder::Write2(buffer, 3, 4321U);
    CHECK(pkmn::savegen::encoding::BcdEncoder::Decode2(buffer, 3) == 4321U);

    pkmn::savegen::encoding::BitfieldWriter::WriteBit(buffer, 5, 3, true);
    CHECK(pkmn::savegen::encoding::BitfieldWriter::ReadBit(buffer, 5, 3));
    pkmn::savegen::encoding::BitfieldWriter::WriteBit(buffer, 5, 3, false);
    CHECK_FALSE(pkmn::savegen::encoding::BitfieldWriter::ReadBit(buffer, 5, 3));

    const std::vector<std::uint8_t> encoded =
        pkmn::savegen::encoding::Gen1TextEncoder::EncodeName("Az-9?!", 11);
    std::vector<std::uint8_t> textBuffer(11, 0x50);
    pkmn::savegen::encoding::PrimitiveWriter::WriteBytes(textBuffer, 0, encoded);
    CHECK(pkmn::savegen::encoding::Gen1TextEncoder::DecodeName(textBuffer, 0, 11) == "Az-9?!");

    CHECK_THROWS_AS(
        pkmn::savegen::encoding::PrimitiveWriter::WriteU24BigEndian(buffer, 6, 1U),
        std::out_of_range);
}

TEST_CASE("Dummy red json validates and builds a semantic model") {
    const auto parsed = pkmn::savegen::input::RedJsonReader::ReadFromFile(DummyJsonPath());
    const auto validation = pkmn::savegen::input::RedJsonValidator::Validate(parsed.document);
    CHECK(validation.ok);
    CHECK(validation.schemaVersion == "0.1.0");
    CHECK(validation.targetGame == "Pokemon Red");
    CHECK(validation.physicalImagePresent);

    const auto sanitized = pkmn::savegen::input::PhysicalImageIsolationGuard::Sanitize(parsed.document);
    CHECK(sanitized.physicalImageRemoved);
    CHECK_FALSE(sanitized.document.contains("physicalImage"));

    const auto build = pkmn::savegen::model::RedSemanticStateBuilder::Build(
        sanitized.document, sanitized.physicalImageRemoved);
    REQUIRE(build.ok);
    CHECK(build.state.identity.playerName == "RED");
    CHECK(build.state.identity.rivalName == "BLUE");
    CHECK(build.state.identity.trainerId == 60066);
    CHECK(build.state.core.money == 3000U);
    CHECK(build.state.inventory.pcItems.size() == 1);
    CHECK(build.state.inventory.pcItems.front().name == "POTION");
    CHECK(build.state.pokedex.owned.size() == 151);
    CHECK(build.state.party.count == 0);
    CHECK_FALSE(build.state.daycare.inUse);
    CHECK(build.state.hallOfFame.entryCount == 0);
    CHECK(build.state.eventSubset.visitedTowns.size() == 11);
    CHECK(build.state.physicalImageIgnored);
}

TEST_CASE("Physical image changes do not affect semantic state mapping") {
    const auto parsed = pkmn::savegen::input::RedJsonReader::ReadFromFile(DummyJsonPath());
    const auto originalSanitized = pkmn::savegen::input::PhysicalImageIsolationGuard::Sanitize(parsed.document);
    const auto originalBuild =
        pkmn::savegen::model::RedSemanticStateBuilder::Build(originalSanitized.document, true);
    REQUIRE(originalBuild.ok);

    std::vector<nlohmann::json> variants;
    nlohmann::json removed = parsed.document;
    removed.erase("physicalImage");
    variants.push_back(removed);

    nlohmann::json invalidText = parsed.document;
    invalidText["physicalImage"] = "definitely-not-a-raw-image";
    variants.push_back(invalidText);

    nlohmann::json unrelatedObject = parsed.document;
    unrelatedObject["physicalImage"] = {
        {"encoding", "hex_uppercase_continuous"},
        {"standardSramHex", "ABCD"},
        {"trailingDataHex", ""},
        {"totalLength", 2},
        {"standardSramLength", 2},
        {"trailingLength", 0}
    };
    variants.push_back(unrelatedObject);

    nlohmann::json anotherObject = parsed.document;
    anotherObject["physicalImage"] = {
        {"encoding", "ignored"},
        {"payload", "FFFF"},
        {"unexpected", true}
    };
    variants.push_back(anotherObject);

    for (const auto& variant : variants) {
        const auto validation = pkmn::savegen::input::RedJsonValidator::Validate(variant);
        CHECK(validation.errors.empty());
        const auto sanitized = pkmn::savegen::input::PhysicalImageIsolationGuard::Sanitize(variant);
        const auto build = pkmn::savegen::model::RedSemanticStateBuilder::Build(sanitized.document, true);
        REQUIRE(build.ok);
        const auto differences =
            pkmn::savegen::comparison::SemanticComparator::CompareOwnedFields(originalBuild.state, build.state);
        CHECK(differences.empty());
    }
}

TEST_CASE("Template profile and validator analyze the committed dummy") {
    const auto profile =
        pkmn::savegen::template_profile::TemplateProfileLoader::LoadFromFile(ProfilePath());
    const auto loaded =
        pkmn::savegen::template_loader::CanonicalTemplateLoader::Load(DummySavPath());
    const auto validation =
        pkmn::savegen::template_validation::TemplateValidator::Validate(profile, loaded);

    CHECK(validation.accepted);
    CHECK(validation.sizeMatches);
    CHECK(validation.hashMatches);
    CHECK(validation.baseline.mainChecksumValid);
    CHECK_FALSE(validation.baseline.bank2AllChecksumValid);
    CHECK_FALSE(validation.baseline.bank3AllChecksumValid);
    CHECK(validation.baseline.currentBoxCacheCount == 0);
    CHECK(validation.baseline.selectedBoxNumber == 1);
    CHECK(validation.baseline.permanentBoxCounts.front() == 0xFF);
    CHECK(validation.baseline.permanentBoxesSuspicious);
    CHECK(validation.baseline.cacheDiffersFromPermanent);
    CHECK_FALSE(validation.warnings.empty());
}

TEST_CASE("Deterministic working-buffer initialization copies the canonical template exactly") {
    const auto profile =
        pkmn::savegen::template_profile::TemplateProfileLoader::LoadFromFile(ProfilePath());
    const auto loaded =
        pkmn::savegen::template_loader::CanonicalTemplateLoader::Load(DummySavPath());

    const auto first = pkmn::savegen::generation::RedSaveInitializer::Initialize(loaded, profile, true);
    const auto second = pkmn::savegen::generation::RedSaveInitializer::Initialize(loaded, profile, true);

    CHECK(first.bytes == loaded.bytes);
    CHECK(second.bytes == loaded.bytes);
    CHECK(first.bytes == second.bytes);
    CHECK(first.report.templateSha256 == loaded.sha256);
    CHECK(first.report.physicalImageIgnored);
    REQUIRE(first.report.ranges.size() == 1);
    CHECK(first.report.ranges.front().classification == "template-inherited");
}

TEST_CASE("Milestone 3 generator writes owned semantic fields from target input") {
    const auto tempDir = MakeTempDir("milestone2-generate");
    const auto outputPath = tempDir / "generated.sav";
    const auto reportPath = tempDir / "generated.generation-report.json";

    pkmn::savegen::generation::GenerateRequest request;
    request.inputJsonPath = DummyJsonPath();
    request.templateSavePath = DummySavPath();
    request.profilePath = ProfilePath();
    request.outputSavePath = outputPath;
    request.outputReportPath = reportPath;

    const auto result = pkmn::savegen::generation::MinimalSaveGenerator::Generate(request);

    CHECK(std::filesystem::exists(outputPath));
    CHECK(std::filesystem::exists(reportPath));
    CHECK(result.outputBytes.size() == pkmn::savegen::encoding::Gen1Layout::ExpectedSaveSize);
    CHECK(result.integrity.ok);
    CHECK(result.integrity.mainChecksumValid);
    CHECK_FALSE(result.integrity.bank2ChecksumValid);
    CHECK_FALSE(result.integrity.bank3ChecksumValid);
    CHECK(result.integrity.bankStorageUnchanged);

    CHECK(pkmn::savegen::encoding::Gen1TextEncoder::DecodeName(
              result.outputBytes,
              pkmn::savegen::encoding::Gen1Layout::TrainerNameOff,
              pkmn::savegen::encoding::Gen1Layout::TrainerNameLen) == "RED");
    CHECK(pkmn::savegen::encoding::Gen1TextEncoder::DecodeName(
              result.outputBytes,
              pkmn::savegen::encoding::Gen1Layout::RivalNameOff,
              pkmn::savegen::encoding::Gen1Layout::RivalNameLen) == "BLUE");
    CHECK(pkmn::savegen::encoding::PrimitiveWriter::ReadU16BigEndian(
              result.outputBytes,
              pkmn::savegen::encoding::Gen1Layout::TrainerIdOff) == 60066);
    CHECK(pkmn::savegen::encoding::BcdEncoder::Decode3(
              result.outputBytes,
              pkmn::savegen::encoding::Gen1Layout::MoneyOff) == 3000U);
    CHECK(pkmn::savegen::encoding::BcdEncoder::Decode2(
              result.outputBytes,
              pkmn::savegen::encoding::Gen1Layout::CoinsOff) == 0U);
    CHECK(result.outputBytes[pkmn::savegen::encoding::Gen1Layout::BadgesOff] == 0);
    CHECK(result.outputBytes[pkmn::savegen::encoding::Gen1Layout::BadgesMirrorOff] == 0);
    CHECK(result.outputBytes[pkmn::savegen::encoding::Gen1Layout::BagItemsCountOff] == 0);
    CHECK(result.outputBytes[pkmn::savegen::encoding::Gen1Layout::BagItemsPairsOff] == 0xFF);
    CHECK(result.outputBytes[pkmn::savegen::encoding::Gen1Layout::PCItemBoxCountOff] == 1);
    CHECK(result.outputBytes[pkmn::savegen::encoding::Gen1Layout::PCItemBoxPairsOff] == 20);
    CHECK(result.outputBytes[pkmn::savegen::encoding::Gen1Layout::PCItemBoxPairsOff + 1] == 1);
    CHECK(result.outputBytes[pkmn::savegen::encoding::Gen1Layout::PCItemBoxPairsOff + 2] == 0xFF);
    CHECK(result.outputBytes[pkmn::savegen::encoding::Gen1Layout::PartyCountOff] == 0);
    CHECK(result.outputBytes[pkmn::savegen::encoding::Gen1Layout::PartySpeciesOff] == 0xFF);
    CHECK(result.outputBytes[pkmn::savegen::encoding::Gen1Layout::HallOfFameRecordCountOff] == 0);
    CHECK(result.outputBytes[pkmn::savegen::encoding::Gen1Layout::DaycareInUseOff] == 0);
    CHECK(result.outputBytes[pkmn::savegen::encoding::Gen1Layout::MapIdOff] == 38);
    CHECK(result.outputBytes[pkmn::savegen::encoding::Gen1Layout::XCoordOff] == 3);
    CHECK(result.outputBytes[pkmn::savegen::encoding::Gen1Layout::YCoordOff] == 6);
    CHECK(ReadBits(result.outputBytes,
                   pkmn::savegen::encoding::Gen1Layout::VisitedTownsOff,
                   pkmn::savegen::encoding::Gen1Layout::VisitedTownsUsedBits) ==
          std::vector<bool>(pkmn::savegen::encoding::Gen1Layout::VisitedTownsUsedBits, false));
}

TEST_CASE("Milestone 2 generation ignores target physicalImage and is deterministic") {
    const auto parsed = pkmn::savegen::input::RedJsonReader::ReadFromFile(DummyJsonPath());
    const auto tempDir = MakeTempDir("milestone2-determinism");

    nlohmann::json variant = parsed.document;
    variant["physicalImage"] = {
        {"encoding", "hex_uppercase_continuous"},
        {"standardSramHex", "DEADBEEF"},
        {"trailingDataHex", ""},
        {"totalLength", 4},
        {"standardSramLength", 4},
        {"trailingLength", 0}
    };
    const auto variantPath = tempDir / "variant.red.json";
    WriteJson(variantPath, variant);

    pkmn::savegen::generation::GenerateRequest first;
    first.inputJsonPath = DummyJsonPath();
    first.templateSavePath = DummySavPath();
    first.profilePath = ProfilePath();
    first.outputSavePath = tempDir / "first.sav";
    first.outputReportPath = tempDir / "first.report.json";

    pkmn::savegen::generation::GenerateRequest second = first;
    second.inputJsonPath = variantPath;
    second.outputSavePath = tempDir / "second.sav";
    second.outputReportPath = tempDir / "second.report.json";

    const auto firstResult = pkmn::savegen::generation::MinimalSaveGenerator::Generate(first);
    const auto secondResult = pkmn::savegen::generation::MinimalSaveGenerator::Generate(second);
    CHECK(firstResult.outputBytes == secondResult.outputBytes);
}

TEST_CASE("Milestone 3 generation rejects unsupported safe locations and output collisions") {
    const auto parsed = pkmn::savegen::input::RedJsonReader::ReadFromFile(DummyJsonPath());
    const auto tempDir = MakeTempDir("milestone2-reject");

    nlohmann::json badLocation = parsed.document;
    badLocation["decoded"]["location"]["map"]["id"] = 1;
    badLocation["decoded"]["location"]["x"]["value"] = 1;
    badLocation["decoded"]["location"]["y"]["value"] = 1;
    const auto badLocationPath = tempDir / "bad-location.red.json";
    WriteJson(badLocationPath, badLocation);

    pkmn::savegen::generation::GenerateRequest request;
    request.inputJsonPath = badLocationPath;
    request.templateSavePath = DummySavPath();
    request.profilePath = ProfilePath();
    request.outputSavePath = tempDir / "bad-location.sav";
    request.outputReportPath = tempDir / "bad-location.report.json";
    CHECK_THROWS(pkmn::savegen::generation::MinimalSaveGenerator::Generate(request));

    const auto existingOutput = tempDir / "existing.sav";
    {
        std::ofstream out(existingOutput);
        REQUIRE(out.good());
        out << "already here";
    }

    request.inputJsonPath = DummyJsonPath();
    request.outputSavePath = existingOutput;
    request.outputReportPath = tempDir / "existing.report.json";
    CHECK_THROWS(pkmn::savegen::generation::MinimalSaveGenerator::Generate(request));
}

TEST_CASE("Milestone 3 generation overwrites dummy-owned fields and avoids contamination") {
    const auto parsed = pkmn::savegen::input::RedJsonReader::ReadFromFile(DummyJsonPath());
    const auto tempDir = MakeTempDir("milestone3-contamination");

    nlohmann::json mutated = parsed.document;
    mutated["decoded"]["trainer"]["name"]["value"] = "ASH";
    mutated["decoded"]["rival"]["name"]["value"] = "GARY";
    mutated["decoded"]["trainer"]["trainerId"]["value"] = 12345;
    mutated["decoded"]["options"]["optionsByte"]["value"] = 90;
    mutated["decoded"]["options"]["letterDelayByte"]["value"] = 3;
    mutated["decoded"]["options"]["contrast"]["value"] = 2;
    mutated["decoded"]["moneyAndCoins"]["money"]["value"] = 654321;
    mutated["decoded"]["moneyAndCoins"]["coins"]["value"] = 321;
    mutated["decoded"]["badges"]["rawBitfield"] = "0xA5";

    for (auto& species : mutated["decoded"]["pokedex"]["species"]) {
        species["owned"] = false;
        species["seen"] = false;
    }
    mutated["decoded"]["pokedex"]["species"][0]["owned"] = true;
    mutated["decoded"]["pokedex"]["species"][0]["seen"] = true;
    mutated["decoded"]["pokedex"]["species"][24]["seen"] = true;
    mutated["decoded"]["pokedex"]["species"][150]["owned"] = true;
    mutated["decoded"]["pokedex"]["species"][150]["seen"] = true;
    mutated["decoded"]["pokedex"]["ownedCount"] = 2;
    mutated["decoded"]["pokedex"]["seenCount"] = 3;

    mutated["decoded"]["inventory"]["bag"]["count"] = 2;
    mutated["decoded"]["inventory"]["bag"]["items"] = nlohmann::json::array({
        {
            {"slot", 1},
            {"item", {{"name", "POTION"}, {"id", 20}, {"hex", "0x14"}}},
            {"quantity", 5}
        },
        {
            {"slot", 2},
            {"item", {{"name", "ANTIDOTE"}, {"id", 11}, {"hex", "0x0B"}}},
            {"quantity", 2}
        }
    });
    mutated["decoded"]["inventory"]["pcItemStorage"]["count"] = 2;
    mutated["decoded"]["inventory"]["pcItemStorage"]["items"] = nlohmann::json::array({
        {
            {"slot", 1},
            {"item", {{"name", "POKE BALL"}, {"id", 4}, {"hex", "0x04"}}},
            {"quantity", 10}
        },
        {
            {"slot", 2},
            {"item", {{"name", "ESCAPE ROPE"}, {"id", 29}, {"hex", "0x1D"}}},
            {"quantity", 1}
        }
    });

    for (auto& entry : mutated["decoded"]["visitedTowns"]) {
        entry["visited"] = false;
    }
    mutated["decoded"]["visitedTowns"][0]["visited"] = true;
    mutated["decoded"]["visitedTowns"][2]["visited"] = true;
    mutated["decoded"]["visitedTowns"][10]["visited"] = true;

    for (auto& entry : mutated["decoded"]["hiddenItems"]) {
        entry["collected"] = false;
    }
    mutated["decoded"]["hiddenItems"][1]["collected"] = true;
    mutated["decoded"]["hiddenItems"][6]["collected"] = true;

    for (auto& entry : mutated["decoded"]["hiddenCoins"]) {
        entry["collected"] = false;
    }
    mutated["decoded"]["hiddenCoins"][0]["collected"] = true;
    mutated["decoded"]["hiddenCoins"][11]["collected"] = true;

    const auto inputPath = tempDir / "mutated.red.json";
    WriteJson(inputPath, mutated);

    pkmn::savegen::generation::GenerateRequest request;
    request.inputJsonPath = inputPath;
    request.templateSavePath = DummySavPath();
    request.profilePath = ProfilePath();
    request.outputSavePath = tempDir / "generated.sav";
    request.outputReportPath = tempDir / "generated.report.json";

    const auto result = pkmn::savegen::generation::MinimalSaveGenerator::Generate(request);
    const auto templateLoaded =
        pkmn::savegen::template_loader::CanonicalTemplateLoader::Load(DummySavPath());

    CHECK(pkmn::savegen::encoding::Gen1TextEncoder::DecodeName(
              result.outputBytes,
              pkmn::savegen::encoding::Gen1Layout::TrainerNameOff,
              pkmn::savegen::encoding::Gen1Layout::TrainerNameLen) == "ASH");
    CHECK(pkmn::savegen::encoding::Gen1TextEncoder::DecodeName(
              result.outputBytes,
              pkmn::savegen::encoding::Gen1Layout::RivalNameOff,
              pkmn::savegen::encoding::Gen1Layout::RivalNameLen) == "GARY");
    CHECK(pkmn::savegen::encoding::PrimitiveWriter::ReadU16BigEndian(
              result.outputBytes,
              pkmn::savegen::encoding::Gen1Layout::TrainerIdOff) == 12345);
    CHECK(result.outputBytes[pkmn::savegen::encoding::Gen1Layout::OptionsOff] == 90);
    CHECK(result.outputBytes[pkmn::savegen::encoding::Gen1Layout::LetterDelayOff] == 3);
    CHECK(result.outputBytes[pkmn::savegen::encoding::Gen1Layout::ContrastOff] == 2);
    CHECK(pkmn::savegen::encoding::BcdEncoder::Decode3(
              result.outputBytes,
              pkmn::savegen::encoding::Gen1Layout::MoneyOff) == 654321U);
    CHECK(pkmn::savegen::encoding::BcdEncoder::Decode2(
              result.outputBytes,
              pkmn::savegen::encoding::Gen1Layout::CoinsOff) == 321U);
    CHECK(result.outputBytes[pkmn::savegen::encoding::Gen1Layout::BadgesOff] == 0xA5);
    CHECK(result.outputBytes[pkmn::savegen::encoding::Gen1Layout::BadgesMirrorOff] == 0xA5);

    const auto ownedBits = ReadBits(result.outputBytes,
                                    pkmn::savegen::encoding::Gen1Layout::PokedexOwnedOff,
                                    151);
    const auto seenBits = ReadBits(result.outputBytes,
                                   pkmn::savegen::encoding::Gen1Layout::PokedexSeenOff,
                                   151);
    CHECK(ownedBits[0]);
    CHECK_FALSE(ownedBits[24]);
    CHECK(ownedBits[150]);
    CHECK(seenBits[0]);
    CHECK(seenBits[24]);
    CHECK(seenBits[150]);

    const auto bagItems = ReadItemList(result.outputBytes,
                                       pkmn::savegen::encoding::Gen1Layout::BagItemsCountOff,
                                       pkmn::savegen::encoding::Gen1Layout::BagItemsPairsOff,
                                       pkmn::savegen::encoding::Gen1Layout::BagItemsMaxPairs);
    REQUIRE(bagItems.size() == 2);
    CHECK(bagItems[0].id == 20);
    CHECK(bagItems[0].quantity == 5);
    CHECK(bagItems[1].id == 11);
    CHECK(bagItems[1].quantity == 2);

    const auto pcItems = ReadItemList(result.outputBytes,
                                      pkmn::savegen::encoding::Gen1Layout::PCItemBoxCountOff,
                                      pkmn::savegen::encoding::Gen1Layout::PCItemBoxPairsOff,
                                      pkmn::savegen::encoding::Gen1Layout::PCItemBoxMaxPairs);
    REQUIRE(pcItems.size() == 2);
    CHECK(pcItems[0].id == 4);
    CHECK(pcItems[0].quantity == 10);
    CHECK(pcItems[1].id == 29);
    CHECK(pcItems[1].quantity == 1);

    const auto visitedTowns = ReadBits(result.outputBytes,
                                       pkmn::savegen::encoding::Gen1Layout::VisitedTownsOff,
                                       pkmn::savegen::encoding::Gen1Layout::VisitedTownsUsedBits);
    const auto hiddenItems = ReadBits(result.outputBytes,
                                      pkmn::savegen::encoding::Gen1Layout::HiddenItemsOff,
                                      pkmn::savegen::encoding::Gen1Layout::HiddenItemsUsedBits);
    const auto hiddenCoins = ReadBits(result.outputBytes,
                                      pkmn::savegen::encoding::Gen1Layout::HiddenCoinsOff,
                                      pkmn::savegen::encoding::Gen1Layout::HiddenCoinsUsedBits);
    CHECK(visitedTowns[0]);
    CHECK(visitedTowns[2]);
    CHECK(visitedTowns[10]);
    CHECK(hiddenItems[1]);
    CHECK(hiddenItems[6]);
    CHECK(hiddenCoins[0]);
    CHECK(hiddenCoins[11]);

    CHECK(result.outputBytes[pkmn::savegen::encoding::Gen1Layout::TrainerNameOff] !=
          templateLoaded.bytes[pkmn::savegen::encoding::Gen1Layout::TrainerNameOff]);
    CHECK(result.outputBytes[pkmn::savegen::encoding::Gen1Layout::BagItemsCountOff] !=
          templateLoaded.bytes[pkmn::savegen::encoding::Gen1Layout::BagItemsCountOff]);
    CHECK(result.outputBytes[pkmn::savegen::encoding::Gen1Layout::PCItemBoxCountOff] !=
          templateLoaded.bytes[pkmn::savegen::encoding::Gen1Layout::PCItemBoxCountOff]);
}

TEST_CASE("Milestone 3 generation rejects unsupported deferred states and invalid inventory") {
    const auto parsed = pkmn::savegen::input::RedJsonReader::ReadFromFile(DummyJsonPath());
    const auto tempDir = MakeTempDir("milestone3-reject");

    auto make_request = [&](const std::filesystem::path& inputPath,
                            const std::string& baseName) {
        pkmn::savegen::generation::GenerateRequest request;
        request.inputJsonPath = inputPath;
        request.templateSavePath = DummySavPath();
        request.profilePath = ProfilePath();
        request.outputSavePath = tempDir / (baseName + ".sav");
        request.outputReportPath = tempDir / (baseName + ".report.json");
        return request;
    };

    nlohmann::json withParty = parsed.document;
    withParty["decoded"]["party"]["count"] = 1;
    const auto withPartyPath = tempDir / "with-party.red.json";
    WriteJson(withPartyPath, withParty);
    CHECK_THROWS(pkmn::savegen::generation::MinimalSaveGenerator::Generate(
        make_request(withPartyPath, "with-party")));

    nlohmann::json withDaycare = parsed.document;
    withDaycare["decoded"]["daycare"]["inUse"] = true;
    const auto withDaycarePath = tempDir / "with-daycare.red.json";
    WriteJson(withDaycarePath, withDaycare);
    CHECK_THROWS(pkmn::savegen::generation::MinimalSaveGenerator::Generate(
        make_request(withDaycarePath, "with-daycare")));

    nlohmann::json withHall = parsed.document;
    withHall["decoded"]["hallOfFame"]["entryCount"] = 1;
    const auto withHallPath = tempDir / "with-hall.red.json";
    WriteJson(withHallPath, withHall);
    CHECK_THROWS(pkmn::savegen::generation::MinimalSaveGenerator::Generate(
        make_request(withHallPath, "with-hall")));

    nlohmann::json duplicateBag = parsed.document;
    duplicateBag["decoded"]["inventory"]["bag"]["count"] = 2;
    duplicateBag["decoded"]["inventory"]["bag"]["items"] = nlohmann::json::array({
        {
            {"slot", 1},
            {"item", {{"name", "POTION"}, {"id", 20}, {"hex", "0x14"}}},
            {"quantity", 1}
        },
        {
            {"slot", 2},
            {"item", {{"name", "POTION"}, {"id", 20}, {"hex", "0x14"}}},
            {"quantity", 2}
        }
    });
    const auto duplicateBagPath = tempDir / "duplicate-bag.red.json";
    WriteJson(duplicateBagPath, duplicateBag);
    CHECK_THROWS(pkmn::savegen::generation::MinimalSaveGenerator::Generate(
        make_request(duplicateBagPath, "duplicate-bag")));
}
