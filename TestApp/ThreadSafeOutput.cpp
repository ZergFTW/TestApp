#include "ThreadSafeOutput.h"

#include <iostream>

using namespace std;

mutex ThreadSafeOutput::mMutex;

ThreadSafeOutput::ThreadSafeOutput(const ThreadSafeOutput::Stream& outputStream) :
    mOutputStream(outputStream){
}

ThreadSafeOutput::~ThreadSafeOutput() {
    writeToStream(); /// writes at destruction
}

void ThreadSafeOutput::writeToStream() const noexcept{
    std::lock_guard<mutex>lock(ThreadSafeOutput::mMutex);
    try {
        getStream() << this->str();
    }
    catch (const ios_base::failure& err) {
        getStream().clear();
        std::cerr << "Output error: " << err.what() << " " << err.code() << endl;
    }
    catch (const std::runtime_error& err)
    {
        std::cerr << "Runtime error during the output: " << err.what() << std::endl;
    }
    catch (const std::exception& err)
    {
        std::cerr << "Error during the output: " << err.what() << std::endl;
    }
    catch (...) {
        std::cerr << "Unknown error occured during the output" << endl;
    }
}

std::ostream& ThreadSafeOutput::getStream() const{
    if (mOutputStream == Stream::StandardOutput) {
        return cout;
    }
    else {
        return cerr;
    }
}
