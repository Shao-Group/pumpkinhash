#include "pumpkinhash.hpp"
#include "path_utils.hpp"

#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>

int main(int argc, char **argv)
{
    if (argc != 7)
    {
        cerr << "Invalid number of command-line arguments provided!\nCorrect usage: ./pumpkinhash_seeds_generator [dataFileName] [paramD] [numRepeats] [numMaxEditsE] [doGenerateEplus1Seeds] [doUseTablesC]" << endl;

        return 1;
    }

    string dataFileName = argv[1];

    string dataFilePath = pathJoin(pathJoin("..", "data"), dataFileName);

    ifstream dataFile;

    dataFile.open(dataFilePath);

    if (!dataFile.is_open())
    {
        exit(EXIT_FAILURE);
    }

    int paramD = stoi(argv[2]), numRepeats = stoi(argv[3]), numMaxEditsE = stoi(argv[4]);

    bool doGenerateEplus1Seeds = (stoi(argv[5]) == 0) ? false : true, doUseTablesC = (stoi(argv[6]) == 0) ? false : true;

    map<char, int> defaultAlphabet = {{'A', 0}, {'C', 1}, {'G', 2}, {'T', 3}};

    string seedsFileFolderPath = pathJoin("..", "seeds");

    if (!pathExists(seedsFileFolderPath))
    {
        if (!createDirectories(seedsFileFolderPath))
        {
            exit(EXIT_FAILURE);
        }
    }

    string seedsFilePath = pathJoin(seedsFileFolderPath, string("seeds_") + dataFileName + string("_paramD") + to_string(paramD) + string("_numRepeats") + to_string(numRepeats) + string("_numMaxEditsE") + to_string(numMaxEditsE) + string("_doGenerateEplus1Seeds=") + to_string(doGenerateEplus1Seeds) + string("_doUseTablesC=") + to_string(doUseTablesC));

    ofstream seedsFile(seedsFilePath);

    if (!seedsFile.is_open())
    {
        exit(EXIT_FAILURE);
    }

    string sequence;

    cout << "Generating " << numRepeats * (doGenerateEplus1Seeds ? numMaxEditsE + 1 : 1) << " seeds for sequences in " << dataFileName << " file with D = " << paramD << " and at most " << numMaxEditsE << " edits..." << endl;

    vector<string> sequences;

    while (dataFile >> sequence)
    {
        sequences.push_back(sequence);
    }

    dataFile.close();

    vector<string> sequenceOutputs(sequences.size());

#pragma omp parallel for
    for (int sequenceIdx = 0; sequenceIdx < (int) sequences.size(); sequenceIdx++)
    {
        const string &currentSequence = sequences[sequenceIdx];
        int windowSizeN = currentSequence.length();

        PumpkinHash pumpkinHash(windowSizeN, paramD, defaultAlphabet, doUseTablesC);

        ostringstream sequenceOutput;

        sequenceOutput << currentSequence << endl;

        for (int repeat = 1; repeat <= numRepeats; repeat++)
        {
            pumpkinHash.loadTables(repeat);

            vector<Seed> seeds = pumpkinHash.solveDP(currentSequence, numMaxEditsE, doGenerateEplus1Seeds, true);

            for (int seedIdx = 0; seedIdx < (int) seeds.size(); seedIdx++)
            {
                sequenceOutput << repeat << "," << seeds[seedIdx].psi << "," << seeds[seedIdx].omega << "," << seeds[seedIdx].seed << endl;
            }
        }

        sequenceOutputs[sequenceIdx] = sequenceOutput.str();
    }

    for (int sequenceIdx = 0; sequenceIdx < (int) sequences.size(); sequenceIdx++)
    {
        seedsFile << sequenceOutputs[sequenceIdx];
    }

    seedsFile.close();

    cout << "Done!" << endl;

    return 0;
}