#include "Parser.h"

#include <fstream>
#include <future>

#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/join.hpp>
#include <boost/algorithm/string/classification.hpp>

#include "ThreadSafeOutput.h"
#include <future>

using namespace std;
namespace Fs = std::experimental::filesystem;


bool Parser::readDirectory(const std::string& inputDirectory) {

    size_t filesCount = 0;
    try {
        filesCount = mInputFiles.appendDirectoryFiles(inputDirectory);
    }
    catch (const experimental::filesystem::filesystem_error &err) {
        TSCerr << "Error! " << err.what() << ": " << err.path1() << endl;
        return false;
    }
    if (filesCount == 0) {
        TSCerr << "No suitable files found in directory: " << inputDirectory << endl;
        return false;
    }

    TSCout << filesCount << " suitable" << (filesCount == 1 ? " file" : " files") 
            << " found in directory " << inputDirectory << endl;
    return true;
}

bool Parser::openOutputFile(const std::string& outputFile) {
    try {
        mResults.openOutputFile(outputFile);
    }
    catch (const Fs::filesystem_error& err) {
        TSCerr << "Error: " << err.what() << " " << err.path1() << endl;
        return false;
    }
    return true;
}

void Parser::startParsing(unsigned numberOfThreads) {

    // tries to get an available number of threads
    if (numberOfThreads == 0) {
        numberOfThreads = thread::hardware_concurrency() ? thread::hardware_concurrency() : 1;
    }

    TSCout << "Parser will start " << numberOfThreads << " threads" << endl;

    // starting threads for input
    for (unsigned i = 0; i < numberOfThreads; ++i) {
        mInputThreadsResults.emplace_back(async(launch::async, &Parser::parse, this));
        TSCout << "Thread #" << i + 1 << " started" << endl;
    }

    // starting thread for output
    mOutputThreadResult = async(launch::async, &FileWriterQueue::writeResults, &mResults);
    

    // checks everything and waits until job is done
    while (true) {
        
        cycleInputThreads(); // will remove finished thread results

        if (mInputThreadsResults.empty()) {
            // all input threads finished - need to stop output
            mResults.stopWriting();
        }

        if (isOutputThreadFinished()) {
            // output finished successfully
            return;
        }

        // sleep for a while
        this_thread::sleep_for(chrono::milliseconds(60));
    }
}

void Parser::parse() {
    Fs::path inputFilePath;

    while (mInputFiles.getNextFile(inputFilePath)){

        vector<string> results;
        TSCout << "Parsing file " << inputFilePath.filename() << endl;
        try {
             results = readAndSplitFile(inputFilePath);
        }
        catch (const Fs::filesystem_error& err) {
            TSCerr << "Can't parse file: " << err.path1() << "\nError: " << err.what() << endl;
            continue;
        }

        mResults.appendResults(inputFilePath.filename().string(), results);
    }
}

/// helper for startParsing
/// checks every input thread
/// removes any successfully finished thread result
/// restart every crashed thread
void Parser::cycleInputThreads() {
    for (size_t i = 0; i < mInputThreadsResults.size(); ++i) {
        if (!mInputThreadsResults[i].valid()) {
            continue;
        }
        try {
            (mInputThreadsResults[i].get());
            TSCout << "Input thread finished successfully" << endl;
            mInputThreadsResults.erase(mInputThreadsResults.begin() + i);
            --i;
            continue;
        }
        // whoops, something goes wrong
        catch (const std::runtime_error& err) {
            TSCerr << "Runtime error in input thread " << err.what() << std::endl;
        }
        catch (const std::exception& err) {
            TSCerr << "Error in input thread #" << i + 1 << ": " << err.what() << std::endl;
        }
        catch (...) {
            TSCerr << "Unknown error in input thread #" << i + 1 << std::endl;
        }
        // restart it
        TSCout << "Restarting thread #" << i + 1 << std::endl;
        mInputThreadsResults.emplace(mInputThreadsResults.begin() + i,
            async(launch::async, &Parser::parse, this));
    }
}

/// helper for startParsing
/// checks output thread
/// return true if it is finished successfully - false if it is still working
/// terminates program if something goes wrong - we cant do anything with output
bool Parser::isOutputThreadFinished() {
    if (mOutputThreadResult.valid()) {
        try {
            mOutputThreadResult.get();
            TSCout << "Done!" << endl; // Everything dumped to output file, Done!
            return true;
        }
        // whoops, something goes wrong
        catch (const std::runtime_error& re) {
            TSCerr << "Runtime error in output thread: " << re.what() << std::endl;
        }
        catch (const std::exception& err) {
            TSCerr << "Error in output thread: " << err.what() << std::endl;
        }
        catch (...) {
            TSCerr << "Unknown error in output thread #" << std::endl;
        }
        exit(-1); // well, if something wrong with output file - we can't do anything
    }
    return false;
}


vector<string> Parser::readAndSplitFile(const Fs::path& inputFilePath) {

    ifstream inputFile(inputFilePath);
    if (!inputFile.is_open()) {
        throw Fs::filesystem_error("Can't read file", inputFilePath, make_error_code(errc::io_error));
    }

    // Put results here
    vector<string> result;

    // First line of the file
    string firstLine;
    getline(inputFile, firstLine);

    // Nothing to parse
    if (firstLine.empty()) {
        return result; ;
    }

    // Reading delimiters
    string delim;
    vector<string> delimiters;
    while (getline(inputFile, delim)) {
        if (delim.empty()) {
            continue;
        }
        delimiters.push_back(delim);
    }

    // No delimiters found, line split is not required
    if (delimiters.empty()) {
        result.push_back(firstLine);
        return result;
    }

    // All delimiters 1 character long, we can use simpler approach
    if (!any_of(delimiters.begin(), delimiters.end(),
        [](const string& s){return s.length() > 1;} )) {
        const string joinedDelimiters(boost::algorithm::join(delimiters, ""));
        boost::split(result, firstLine, boost::algorithm::is_any_of(joinedDelimiters));
        return result;
    }

    // Some (or all) delimiters are longer than one character
    result = multiCharStringSplit(firstLine, delimiters);
    return result;
}


/// Splits string by delimiters even if delimiter(s) contains more than one character.
/// Warning! Sorts original delimiters vector to avoid copying
vector<string> Parser::multiCharStringSplit(const string& input, vector<string>& delimiters) {
    // we could implement tire with dictionary, but I guess it will be overkill.
    // So we will use sorted vector<string> as cheaper,
    // but little bit slower version of a dictionary
    std::sort(delimiters.begin(), delimiters.end());

    vector<string> results;

    size_t currentPos = 0;
    size_t previousCutPos = 0;
    string wantedDelimiter;
    while (currentPos < input.length()) {
        // reset wanted delimiter without memory reallocation
        wantedDelimiter.resize(0);

        // range used for delimiters search - we narrow it, when something found
        pair<vector<string>::iterator, vector<string>::iterator> currentRange(
            delimiters.begin(), delimiters.end());

        // inner loop is used in cases of partial delimiter match
        while (true) {
            wantedDelimiter.push_back(input[currentPos]);

            // search among delimiters
            // there may be more than one delimiter starting with this letter(s),
            // so we search for a range
            // we compare only as many first characters as wantedDelimiter has
            currentRange = equal_range(currentRange.first, currentRange.second, wantedDelimiter,
                [&wantedDelimiter](const string& a, const string& b) {
                return a.compare(0, wantedDelimiter.length(), b, 0, wantedDelimiter.length()) < 0;
            });

            if (currentRange.first == currentRange.second){
                // nothing found, but if there was partial match (in previous iteration) - 
                // we must continue from beginning of that match
                // otherwise we can accidentally skip some delimiters
                // e.g. "hello world" will not be split at "lo" delimiter
                currentPos -= (wantedDelimiter.length() - 1); // go back to partial match begin
                ++currentPos;
                break;
            }
            // partial match found!

            // check for exact match in delimiters
            if (binary_search(currentRange.first, currentRange.second, wantedDelimiter)) {
                // Yay! exact match found! "Cut" string
                ++currentPos;
                results.push_back(input.substr(previousCutPos, currentPos - wantedDelimiter.length() - previousCutPos));
                previousCutPos = currentPos;
                break;
            }

            // no exact match - need to append one char to wanted delimiter and try again
            ++currentPos;

            // end of input, BUT maybe there is a shorter delimiter at the end?
            // e.g. we searched for "lloz" and "lo" in "hello" - without this check
            // whole "hello" will be returned
            if (currentPos >= input.length()) {
                currentPos -= (wantedDelimiter.length() - 1);
                break;
            }
        } // inner loop
    } // outer loop
    // done - now we need to append leftovers
    results.push_back(input.substr(previousCutPos));
    return results;
}
