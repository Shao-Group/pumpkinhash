#include "pumpkinhash.hpp"

int main(int argc, char **argv)
{
    PumpkinHash pumpkinHash;

    pumpkinHash.generateTables();

    Seed seed = pumpkinHash.solveDP("GAGTC", 2);

    cout << seed.psi << ", " << seed.omega << endl;

    return 0;
}