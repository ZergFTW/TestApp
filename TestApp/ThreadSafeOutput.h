#pragma once

#include <sstream>
#include <mutex>

/// for convenience
#define TSCout ThreadSafeOutput(ThreadSafeOutput::StandardOutput)
#define TSCerr ThreadSafeOutput(ThreadSafeOutput::StandardError)

/// simple thread-safe wrapper for cout/cerr
/// buffers input data, as long as object exists
/// sends everything when destructed
class ThreadSafeOutput: public std::stringstream
{
public:
    enum Stream { StandardOutput, StandardError };

    ThreadSafeOutput(const Stream & outputStream);
    virtual ~ThreadSafeOutput();

private:
    void writeToStream() const noexcept;
    Stream mOutputStream;
    static std::mutex mMutex;
    std::ostream& getStream() const;
};

