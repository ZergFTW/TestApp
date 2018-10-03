#include <iostream>
#include <string>

#include "Parser.h"
#include "ThreadSafeOutput.h"

using namespace std;


int main(){
    Parser pars;


    string inputDirectory;
    do {
        TSCout << endl << "Enter the input directory path:" << endl;
        getline(cin, inputDirectory);
    } while (!pars.readDirectory(inputDirectory));


    string outputFile;
    do {
        TSCout << endl << "Enter the output file path:" << endl;
        getline(cin, outputFile);
    } while (!pars.openOutputFile(outputFile));


    TSCout << endl << "Enter the number of parser threads." << endl
        << "Enter 0 or negative number for maximum available threads :";
    int threadsCount = 0;
    cin >> threadsCount;
    while (cin.fail()) {
        TSCout << endl << "Incorrect input! Enter a number: ";
        cin.clear();
        cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

        cin >> threadsCount;
    }

    threadsCount = threadsCount < 0 ? 0 : threadsCount;

    pars.startParsing(threadsCount);

    return 0;
}

