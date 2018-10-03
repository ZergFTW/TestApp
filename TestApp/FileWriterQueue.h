#pragma once
#include <vector>
#include <deque>
#include <mutex>
#include <fstream>
#include <atomic>

/// thread safe queue of results
class FileWriterQueue
{
public:
    FileWriterQueue() = default;
    ~FileWriterQueue() = default;

    /// opens file for output
    /// throws filesystem error with explanation if something goes wrong
    void openOutputFile(const std::string& outputPathString);

    /// Appends new results to queue
    void appendResults(const std::string& filename, const std::vector<std::string>& results);

    /// Writes results infinitely, throws if something wrong
    void writeResults();
    void stopWriting();


    FileWriterQueue(const FileWriterQueue&) = delete;
    FileWriterQueue(FileWriterQueue&&) = delete;
    FileWriterQueue& operator=(FileWriterQueue&&) = delete;
    FileWriterQueue& operator=(const FileWriterQueue&&) = delete;

private:
    /// Returns false if container is empty
    bool writeNext();

    std::ofstream mOutputFile;
    std::deque<std::pair<std::string, std::vector<std::string>>> mResults;
    std::mutex mMutex;

    std::atomic_bool running = false;
};

