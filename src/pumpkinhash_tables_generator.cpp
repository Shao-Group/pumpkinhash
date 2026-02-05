#include "pumpkinhash.hpp"

int main(int argc, char **argv)
{
    if (argc != 4)
    {
        cerr << "Invalid number of command-line arguments provided!\nCorrect usage: ./pumpkinhash_tables_generator [windowSizeN] [paramD] [numTablesFileVersions]" << endl;

        return 1;
    }

    int windowSizeN = stoi(argv[1]), paramD = stoi(argv[2]), numTablesFileVersions = stoi(argv[3]);

    map<char, int> defaultAlphabet = {{'A', 0}, {'C', 1}, {'G', 2}, {'T', 3}};

    PumpkinHash pumpkinHash(windowSizeN, paramD, defaultAlphabet);

    cout << "PumpkinHash object created with N = " << windowSizeN << ", D = " << paramD << ", and default alphabet..." << endl;

    for (int tablesFileVersion = 1; tablesFileVersion <= numTablesFileVersions; tablesFileVersion++)
    {
        pumpkinHash.generateTables(tablesFileVersion);
    }

    cout << numTablesFileVersions << " separate sets of ABC tables generated with PumpkinHash!" << endl;

    return 0;
}