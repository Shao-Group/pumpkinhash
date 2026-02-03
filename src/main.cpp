#include "pumpkinhash.hpp"

int main(int argc, char **argv)
{
    PumpkinHash pumpkinHash;

    // pumpkinHash.generateTables();

    pumpkinHash.loadTables();

    Seed seed = pumpkinHash.solveDP("AAAAAAAAAAAAAAAAAAAA", 4);

    cout << seed.psi << ", " << seed.omega << ", " << seed.seed << endl;

    return 0;
}