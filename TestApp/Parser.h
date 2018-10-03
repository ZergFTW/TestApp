#pragma once

#include <string>
#include <vector>
#include <experimental/filesystem>
#include <future>

#include "DirectoryReader.h"
#include "FileWriterQueue.h"


class Parser
{

public:
    Parser() = default;
    virtual ~Parser() = default;

    bool readDirectory(const std::string& inputDirectory);
    bool openOutputFile(const std::string& outputFile);

    void startParsing(unsigned numberOfThreads);

    Parser(const Parser&) = delete;
    Parser(Parser&&) = delete;
    Parser& operator=(const Parser&) = delete;
    Parser& operator=(const Parser&&) = delete;



private:
    void parse();
    void cycleInputThreads();
    bool isOutputThreadFinished();

    static std::vector<std::string> readAndSplitFile(const std::experimental::filesystem::path& inputFilePath);
    static std::vector<std::basic_string<char>> multiCharStringSplit(const std::string& input,
        std::vector<std::basic_string<char>>& delimiters);

    DirectoryReader mInputFiles;
    FileWriterQueue mResults;

    std::vector<std::future<void>> mInputThreadsResults;
    std::future<void> mOutputThreadResult;
};