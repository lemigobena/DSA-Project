// Hashing.cpp
// Implements the hashing utility functions declared in Hashing.h.

#include "Hashing.h" // Include the header for this module
#include <sstream>   // For std::stringstream to format hash output
#include <iomanip>   // For std::hex, std::setw, std::setfill

namespace Hashing {

    /**
     * @brief Calculates a simple (non-cryptographic) hash of a given string content.
     * This uses a FNV-1a like algorithm to generate a hash.
     * While simple, it's sufficient for demonstrating the concept of content addressing.
     * For production, a cryptographic hash (like SHA-1) would be necessary for integrity.
     *
     * @param content The string content to hash.
     * @return A string representing the calculated hash (e.g., "a1b2c3d4...").
     */
    std::string calculateHash(const std::string& content) {
        // FNV-1a parameters (32-bit for simplicity, can be extended to 64-bit)
        const unsigned int FNV_PRIME = 16777619U;
        const unsigned int FNV_OFFSET_BASIS = 2166136261U;

        unsigned int hash = FNV_OFFSET_BASIS;
        for (char c : content) {
            hash ^= static_cast<unsigned char>(c); // XOR with the current byte
            hash *= FNV_PRIME;                   // Multiply by the FNV prime
        }

        // Convert the unsigned int hash to a hexadecimal string.
        // This makes the hash look more like a Git hash (e.g., "a1b2c3d4").
        std::stringstream ss;
        ss << std::hex << std::setw(8) << std::setfill('0') << hash;
        return ss.str();
    }

} // namespace Hashing
