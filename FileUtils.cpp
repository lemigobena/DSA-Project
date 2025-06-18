#include "FileUtils.h"
#include <fstream>
#include <stdexcept>
#include <filesystem>
#include <iostream>
#include <sstream>

// Uncomment the line below to enable debug prints in this file
// #define DEBUG_MODE

namespace fs = std::filesystem;

static bool ensureParentDirectoryExists(const std::string& filePath) {
    fs::path p(filePath);
    fs::path parentDir = p.parent_path();
    if (!parentDir.empty() && !fs::exists(parentDir)) {
        #ifdef DEBUG_MODE
        std::cout << "DEBUG: ensureParentDirectoryExists: Creating parent directory: " << parentDir << std::endl;
        #endif
        std::error_code ec;
        if (fs::create_directories(parentDir, ec)) {
            return true;
        } else {
            std::cerr << "ERROR: ensureParentDirectoryExists: Failed to create parent directory '" << parentDir << "': " << ec.message() << std::endl;
            return false;
        }
    }
    return true;
}

bool FileUtils::writeToFile(const std::string& filename, const std::string& content) {
    #ifdef DEBUG_MODE
    std::cout << "DEBUG: writeToFile called for: '" << filename << "'" << std::endl;
    std::cout << "DEBUG: writeToFile content length: " << content.length() << " bytes." << std::endl;
    // std::cout << "DEBUG: writeToFile content (first 50 chars): '" << content.substr(0, std::min((size_t)50, content.length())) << "...'" << std::endl;
    #endif

    if (!ensureParentDirectoryExists(filename)) {
        std::cerr << "ERROR: writeToFile: Failed to ensure parent directory exists for " << filename << std::endl;
        return false;
    }

    std::ofstream outFile(filename, std::ios::binary);
    if (!outFile.is_open()) {
        std::cerr << "ERROR: writeToFile: Failed to open file for writing: '" << filename << "'" << std::endl;
        return false;
    }

    outFile << content;
    outFile.flush(); // Keep this flush for robustness, even without debug mode

    if (outFile.fail()) {
        std::cerr << "ERROR: writeToFile: Failed to write content to file: '" << filename << "'" << std::endl;
        outFile.close();
        return false;
    }

    outFile.close();
    #ifdef DEBUG_MODE
    std::cout << "DEBUG: writeToFile: Successfully wrote to: '" << filename << "'" << std::endl;
    #endif
    return true;
}

bool FileUtils::readFromFile(const std::string& filename, std::string& content) {
    #ifdef DEBUG_MODE
    std::cout << "DEBUG: readFromFile called for: '" << filename << "'" << std::endl;
    #endif
    
    if (!fs::exists(filename)) {
        std::cerr << "ERROR: readFromFile: File does not exist: '" << filename << "'" << std::endl;
        content = "";
        return false;
    }
    if (!fs::is_regular_file(filename)) {
        std::cerr << "ERROR: readFromFile: Path is not a regular file: '" << filename << "'" << std::endl;
        content = "";
        return false;
    }

    std::ifstream inFile(filename, std::ios::binary);
    if (!inFile.is_open()) {
        std::cerr << "ERROR: readFromFile: Failed to open file for reading: '" << filename << "'" << std::endl;
        content = "";
        return false;
    }

    std::stringstream buffer;
    buffer << inFile.rdbuf();
    content = buffer.str();
    inFile.close();

    #ifdef DEBUG_MODE
    std::cout << "DEBUG: readFromFile: Successfully read " << content.length() << " bytes from: '" << filename << "'" << std::endl;
    // std::cout << "DEBUG: readFromFile: Content: '" << content << "'" << std::endl;
    #endif

    if (content.empty() && fs::file_size(filename) > 0) {
        std::cerr << "WARNING: readFromFile: File '" << filename << "' was opened but read as empty despite having content.\n";
    }

    return true;
}

bool FileUtils::directoryExists(const std::string& path) {
    return fs::is_directory(path);
}

bool FileUtils::createDirectory(const std::string& path) {
    #ifdef DEBUG_MODE
    std::cout << "DEBUG: createDirectory called for: " << path << std::endl;
    #endif
    if (fs::exists(path)) {
        #ifdef DEBUG_MODE
        std::cout << "DEBUG: Directory already exists: " << path << std::endl;
        #endif
        return true;
    }
    std::error_code ec;
    if (fs::create_directories(path, ec)) {
        #ifdef DEBUG_MODE
        std::cout << "DEBUG: Directory created: " << path << std::endl;
        #endif
        return true;
    } else {
        std::cerr << "ERROR: createDirectory: Failed to create directory '" << path << "': " << ec.message() << std::endl;
        return false;
    }
}

bool FileUtils::fileExists(const std::string& path) {
    return fs::is_regular_file(path);
}