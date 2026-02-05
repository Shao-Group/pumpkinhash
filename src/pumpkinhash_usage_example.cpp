#include "pumpkinhash.hpp"

int main(int argc, char **argv)
{
    string sequence = "AAAAAAAAAAAAAAAAAAAA";

    int paramD = 11, tablesFileVersion = 0, numMaxEditsE = 4;

    map<char, int> alphabet = {{'A', 0}, {'C', 1}, {'G', 2}, {'T', 3}};

    PumpkinHash pumpkinHash(sequence.length(), paramD, alphabet);

    pumpkinHash.generateTables(tablesFileVersion);

    pumpkinHash.loadTables(tablesFileVersion);

    Seed seed = pumpkinHash.solveDP(sequence, numMaxEditsE);

    cout << seed.psi << ", " << seed.omega << ", " << seed.seed << endl;

    return 0;
}