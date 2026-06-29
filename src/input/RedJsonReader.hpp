#pragma once

#include <filesystem>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>

#include "../../third_party/nlohmann/json.hpp"

namespace pkmn::savegen::input {

struct ParsedRedJson {
    std::filesystem::path inputPath;
    nlohmann::json document;
    std::size_t inputBytes = 0;
};

class RedJsonReader {
public:
    static ParsedRedJson ReadFromFile(const std::filesystem::path& path) {
        std::ifstream input(path, std::ios::binary);
        if (!input) {
            throw std::runtime_error("failed to open JSON input: " + path.string());
        }
        std::ostringstream buffer;
        buffer << input.rdbuf();
        const std::string text = buffer.str();
        ParsedRedJson result;
        result.inputPath = path;
        result.inputBytes = text.size();
        result.document = nlohmann::json::parse(text);
        return result;
    }
};

}  // namespace pkmn::savegen::input
