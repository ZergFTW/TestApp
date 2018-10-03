#include "DirectoryReader.h"

using namespace std;
namespace Fs = std::experimental::filesystem;


size_t DirectoryReader::appendDirectoryFiles(const std::string& directoryPathString) {

    const Fs::path directoryPath(directoryPathString);

    if (!exists(directoryPath)) {
        throw Fs::filesystem_error("Can't read directory", directoryPath, make_error_code(errc::no_such_file_or_directory));
    }
    if (!is_directory(directoryPath)) {
        throw Fs::filesystem_error("Can't read directory", directoryPath, make_error_code(errc::not_a_directory));
    }

    size_t addedFilesCount = 0;

    if (experimental::filesystem::is_empty(directoryPath)) {
        // it is not an error, if directory is empty
        return addedFilesCount;
    }

    for (const auto & p : Fs::directory_iterator(directoryPath)) {

        error_code e;// temp var to discard errors

        //just skip file if something wrong;
        if (!is_regular_file(p, e) || experimental::filesystem::is_empty(p, e)) {
            continue;
        }

        mMutex.lock();
        mFilesPaths.push_back(p);
        ++addedFilesCount; 
        mMutex.unlock();
    }

    return addedFilesCount;
}


bool DirectoryReader::getNextFile(Fs::path& nextFilePath) {

    lock_guard<mutex>lock(mMutex);

    if (mFilesPaths.empty()) {
        return false;
    }

    nextFilePath = mFilesPaths.back();
    mFilesPaths.pop_back();

    return true;
}
