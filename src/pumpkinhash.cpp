#include "pumpkinhash.hpp"

PumpkinHash::PumpkinHash() : PumpkinHash(20, 11, {{'A', 0}, {'C', 1}, {'G', 2}, {'T', 3}})
{
    // Calling parameterized constructor from default constructor with default arguments
}

PumpkinHash::PumpkinHash(int windowSizeN, int paramD, map<char, int> alphabet)
{
    this->windowSizeN = windowSizeN;
    this->paramD = paramD;
    this->alphabet = alphabet;

    this->tableA = new int[windowSizeN * paramD * alphabet.size()];
    this->tableB1 = new int[windowSizeN * paramD * alphabet.size()];
    this->tableB2 = new int[windowSizeN * paramD * alphabet.size()];
    this->tableC = new int[windowSizeN * alphabet.size()];
}

PumpkinHash::~PumpkinHash()
{
    delete[] this->tableA;
    delete[] this->tableB1;
    delete[] this->tableB2;
    delete[] this->tableC;
}

void PumpkinHash::generateTables()
{
    
}