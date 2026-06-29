#pragma once

#include <CommonCrypto/CommonDigest.h>

#include <cstdint>
#include <iomanip>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

namespace pkmn::savegen::template_support {

class Sha256 {
public:
    static std::string Hex(const std::vector<std::uint8_t>& bytes) {
        unsigned char digest[CC_SHA256_DIGEST_LENGTH];
        CC_SHA256(bytes.data(), static_cast<CC_LONG>(bytes.size()), digest);
        std::ostringstream oss;
        oss << std::hex << std::setfill('0');
        for (unsigned char byte : digest) {
            oss << std::setw(2) << static_cast<int>(byte);
        }
        return oss.str();
    }

    static std::string Hex(std::string_view text) {
        unsigned char digest[CC_SHA256_DIGEST_LENGTH];
        CC_SHA256(text.data(), static_cast<CC_LONG>(text.size()), digest);
        std::ostringstream oss;
        oss << std::hex << std::setfill('0');
        for (unsigned char byte : digest) {
            oss << std::setw(2) << static_cast<int>(byte);
        }
        return oss.str();
    }
};

}  // namespace pkmn::savegen::template_support
