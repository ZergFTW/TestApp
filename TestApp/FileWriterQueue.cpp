#include "FileWriterQueue.h"
#include <experimental/filesystem>

using namespace std;
namespace Fs = std::experimental::filesystem;

void FileWriterQueue::openOutputFile(const std::string& outputPathString) {

    if (mOutputFile.is_open()) {
        throw Fs::filesystem_error("Another file is open already", 
            Fs::path(outputPathString), make_error_code(errc::io_error));
    }
    
    mOutputFile.open(outputPathString, ios_base::ate);
    if (!mOutputFile.is_open()) {
        throw Fs::filesystem_error("Can't open file", 
            Fs::path(outputPathString), make_error_code(errc::io_error));
    }
}

void FileWriterQueue::appendResults(const std::string& filename, const std::vector<std::string>& results) {
    lock_guard<mutex>lock(mMutex);
    mResults.emplace_back(pair<string, vector<string>>(filename, results));
}

void FileWriterQueue::writeResults() {
    running = true;
    while (true) {
        if (!writeNext()) {
            if (!running) {
                return;
            }
            this_thread::sleep_for(chrono::milliseconds(50));
        }
    }
}

void FileWriterQueue::stopWriting() {
    running = false;
}


bool FileWriterQueue::writeNext() {
    unique_lock<mutex>lock(mMutex);
    if (mResults.empty()) {
        return false;
    }
    // for thread-safety, first we move result from the container
    auto frontFileResults = move(mResults.front());
    mResults.pop_front();
    // then unlock mutex
    lock.unlock();

    // and now we can safely write it to the output file
    mOutputFile << "[" << frontFileResults.first << "]" << endl;
    for (const auto& line : frontFileResults.second) {
        mOutputFile << line << endl;
    }
    mOutputFile.flush();
    return true;
}
