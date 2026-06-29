#pragma once

#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "Sha256.hpp"

namespace pkmn::savegen::template_loader {

struct LoadedTemplate {
    std::filesystem::path path;
    std::vector<std::uint8_t> bytes;
    std::string sha256;
};

class CanonicalTemplateLoader {
public:
    static LoadedTemplate Load(const std::filesystem::path& path) {
        std::ifstream input(path, std::ios::binary | std::ios::ate);
        if (!input) {
            throw std::runtime_error("failed to open template: " + path.string());
        }
        const std::streamsize size = input.tellg();
        if (size < 0) {
            throw std::runtime_error("failed to determine template size");
        }
        std::vector<std::uint8_t> bytes(static_cast<std::size_t>(size));
        input.seekg(0, std::ios::beg);
        if (!bytes.empty()) {
            input.read(reinterpret_cast<char*>(bytes.data()), size);
            if (!input) {
                throw std::runtime_error("failed to read template bytes");
            }
        }
        return {path, bytes, template_support::Sha256::Hex(bytes)};
    }
};

}  // namespace pkmn::savegen::template_loader
