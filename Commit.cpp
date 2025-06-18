// Commit.cpp
// Implements the CommitNode class, handling commit object creation,
// serialization, deserialization, and hash calculation.

#include "Commit.h"     // Include the header for CommitNode
#include "Hashing.h"    // For Hashing::calculateHash
#include <sstream>      // For std::stringstream
#include <algorithm>    // For std::sort

// --------------------------------------------------------------------------
// Helper function for timestamp generation
// --------------------------------------------------------------------------

/**
 * @brief Generates the current timestamp in ISO 8601 format (YYYY-MM-DDTHH:MM:SS).
 * This ensures a consistent and readable timestamp for commit metadata.
 * @return A string representing the current timestamp.
 */
std::string CommitNode::generateTimestamp() const {
    std::time_t now = std::time(nullptr);
    std::tm tm_struct;

    // Use platform-specific safe version of localtime
#ifdef _WIN32
    localtime_s(&tm_struct, &now);
#else
    localtime_r(&now, &tm_struct);
#endif

    std::stringstream ss;
    // Format: YYYY-MM-DDTHH:MM:SS
    ss << std::put_time(&tm_struct, "%Y-%m-%dT%H:%M:%S");
    return ss.str();
}

// --------------------------------------------------------------------------
// Constructors
// --------------------------------------------------------------------------

/**
 * @brief Default constructor for CommitNode.
 * Initializes an empty commit. Useful for deserialization or creating a blank object.
 */
CommitNode::CommitNode() : message(""), timestamp(""), hash("") {
    // Default constructor, members initialized to empty/default values
}

/**
 * @brief Constructor for creating a new CommitNode.
 * Sets the message, parent hashes, and file snapshot.
 * The timestamp is generated automatically.
 * The commit's hash is calculated upon construction.
 *
 * @param message The commit message.
 * @param parentHashes A vector of hashes of the parent commits (empty for initial commit).
 * @param fileBlobs A map representing the file snapshot (filename to blob hash).
 */
CommitNode::CommitNode(const std::string& message,
                       const std::vector<std::string>& parentHashes,
                       const std::unordered_map<std::string, std::string>& fileBlobs)
    : message(message), parentHashes(parentHashes), fileBlobs(fileBlobs) {
    this->timestamp = generateTimestamp(); // Set timestamp upon creation
    this->hash = calculateHash();          // Calculate hash immediately
}

// --------------------------------------------------------------------------
// Hash Calculation
// --------------------------------------------------------------------------

/**
 * @brief Calculates the hash of the commit object based on its content.
 * The content includes metadata (message, timestamp, parent hashes) and
 * the sorted list of fileblobs (filename:blobhash). Sorting is crucial
 * to ensure that the hash is consistent regardless of map iteration order.
 * @return The hash of the commit as a string.
 */
std::string CommitNode::calculateHash() const {
    std::stringstream contentToHash;

    contentToHash << "commit\n";
    contentToHash << "message:" << message << "\n";
    contentToHash << "timestamp:" << timestamp << "\n";

    // Add parent hashes
    for (const std::string& parent : parentHashes) {
        contentToHash << "parent:" << parent << "\n";
    }

    // Sort fileBlobs by filename to ensure consistent hash generation
    // Create a vector of pairs from the unordered_map to sort it
    std::vector<std::pair<std::string, std::string>> sortedFileBlobs(fileBlobs.begin(), fileBlobs.end());
    std::sort(sortedFileBlobs.begin(), sortedFileBlobs.end(),
              [](const auto& a, const auto& b) {
                  return a.first < b.first;
              });

    // Add file blobs snapshot
    contentToHash << "tree:\n"; // Or similar identifier
    for (const auto& entry : sortedFileBlobs) {
        contentToHash << "  " << entry.first << " " << entry.second << "\n";
    }

    return Hashing::calculateHash(contentToHash.str());
}

// --------------------------------------------------------------------------
// Serialization and Deserialization
// --------------------------------------------------------------------------

/**
 * @brief Serializes the CommitNode object into a string format for storage.
 * The format includes metadata and file snapshot, delimited by newlines.
 * Example format:
 * type:commit
 * hash:<commit_hash>
 * message:<commit_message>
 * timestamp:<timestamp>
 * parent:<parent_hash_1>
 * parent:<parent_hash_2> (if merge commit)
 * file:<filename1> <blob_hash1>
 * file:<filename2> <blob_hash2>
 * ...
 * @return A string representation of the commit object.
 */
std::string CommitNode::serialize() const {
    std::stringstream ss;
    ss << "type:commit\n";
    ss << "hash:" << hash << "\n"; // Include hash directly for verification on deserialize
    ss << "message:" << message << "\n";
    ss << "timestamp:" << timestamp << "\n";
    for (const std::string& parent : parentHashes) {
        ss << "parent:" << parent << "\n";
    }

    // Sort fileBlobs by filename for consistent serialization
    std::vector<std::pair<std::string, std::string>> sortedFileBlobs(fileBlobs.begin(), fileBlobs.end());
    std::sort(sortedFileBlobs.begin(), sortedFileBlobs.end(),
              [](const auto& a, const auto& b) {
                  return a.first < b.first;
              });

    for (const auto& entry : sortedFileBlobs) {
        ss << "file:" << entry.first << " " << entry.second << "\n";
    }
    return ss.str();
}

/**
 * @brief Deserializes a string content into a CommitNode object.
 * Parses the string content line by line to reconstruct the commit object.
 * @param content The string content representing a serialized commit.
 * @return A CommitNode object parsed from the content.
 * @throws std::runtime_error if the content format is invalid.
 */
CommitNode CommitNode::deserialize(const std::string& content) {
    CommitNode commit; // Create a temporary commit object to populate
    std::stringstream ss(content);
    std::string line;

    std::vector<std::string> deserializedParentHashes;
    std::unordered_map<std::string, std::string> deserializedFileBlobs;

    while (std::getline(ss, line)) {
        if (line.rfind("type:", 0) == 0) { // check if line starts with "type:"
            if (line.substr(5) != "commit") {
                throw std::runtime_error("Deserialization error: Expected type:commit, got " + line.substr(5));
            }
        } else if (line.rfind("hash:", 0) == 0) {
            commit.hash = line.substr(5);
        } else if (line.rfind("message:", 0) == 0) {
            commit.message = line.substr(8);
        } else if (line.rfind("timestamp:", 0) == 0) {
            commit.timestamp = line.substr(10);
        } else if (line.rfind("parent:", 0) == 0) {
            deserializedParentHashes.push_back(line.substr(7));
        } else if (line.rfind("file:", 0) == 0) {
            std::string fileEntry = line.substr(5);
            size_t spacePos = fileEntry.find(' ');
            if (spacePos == std::string::npos) {
                throw std::runtime_error("Deserialization error: Invalid file entry format in commit.");
            }
            std::string filename = fileEntry.substr(0, spacePos);
            std::string blobHash = fileEntry.substr(spacePos + 1);
            deserializedFileBlobs[filename] = blobHash;
        } else {
            // Ignore unknown lines or empty lines for robustness
            // std::cerr << "Warning: Unknown line in commit deserialization: " << line << std::endl;
        }
    }

    commit.parentHashes = deserializedParentHashes;
    commit.fileBlobs = deserializedFileBlobs;

    // Optional: Verify the deserialized hash matches the content's calculated hash
    // if (commit.hash != commit.calculateHash()) {
    //     throw std::runtime_error("Deserialization error: Commit hash mismatch. Data might be corrupted.");
    // }

    return commit;
}
