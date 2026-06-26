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

    int *tableA;
    int *tableB1;
    int *tableB2;
    int *tableC;

    vector<int*> tablesC;

    int returnDPTableIndex(const int, const int, const int, const int, const int);

public:
    PumpkinHash();
    PumpkinHash(const int, const int, const map<char, int>, const bool);
    ~PumpkinHash();

    void generateTables(const int);
    void loadTables(const int);

    vector<Seed> solveDP(const string, const int, const bool, const bool useTableB = false);
};

inline int PumpkinHash::returnDPTableIndex(const int numMaxEditsE, const int n, const int es, const int ed, const int d)
{
    int dpTableIndex = n * (numMaxEditsE + 1) * (numMaxEditsE + 2) * this->paramD / 2 + (es * (2 * (numMaxEditsE + 1) - (es - 1)) / 2 + ed) * this->paramD + d;

    return dpTableIndex;
}

#endif