#include <iostream>
#include <string>

#include "Parser.h"
#include "ThreadSafeOutput.h"

using namespace std;


int main(){
    while (true){
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


        TSCout << "\nEnter the number of parser threads." << endl
            << "Enter 0 or negative number for maximum available threads :";
        int threadsCount = 0;
        cin >> threadsCount;
        while (cin.fail()) {
            TSCout << endl << "Incorrect input! Enter a number: ";
            cin.clear();
            cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

            cin >> threadsCount;
        }
        // ignore all extra characters
        cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        // can't be < 0
        threadsCount = threadsCount < 0 ? 0 : threadsCount;

        pars.startParsing(threadsCount); // main function

        // done! should we quit?
        char respond = '\0';
        while (true) {
            TSCout << "\nParse another directory? (Y/N): ";
            cin >> respond;
            respond = tolower(respond);
            if (cin.fail()) {
                cin.clear();
                cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                continue;
            }

            cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            if (respond == 'y') {
                // continue with another directory
                break;
            }
            if (respond == 'n') {
                // quit
                return 0;
            }
        }

    } // global loop
}

