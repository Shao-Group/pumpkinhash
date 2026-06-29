#ifndef PUMPKINHASH_HPP
#define PUMPKINHASH_HPP

#include <cstdint>
#include <map>
#include <string>
#include <vector>

using namespace std;

struct DpTableCell
{
    int dpTableValue;
    uint64_t dpTableSeed;
};

struct Seed
{
    int psi;
    int omega;
    string seed;
};

class PumpkinHash
{
    int windowSizeN;
    int paramD;
    map<char, int> alphabet;

    vector<int> charToSigmaLookup;
    vector<char> sigmaToCharLookup;

    int *tableA;
    int *tableB1;
    int *tableB2;
    int *tableC;

    vector<int*> tablesC;

public:
    PumpkinHash();
    PumpkinHash(const int, const int, const map<char, int>, const bool);
    ~PumpkinHash();

    void generateTables(const int);
    void loadTables(const int);

    vector<Seed> solveDP(const string &, const int, const bool, const bool useTableB = false);
};

#endif