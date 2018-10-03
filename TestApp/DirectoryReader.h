#pragma once

#include <string>
#include <experimental/filesystem>
#include <vector>
#include <mutex>

/// Thread-safe class for reading directory content.
/// Saves file paths of not empty files, found in the directory.
class DirectoryReader
{
public:
    DirectoryReader() = default;
    virtual ~DirectoryReader() = default;

    /// Reads directory content, appends all non-empty files paths for further usage.
    /// Return number of saved paths.
    /// Throws filesystem_error with explanation if something goes wrong.
    size_t appendDirectoryFiles(const std::string& directoryPathString);
    
    /// Tries to get next path from internal container.
    /// Removes returned value from internal container.
    /// Returns false if container is empty.
    bool getNextFile(std::experimental::filesystem::path& nextFilePath);

    
    DirectoryReader(const DirectoryReader&) = delete;
    DirectoryReader(DirectoryReader&&) = delete;
    DirectoryReader& operator=(const DirectoryReader&) = delete;
    DirectoryReader& operator=(const DirectoryReader&&) = delete;

private:
    std::vector<std::experimental::filesystem::path> mFilesPaths;
    std::mutex mMutex;
};

