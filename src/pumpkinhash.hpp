#ifndef PUMPKINHASH_HPP
#define PUMPKINHASH_HPP

#include <chrono>
#include <iostream>
#include <map>
#include <random>

using namespace std;

#define POS_INF numeric_limits<int>::max()
#define NEG_INF numeric_limits<int>::min()

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
    PumpkinHash(int, int, map<char, int>);
    ~PumpkinHash();

    void generateTables();
};

#endif