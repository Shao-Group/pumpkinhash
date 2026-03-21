#include "pumpkinhash.hpp"

int main(int argc, char **argv)
{
    string sequence = "AAAAAAAAAAAAAAAAAAAA";

    int paramD = 11, tablesFileVersion = 0, numMaxEditsE = 4;

    bool doGenerateEplus1Seeds = false;

    map<char, int> alphabet = {{'A', 0}, {'C', 1}, {'G', 2}, {'T', 3}};

    PumpkinHash pumpkinHash(sequence.length(), paramD, alphabet);

    pumpkinHash.generateTables(tablesFileVersion);

    pumpkinHash.loadTables(tablesFileVersion);

    vector<Seed> seeds = pumpkinHash.solveDP(sequence, numMaxEditsE, doGenerateEplus1Seeds);

    cout << seeds[0].psi << ", " << seeds[0].omega << ", " << seeds[0].seed << endl;

    return 0;
}