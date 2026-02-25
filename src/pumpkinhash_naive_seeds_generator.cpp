#include "pumpkinhash.hpp"

int main(int argc, char **argv)
{
    if (argc != 5)
    {
        cerr << "Invalid number of command-line arguments provided!\nCorrect usage: ./pumpkinhash_naive_seeds_generator [dataFileName] [paramD] [numTablesFileVersions] [numMaxEditsE]" << endl;

        return 1;
    }

    string dataFileName = argv[1];

    string dataFilePath = string("..") + filesystem::path::preferred_separator + string("data") + filesystem::path::preferred_separator + dataFileName;

    ifstream dataFile;

    dataFile.open(dataFilePath);

    if (!dataFile.is_open())
    {
        exit(EXIT_FAILURE);
    }

    int paramD = stoi(argv[2]), numTablesFileVersions = stoi(argv[3]), numMaxEditsE = stoi(argv[4]);

    map<char, int> defaultAlphabet = {{'A', 0}, {'C', 1}, {'G', 2}, {'T', 3}};

    string seedsFileFolderPath = string("..") + filesystem::path::preferred_separator + string("seeds");

    filesystem::path seedsFileFolder(seedsFileFolderPath);

    if (!filesystem::exists(seedsFileFolder))
    {
        if (!filesystem::create_directories(seedsFileFolder))
        {
            exit(EXIT_FAILURE);
        }
    }

    string seedsFilePath = seedsFileFolderPath + filesystem::path::preferred_separator + string("seeds_") + dataFileName;

    ofstream seedsFile(seedsFilePath);

    if (!seedsFile.is_open())
    {
        exit(EXIT_FAILURE);
    }

    string sequence;

    cout << "Generating " << numTablesFileVersions << " seeds for sequences in " << dataFileName << " file with D = " << paramD << " and at most " << numMaxEditsE << " edits..." << endl;

    while (dataFile >> sequence)
    {
        int windowSizeN = sequence.length();

        PumpkinHash pumpkinHash(windowSizeN, paramD, defaultAlphabet);

        seedsFile << sequence << endl;

        for (int tablesFileVersion = 1; tablesFileVersion <= numTablesFileVersions; tablesFileVersion++)
        {
            pumpkinHash.loadTables(tablesFileVersion);

            Seed seed = pumpkinHash.solveDPNaive(sequence, numMaxEditsE);

            seedsFile << tablesFileVersion << "," << seed.psi << "," << seed.omega << "," << seed.seed << endl;
        }
    }

    dataFile.close();

    seedsFile.close();

    cout << "Done!" << endl;

    return 0;
}