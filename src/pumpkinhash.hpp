#ifndef PUMPKINHASH_HPP
#define PUMPKINHASH_HPP

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <random>
#include <string>
#include <vector>

using namespace std;

#define POS_INF numeric_limits<int>::max()
#define NEG_INF numeric_limits<int>::min()

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

public:
    PumpkinHash();
    PumpkinHash(const int, const int, const map<char, int>);
    ~PumpkinHash();

    void generateTables();
    void loadTables();

    Seed solveDP(const string, const int);
};

#endif