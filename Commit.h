// Commit.h
// Declares the CommitNode class, representing a commit object in MiniGit.

#ifndef COMMIT_H
#define COMMIT_H

#include <string>           // For std::string
#include <vector>           // For std::vector (for parent hashes)
#include <unordered_map>    // For std::unordered_map (for file blobs snapshot)
#include <ctime>            // For time_t, tm, localtime, strftime (for timestamp)
#include <iomanip>          // For std::put_time

/**
 * @brief The CommitNode class represents a single commit in the MiniGit repository.
 * It contains metadata about the commit and a snapshot of the files it tracks.
 */
class CommitNode {
public:
    // Default constructor
    CommitNode();

    /**
     * @brief Constructor for creating a new CommitNode.
     * @param message The commit message.
     * @param parentHashes A vector of hashes of the parent commits (empty for initial commit).
     * @param fileBlobs A map representing the file snapshot (filename to blob hash).
     */
    CommitNode(const std::string& message,
               const std::vector<std::string>& parentHashes,
               const std::unordered_map<std::string, std::string>& fileBlobs);

    /**
     * @brief Calculates the hash of the commit object based on its content (metadata + fileBlobs).
     * This hash uniquely identifies the commit.
     * @return The hash of the commit as a string.
     */
    std::string calculateHash() const;

    /**
     * @brief Serializes the CommitNode object into a string format for storage.
     * @return A string representation of the commit object.
     */
    std::string serialize() const;

    /**
     * @brief Deserializes a string content into a CommitNode object.
     * This is a static method as it creates a CommitNode instance from raw data.
     * @param content The string content representing a serialized commit.
     * @return A CommitNode object parsed from the content.
     * @throws std::runtime_error if parsing fails.
     */
    static CommitNode deserialize(const std::string& content);

    // Getters for CommitNode properties
    const std::string& getHash() const { return hash; }
    const std::vector<std::string>& getParentHashes() const { return parentHashes; }
    const std::string& getMessage() const { return message; }
    const std::string& getTimestamp() const { return timestamp; }
    const std::unordered_map<std::string, std::string>& getFileBlobs() const { return fileBlobs; }

private:
    std::string hash;                       // The unique hash of this commit
    std::vector<std::string> parentHashes; // Hashes of parent commits
    std::string message;                    // Commit message
    std::string timestamp;                  // Timestamp of the commit (e.g., ISO 8601 format)
    // Snapshot of the repository at this commit:
    // Key: filename (path relative to repo root)
    // Value: blob hash of the file's content
    std::unordered_map<std::string, std::string> fileBlobs;

    /**
     * @brief Generates the current timestamp in a standardized format.
     * @return A string representing the current timestamp.
     */
    std::string generateTimestamp() const;
};

#endif // COMMIT_H
