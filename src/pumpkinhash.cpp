#include "pumpkinhash.hpp"
#include "path_utils.hpp"

#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <fstream>
#include <limits>
#include <random>

#define POS_INF numeric_limits<int>::max()
#define NEG_INF numeric_limits<int>::min()

PumpkinHash::PumpkinHash() : PumpkinHash(20, 11, {{'A', 0}, {'C', 1}, {'G', 2}, {'T', 3}}, false)
{
    // Calling parameterized constructor from default constructor with default arguments
}

PumpkinHash::PumpkinHash(const int windowSizeN, const int paramD, const map<char, int> alphabet, const bool doUsetablesC)
{
    this->windowSizeN = windowSizeN;
    this->paramD = paramD;
    this->alphabet = alphabet;

    this->charToSigmaLookup.assign(256, -1);

    for (auto const &pair : alphabet)
    {
        this->charToSigmaLookup[(unsigned char) pair.first] = pair.second;
    }

    this->sigmaToCharLookup.assign(alphabet.size() + 1, '\0');

    for (auto const &pair : alphabet)
    {
        this->sigmaToCharLookup[pair.second] = pair.first;
    }

    this->sigmaToCharLookup[alphabet.size()] = '-';

    this->tableA = new int[windowSizeN * paramD * alphabet.size()];
    this->tableB1 = new int[windowSizeN * paramD * alphabet.size()];
    this->tableB2 = new int[windowSizeN * paramD * alphabet.size()];

    if (doUsetablesC)
    {
        this->tableC = nullptr;

        for (int d = 0; d < paramD; d++)
        {
            this->tablesC.push_back(new int[windowSizeN * alphabet.size()]);
        }
    }
    else
    {
        this->tableC = new int[windowSizeN * alphabet.size()];

        for (int d = 0; d < paramD; d++)
        {
            this->tablesC.push_back(nullptr);
        }
    }
}

PumpkinHash::~PumpkinHash()
{
    delete[] this->tableA;
    delete[] this->tableB1;
    delete[] this->tableB2;
    delete[] this->tableC;

    for (int d = 0; d < this->paramD; d++)
    {
        delete[] this->tablesC[d];
    }
}

void PumpkinHash::generateTables(const int tablesFileVersion)
{
    random_device seedSource;

    mt19937 pseudoRandomNumberEngine(seedSource());

    uniform_int_distribution<int> uniformIntegerDistribution(1 << 5, 1 << 10);

    int64_t tableANumbersSum = 0, tableANumbersCount = this->windowSizeN * this->paramD * this->alphabet.size();

    for (int n = 0; n < this->windowSizeN; n++)
    {
        for (int d = 0; d < this->paramD; d++)
        {
            for (int sigma = 0; sigma < this->alphabet.size(); sigma++)
            {
                this->tableA[n * this->paramD * this->alphabet.size() + d * this->alphabet.size() + sigma] = uniformIntegerDistribution(pseudoRandomNumberEngine);

                tableANumbersSum = tableANumbersSum + this->tableA[n * this->paramD * this->alphabet.size() + d * this->alphabet.size() + sigma];
            }
        }
    }

    int tableANumbersMean = tableANumbersSum / tableANumbersCount;

    for (int n = 0; n < this->windowSizeN; n++)
    {
        for (int d = 0; d < this->paramD; d++)
        {
            for (int sigma = 0; sigma < this->alphabet.size(); sigma++)
            {
                this->tableA[n * this->paramD * this->alphabet.size() + d * this->alphabet.size() + sigma] = this->tableA[n * this->paramD * this->alphabet.size() + d * this->alphabet.size() + sigma] - tableANumbersMean;
            }
        }
    }

    vector<int> pairValues;

    for (int i = 0; i < 4; i++)
    {
        pairValues.push_back(i);
    }

    if (this->alphabet.size() > 4)
    {
        for (int i = 4; i < this->alphabet.size(); i++)
        {
            pairValues.push_back(i % 4);
        }
    }

    unsigned int currentTimeBasedSeed;

    for (int n = 0; n < this->windowSizeN; n++)
    {
        for (int d = 0; d < this->paramD; d++)
        {
            currentTimeBasedSeed = chrono::system_clock::now().time_since_epoch().count();

            shuffle(pairValues.begin(), pairValues.end(), default_random_engine(currentTimeBasedSeed));

            for (int sigma = 0; sigma < this->alphabet.size(); sigma++)
            {
                this->tableB1[n * this->paramD * this->alphabet.size() + d * this->alphabet.size() + sigma] = ((pairValues[sigma] >> 1) % 2 == 1) ? 1 : -1;
                this->tableB2[n * this->paramD * this->alphabet.size() + d * this->alphabet.size() + sigma] = ((pairValues[sigma] >> 0) % 2 == 1) ? 1 : -1;
            }
        }
    }

    if (this->tableC == nullptr)
    {
        vector<int> paramDValues;

        for (int i = 0; i < this->paramD; i++)
        {
            paramDValues.push_back(i);
        }

        for (int n = 0; n < this->windowSizeN; n++)
        {
            for (int sigma = 0; sigma < this->alphabet.size(); sigma++)
            {
                currentTimeBasedSeed = chrono::system_clock::now().time_since_epoch().count();

                shuffle(paramDValues.begin(), paramDValues.end(), default_random_engine(currentTimeBasedSeed));

                for (int d = 0; d < this->paramD; d++)
                {
                    this->tablesC[d][n * this->alphabet.size() + sigma] = paramDValues[d];
                }
            }
        }
    }
    else
    {
        vector<int> paramDValues;

        for (int i = 0; i < this->paramD; i++)
        {
            paramDValues.push_back(i);
        }

        if (this->alphabet.size() > this->paramD)
        {
            for (int i = this->paramD; i < this->alphabet.size(); i++)
            {
                paramDValues.push_back(i % this->paramD);
            }
        }

        for (int n = 0; n < this->windowSizeN; n++)
        {
            currentTimeBasedSeed = chrono::system_clock::now().time_since_epoch().count();

            shuffle(paramDValues.begin(), paramDValues.end(), default_random_engine(currentTimeBasedSeed));

            for (int sigma = 0; sigma < this->alphabet.size(); sigma++)
            {
                this->tableC[n * this->alphabet.size() + sigma] = paramDValues[sigma];
            }
        }
    }

    string tablesFileFolderPath = pathJoin("..", pathJoin("tables", string("tables_N") + to_string(this->windowSizeN) + string("_D") + to_string(this->paramD) + string("_Sigma") + to_string(this->alphabet.size())));

    if (!pathExists(tablesFileFolderPath))
    {
        if (!createDirectories(tablesFileFolderPath))
        {
            exit(EXIT_FAILURE);
        }
    }

    string tablesFilePath = pathJoin(tablesFileFolderPath, string("tables_N") + to_string(this->windowSizeN) + string("_D") + to_string(this->paramD) + string("_Sigma") + to_string(this->alphabet.size()) + string("_Version") + to_string(tablesFileVersion));

    ofstream tablesFile(tablesFilePath);

    if (!tablesFile.is_open())
    {
        exit(EXIT_FAILURE);
    }

    for (int n = 0; n < this->windowSizeN; n++)
    {
        for (int d = 0; d < this->paramD; d++)
        {
            for (int sigma = 0; sigma < this->alphabet.size(); sigma++)
            {
                tablesFile << this->tableA[n * this->paramD * this->alphabet.size() + d * this->alphabet.size() + sigma] << " ";
            }

            tablesFile << endl;
        }

        tablesFile << endl;
    }

    for (int n = 0; n < this->windowSizeN; n++)
    {
        for (int d = 0; d < this->paramD; d++)
        {
            for (int sigma = 0; sigma < this->alphabet.size(); sigma++)
            {
                tablesFile << this->tableB1[n * this->paramD * this->alphabet.size() + d * this->alphabet.size() + sigma] << " ";
            }

            tablesFile << endl;
        }

        tablesFile << endl;
    }

    for (int n = 0; n < this->windowSizeN; n++)
    {
        for (int d = 0; d < this->paramD; d++)
        {
            for (int sigma = 0; sigma < this->alphabet.size(); sigma++)
            {
                tablesFile << this->tableB2[n * this->paramD * this->alphabet.size() + d * this->alphabet.size() + sigma] << " ";
            }

            tablesFile << endl;
        }

        tablesFile << endl;
    }

    if (this->tableC == nullptr)
    {
        for (int d = 0; d < this->paramD; d++)
        {
            for (int n = 0; n < this->windowSizeN; n++)
            {
                for (int sigma = 0; sigma < this->alphabet.size(); sigma++)
                {
                    tablesFile << this->tablesC[d][n * this->alphabet.size() + sigma] << " ";
                }

                tablesFile << endl;
            }

            if (d < this->paramD - 1)
            {
                tablesFile << endl;
            }
        }
    }
    else
    {
        for (int n = 0; n < this->windowSizeN; n++)
        {
            for (int sigma = 0; sigma < this->alphabet.size(); sigma++)
            {
                tablesFile << this->tableC[n * this->alphabet.size() + sigma] << " ";
            }

            tablesFile << endl;
        }
    }

    tablesFile.close();

    return;
}

void PumpkinHash::loadTables(const int tablesFileVersion)
{
    string tablesFilePath = pathJoin(pathJoin(pathJoin("..", "tables"), string("tables_N") + to_string(this->windowSizeN) + string("_D") + to_string(this->paramD) + string("_Sigma") + to_string(this->alphabet.size())), string("tables_N") + to_string(this->windowSizeN) + string("_D") + to_string(this->paramD) + string("_Sigma") + to_string(this->alphabet.size()) + string("_Version") + to_string(tablesFileVersion));

    ifstream tablesFile;

    tablesFile.open(tablesFilePath);

    if (!tablesFile.is_open())
    {
        exit(EXIT_FAILURE);
    }

    for (int n = 0; n < this->windowSizeN; n++)
    {
        for (int d = 0; d < this->paramD; d++)
        {
            for (int sigma = 0; sigma < this->alphabet.size(); sigma++)
            {
                tablesFile >> this->tableA[n * this->paramD * this->alphabet.size() + d * this->alphabet.size() + sigma];
            }
        }
    }

    for (int n = 0; n < this->windowSizeN; n++)
    {
        for (int d = 0; d < this->paramD; d++)
        {
            for (int sigma = 0; sigma < this->alphabet.size(); sigma++)
            {
                tablesFile >> this->tableB1[n * this->paramD * this->alphabet.size() + d * this->alphabet.size() + sigma];
            }
        }
    }

    for (int n = 0; n < this->windowSizeN; n++)
    {
        for (int d = 0; d < this->paramD; d++)
        {
            for (int sigma = 0; sigma < this->alphabet.size(); sigma++)
            {
                tablesFile >> this->tableB2[n * this->paramD * this->alphabet.size() + d * this->alphabet.size() + sigma];
            }
        }
    }

    if (this->tableC == nullptr)
    {
        for (int d = 0; d < this->paramD; d++)
        {
            for (int n = 0; n < this->windowSizeN; n++)
            {
                for (int sigma = 0, tablesCEntry; sigma < this->alphabet.size(); sigma++)
                {
                    tablesFile >> tablesCEntry;

                    // tablesC in this case functions as an inverse lookup table for the original tablesC
                    this->tablesC[tablesCEntry][n * this->alphabet.size() + sigma] = d;
                }
            }
        }
    }
    else
    {
        for (int n = 0; n < this->windowSizeN; n++)
        {
            for (int sigma = 0; sigma < this->alphabet.size(); sigma++)
            {
                tablesFile >> this->tableC[n * this->alphabet.size() + sigma];
            }
        }
    }

    tablesFile.close();

    return;
}

vector<Seed> PumpkinHash::solveDP(const string &sequence, const int numMaxEditsE, const bool doGenerateEplus1Seeds, const bool useTableB)
{
    if (sequence.length() != (size_t) this->windowSizeN)
    {
        exit(EXIT_FAILURE);
    }

    const int sigmaSize = this->alphabet.size();
    const int paramD = this->paramD;
    
    vector<int> sequenceSymbols(this->windowSizeN);

    for (int i = 0; i < this->windowSizeN; i++)
    {
        sequenceSymbols[i] = this->charToSigmaLookup[(unsigned char) sequence[i]];
    }

    const int windowSizeN = this->windowSizeN;
    const int dpTableSize = (windowSizeN + 1) * (numMaxEditsE + 1) * (numMaxEditsE + 2) * paramD / 2;

    auto dpTableIndex = [numMaxEditsE, paramD](const int n, const int es, const int ed, const int d) -> int
    {
        return n * (numMaxEditsE + 1) * (numMaxEditsE + 2) * paramD / 2 + (es * (2 * (numMaxEditsE + 1) - (es - 1)) / 2 + ed) * paramD + d;
    };

    auto decodeSeedString = [this, windowSizeN](uint64_t dpTableTSeedCopy) -> string
    {
        string seedStr(windowSizeN, '\0');

        for (int i = windowSizeN - 1; i >= 0; i--)
        {
            seedStr[i] = this->sigmaToCharLookup[dpTableTSeedCopy & 7];

            dpTableTSeedCopy >>= 3;
        }

        return seedStr;
    };

    if (useTableB)
    {
        DpTableCell *dpTableTmin = new DpTableCell[dpTableSize];
        DpTableCell *dpTableTmax = new DpTableCell[dpTableSize];

        for (int i = 0; i < dpTableSize; i++)
        {
            dpTableTmin[i].dpTableValue = POS_INF;
            dpTableTmin[i].dpTableSeed = UINT64_MAX;
            dpTableTmax[i].dpTableValue = NEG_INF;
            dpTableTmax[i].dpTableSeed = UINT64_MAX;
        }

        dpTableTmin[0].dpTableValue = 0;
        dpTableTmax[0].dpTableValue = 0;
        dpTableTmin[0].dpTableSeed = 0;
        dpTableTmax[0].dpTableSeed = 0;

        for (int n = 1; n <= windowSizeN; n++)
        {
            for (int es = 0; es <= numMaxEditsE; es++)
            {
                for (int ed = 0; ed <= numMaxEditsE - es && ed <= n - 1; ed++)
                {
                    const int tableBaseOffset = (n - 1 - ed) * paramD * sigmaSize;
                    const int tableCBaseOffset = (n - 1 - ed) * sigmaSize;
                    
                    const int sequenceSymbol = sequenceSymbols[n - 1];

                    for (int d = 0; d < paramD; d++)
                    {
                        // Case-1: base s_n deleted
                        DpTableCell dpTableTminCase1Del, dpTableTmaxCase1Del;

                        dpTableTminCase1Del.dpTableValue = POS_INF;
                        dpTableTmaxCase1Del.dpTableValue = NEG_INF;

                        dpTableTminCase1Del.dpTableSeed = UINT64_MAX;
                        dpTableTmaxCase1Del.dpTableSeed = UINT64_MAX;

                        if (ed != 0)
                        {
                            const int dpTableIdxPrevDel = dpTableIndex(n - 1, es, ed - 1, d);
                            const uint64_t dpTableTminPrevDelSeed = dpTableTmin[dpTableIdxPrevDel].dpTableSeed;
                            const uint64_t dpTableTmaxPrevDelSeed = dpTableTmax[dpTableIdxPrevDel].dpTableSeed;

                            dpTableTminCase1Del.dpTableValue = dpTableTmin[dpTableIdxPrevDel].dpTableValue;
                            dpTableTmaxCase1Del.dpTableValue = dpTableTmax[dpTableIdxPrevDel].dpTableValue;

                            dpTableTminCase1Del.dpTableSeed = (dpTableTminPrevDelSeed == UINT64_MAX) ? UINT64_MAX : (dpTableTminPrevDelSeed << 3) | sigmaSize;
                            dpTableTmaxCase1Del.dpTableSeed = (dpTableTmaxPrevDelSeed == UINT64_MAX) ? UINT64_MAX : (dpTableTmaxPrevDelSeed << 3) | sigmaSize;
                        }

                        // Case-2: base s_n retained
                        DpTableCell dpTableTminCase2Ret, dpTableTmaxCase2Ret;

                        dpTableTminCase2Ret.dpTableValue = this->tableA[tableBaseOffset + d * sigmaSize + sequenceSymbol] * this->tableB2[tableBaseOffset + d * sigmaSize + sequenceSymbol];
                        dpTableTmaxCase2Ret.dpTableValue = this->tableA[tableBaseOffset + d * sigmaSize + sequenceSymbol] * this->tableB2[tableBaseOffset + d * sigmaSize + sequenceSymbol];

                        int dPrevious = -1;

                        if (this->tableC == nullptr)
                        {
                            dPrevious = this->tablesC[d][tableCBaseOffset + sequenceSymbol];
                        }
                        else
                        {
                            dPrevious = (d - this->tableC[tableCBaseOffset + sequenceSymbol] + this->paramD) % this->paramD;
                        }

                        if (this->tableB1[tableBaseOffset + d * sigmaSize + sequenceSymbol] == 1)
                        {
                            const int dpTableIdxPrev = dpTableIndex(n - 1, es, ed, dPrevious);
                            const int dpTableTminPrevValue = dpTableTmin[dpTableIdxPrev].dpTableValue;
                            const int dpTableTmaxPrevValue = dpTableTmax[dpTableIdxPrev].dpTableValue;
                            const uint64_t dpTableTminPrevSeed = dpTableTmin[dpTableIdxPrev].dpTableSeed;
                            const uint64_t dpTableTmaxPrevSeed = dpTableTmax[dpTableIdxPrev].dpTableSeed;

                            dpTableTminCase2Ret.dpTableValue = (dpTableTminPrevValue == POS_INF) ? POS_INF : (dpTableTminCase2Ret.dpTableValue + dpTableTminPrevValue);
                            dpTableTmaxCase2Ret.dpTableValue = (dpTableTmaxPrevValue == NEG_INF) ? NEG_INF : (dpTableTmaxCase2Ret.dpTableValue + dpTableTmaxPrevValue);

                            dpTableTminCase2Ret.dpTableSeed = (dpTableTminPrevSeed == UINT64_MAX) ? UINT64_MAX : (dpTableTminPrevSeed << 3) | sequenceSymbol;
                            dpTableTmaxCase2Ret.dpTableSeed = (dpTableTmaxPrevSeed == UINT64_MAX) ? UINT64_MAX : (dpTableTmaxPrevSeed << 3) | sequenceSymbol;
                        }
                        else
                        {
                            const int dpTableIdxPrev = dpTableIndex(n - 1, es, ed, dPrevious);
                            const int dpTableTminPrevValue = dpTableTmin[dpTableIdxPrev].dpTableValue;
                            const int dpTableTmaxPrevValue = dpTableTmax[dpTableIdxPrev].dpTableValue;
                            const uint64_t dpTableTminPrevSeed = dpTableTmin[dpTableIdxPrev].dpTableSeed;
                            const uint64_t dpTableTmaxPrevSeed = dpTableTmax[dpTableIdxPrev].dpTableSeed;

                            dpTableTminCase2Ret.dpTableValue = (dpTableTmaxPrevValue == NEG_INF) ? POS_INF : (dpTableTminCase2Ret.dpTableValue - dpTableTmaxPrevValue);
                            dpTableTmaxCase2Ret.dpTableValue = (dpTableTminPrevValue == POS_INF) ? NEG_INF : (dpTableTmaxCase2Ret.dpTableValue - dpTableTminPrevValue);

                            dpTableTminCase2Ret.dpTableSeed = (dpTableTmaxPrevSeed == UINT64_MAX) ? UINT64_MAX : (dpTableTmaxPrevSeed << 3) | sequenceSymbol;
                            dpTableTmaxCase2Ret.dpTableSeed = (dpTableTminPrevSeed == UINT64_MAX) ? UINT64_MAX : (dpTableTminPrevSeed << 3) | sequenceSymbol;
                        }

                        // Case-3: base s_n substituted with s'_n
                        DpTableCell dpTableTminCase3Sub, dpTableTmaxCase3Sub;

                        dpTableTminCase3Sub.dpTableValue = POS_INF;
                        dpTableTmaxCase3Sub.dpTableValue = NEG_INF;

                        dpTableTminCase3Sub.dpTableSeed = UINT64_MAX;
                        dpTableTmaxCase3Sub.dpTableSeed = UINT64_MAX;

                        if (es != 0)
                        {
                            for (int sigma = 0; sigma < sigmaSize; sigma++)
                            {
                                if (sigma == sequenceSymbol)
                                {
                                    continue;
                                }

                                DpTableCell dpTableTminCase3SubTemp, dpTableTmaxCase3SubTemp;

                                dpTableTminCase3SubTemp.dpTableValue = this->tableA[tableBaseOffset + d * sigmaSize + sigma] * this->tableB2[tableBaseOffset + d * sigmaSize + sigma];
                                dpTableTmaxCase3SubTemp.dpTableValue = this->tableA[tableBaseOffset + d * sigmaSize + sigma] * this->tableB2[tableBaseOffset + d * sigmaSize + sigma];

                                dPrevious = -1;

                                if (this->tableC == nullptr)
                                {
                                    dPrevious = this->tablesC[d][tableCBaseOffset + sigma];
                                }
                                else
                                {
                                    dPrevious = (d - this->tableC[tableCBaseOffset + sigma] + this->paramD) % this->paramD;
                                }

                                if (this->tableB1[tableBaseOffset + d * sigmaSize + sigma] == 1)
                                {
                                    const int dpTableIdxPrevSub = dpTableIndex(n - 1, es - 1, ed, dPrevious);
                                    const int dpTableTminPrevSubValue = dpTableTmin[dpTableIdxPrevSub].dpTableValue;
                                    const int dpTableTmaxPrevSubValue = dpTableTmax[dpTableIdxPrevSub].dpTableValue;
                                    const uint64_t dpTableTminPrevSubSeed = dpTableTmin[dpTableIdxPrevSub].dpTableSeed;
                                    const uint64_t dpTableTmaxPrevSubSeed = dpTableTmax[dpTableIdxPrevSub].dpTableSeed;

                                    dpTableTminCase3SubTemp.dpTableValue = (dpTableTminPrevSubValue == POS_INF) ? POS_INF : (dpTableTminCase3SubTemp.dpTableValue + dpTableTminPrevSubValue);
                                    dpTableTmaxCase3SubTemp.dpTableValue = (dpTableTmaxPrevSubValue == NEG_INF) ? NEG_INF : (dpTableTmaxCase3SubTemp.dpTableValue + dpTableTmaxPrevSubValue);

                                    dpTableTminCase3SubTemp.dpTableSeed = (dpTableTminPrevSubSeed == UINT64_MAX) ? UINT64_MAX : (dpTableTminPrevSubSeed << 3) | sigma;
                                    dpTableTmaxCase3SubTemp.dpTableSeed = (dpTableTmaxPrevSubSeed == UINT64_MAX) ? UINT64_MAX : (dpTableTmaxPrevSubSeed << 3) | sigma;
                                }
                                else
                                {
                                    const int dpTableIdxPrevSub = dpTableIndex(n - 1, es - 1, ed, dPrevious);
                                    const int dpTableTminPrevSubValue = dpTableTmin[dpTableIdxPrevSub].dpTableValue;
                                    const int dpTableTmaxPrevSubValue = dpTableTmax[dpTableIdxPrevSub].dpTableValue;
                                    const uint64_t dpTableTminPrevSubSeed = dpTableTmin[dpTableIdxPrevSub].dpTableSeed;
                                    const uint64_t dpTableTmaxPrevSubSeed = dpTableTmax[dpTableIdxPrevSub].dpTableSeed;

                                    dpTableTminCase3SubTemp.dpTableValue = (dpTableTmaxPrevSubValue == NEG_INF) ? POS_INF : (dpTableTminCase3SubTemp.dpTableValue - dpTableTmaxPrevSubValue);
                                    dpTableTmaxCase3SubTemp.dpTableValue = (dpTableTminPrevSubValue == POS_INF) ? NEG_INF : (dpTableTmaxCase3SubTemp.dpTableValue - dpTableTminPrevSubValue);

                                    dpTableTminCase3SubTemp.dpTableSeed = (dpTableTmaxPrevSubSeed == UINT64_MAX) ? UINT64_MAX : (dpTableTmaxPrevSubSeed << 3) | sigma;
                                    dpTableTmaxCase3SubTemp.dpTableSeed = (dpTableTminPrevSubSeed == UINT64_MAX) ? UINT64_MAX : (dpTableTminPrevSubSeed << 3) | sigma;
                                }

                                if (dpTableTminCase3SubTemp.dpTableValue < dpTableTminCase3Sub.dpTableValue)
                                {
                                    dpTableTminCase3Sub.dpTableValue = dpTableTminCase3SubTemp.dpTableValue;
                                    dpTableTminCase3Sub.dpTableSeed = dpTableTminCase3SubTemp.dpTableSeed;
                                }

                                if (dpTableTmaxCase3SubTemp.dpTableValue > dpTableTmaxCase3Sub.dpTableValue)
                                {
                                    dpTableTmaxCase3Sub.dpTableValue = dpTableTmaxCase3SubTemp.dpTableValue;
                                    dpTableTmaxCase3Sub.dpTableSeed = dpTableTmaxCase3SubTemp.dpTableSeed;
                                }
                            }
                        }

                        const int dpTableIdxCur = dpTableIndex(n, es, ed, d);

                        if (dpTableTminCase1Del.dpTableValue < dpTableTminCase2Ret.dpTableValue)
                        {
                            if (dpTableTminCase1Del.dpTableValue < dpTableTminCase3Sub.dpTableValue)
                            {
                                dpTableTmin[dpTableIdxCur].dpTableValue = dpTableTminCase1Del.dpTableValue;
                                dpTableTmin[dpTableIdxCur].dpTableSeed = dpTableTminCase1Del.dpTableSeed;
                            }
                            else
                            {
                                dpTableTmin[dpTableIdxCur].dpTableValue = dpTableTminCase3Sub.dpTableValue;
                                dpTableTmin[dpTableIdxCur].dpTableSeed = dpTableTminCase3Sub.dpTableSeed;
                            }
                        }
                        else
                        {
                            if (dpTableTminCase2Ret.dpTableValue < dpTableTminCase3Sub.dpTableValue)
                            {
                                dpTableTmin[dpTableIdxCur].dpTableValue = dpTableTminCase2Ret.dpTableValue;
                                dpTableTmin[dpTableIdxCur].dpTableSeed = dpTableTminCase2Ret.dpTableSeed;
                            }
                            else
                            {
                                dpTableTmin[dpTableIdxCur].dpTableValue = dpTableTminCase3Sub.dpTableValue;
                                dpTableTmin[dpTableIdxCur].dpTableSeed = dpTableTminCase3Sub.dpTableSeed;
                            }
                        }

                        if (dpTableTmaxCase1Del.dpTableValue > dpTableTmaxCase2Ret.dpTableValue)
                        {
                            if (dpTableTmaxCase1Del.dpTableValue > dpTableTmaxCase3Sub.dpTableValue)
                            {
                                dpTableTmax[dpTableIdxCur].dpTableValue = dpTableTmaxCase1Del.dpTableValue;
                                dpTableTmax[dpTableIdxCur].dpTableSeed = dpTableTmaxCase1Del.dpTableSeed;
                            }
                            else
                            {
                                dpTableTmax[dpTableIdxCur].dpTableValue = dpTableTmaxCase3Sub.dpTableValue;
                                dpTableTmax[dpTableIdxCur].dpTableSeed = dpTableTmaxCase3Sub.dpTableSeed;
                            }
                        }
                        else
                        {
                            if (dpTableTmaxCase2Ret.dpTableValue > dpTableTmaxCase3Sub.dpTableValue)
                            {
                                dpTableTmax[dpTableIdxCur].dpTableValue = dpTableTmaxCase2Ret.dpTableValue;
                                dpTableTmax[dpTableIdxCur].dpTableSeed = dpTableTmaxCase2Ret.dpTableSeed;
                            }
                            else
                            {
                                dpTableTmax[dpTableIdxCur].dpTableValue = dpTableTmaxCase3Sub.dpTableValue;
                                dpTableTmax[dpTableIdxCur].dpTableSeed = dpTableTmaxCase3Sub.dpTableSeed;
                            }
                        }
                    }
                }
            }
        }

        vector<Seed> seeds;

        if (doGenerateEplus1Seeds)
        {
            DpTableCell dpTableT[numMaxEditsE + 1][this->paramD];

            for (int d = 0; d < this->paramD; d++)
            {
                for (int es = 0; es <= numMaxEditsE; es++)
                {
                    dpTableT[es][d].dpTableValue = NEG_INF;
                    dpTableT[es][d].dpTableSeed = UINT64_MAX;

                    int ed = numMaxEditsE - es;
                    const int dpTableIdxFinal = dpTableIndex(windowSizeN, es, ed, d);
                    const int dpTableTminFinalValue = dpTableTmin[dpTableIdxFinal].dpTableValue;
                    const int dpTableTmaxFinalValue = dpTableTmax[dpTableIdxFinal].dpTableValue;

                    if (dpTableTminFinalValue == POS_INF && dpTableTmaxFinalValue == NEG_INF)
                    {
                        dpTableT[es][d].dpTableValue = NEG_INF;
                        dpTableT[es][d].dpTableSeed = UINT64_MAX;
                    }
                    else if (dpTableTminFinalValue < POS_INF && dpTableTmaxFinalValue == NEG_INF)
                    {
                        dpTableT[es][d].dpTableValue = abs(dpTableTminFinalValue);
                        dpTableT[es][d].dpTableSeed = dpTableTmin[dpTableIdxFinal].dpTableSeed;
                    }
                    else if (dpTableTminFinalValue == POS_INF && dpTableTmaxFinalValue > NEG_INF)
                    {
                        dpTableT[es][d].dpTableValue = abs(dpTableTmaxFinalValue);
                        dpTableT[es][d].dpTableSeed = dpTableTmax[dpTableIdxFinal].dpTableSeed;
                    }
                    else
                    {
                        if (abs(dpTableTminFinalValue) > abs(dpTableTmaxFinalValue))
                        {
                            dpTableT[es][d].dpTableValue = abs(dpTableTminFinalValue);
                            dpTableT[es][d].dpTableSeed = dpTableTmin[dpTableIdxFinal].dpTableSeed;
                        }
                        else
                        {
                            dpTableT[es][d].dpTableValue = abs(dpTableTmaxFinalValue);
                            dpTableT[es][d].dpTableSeed = dpTableTmax[dpTableIdxFinal].dpTableSeed;
                        }
                    }
                }
            }

            for (int es = 0; es <= numMaxEditsE; es++)
            {
                Seed seed;

                seed.psi = -1;

                for (int d = 0; d < this->paramD; d++)
                {
                    if (dpTableT[es][d].dpTableValue > NEG_INF)
                    {
                        seed.psi = d;
                        break;
                    }
                }

                seed.omega = (seed.psi == -1) ? NEG_INF : dpTableT[es][seed.psi].dpTableValue;

                if (seed.psi == -1)
                {
                    seed.seed = "X";
                }
                else
                {
                    seed.seed = decodeSeedString(dpTableT[es][seed.psi].dpTableSeed);
                }

                seeds.push_back(seed);
            }
        }
        else
        {
            DpTableCell dpTableT[this->paramD];

            for (int d = 0; d < this->paramD; d++)
            {
                dpTableT[d].dpTableValue = NEG_INF;
                dpTableT[d].dpTableSeed = UINT64_MAX;

                for (int es = 0; es <= numMaxEditsE; es++)
                {
                    for (int ed = 0; ed <= numMaxEditsE - es; ed++)
                    {
                        DpTableCell dpTableTd;

                        const int dpTableIdxFinal = dpTableIndex(windowSizeN, es, ed, d);
                        const int dpTableTminFinalValue = dpTableTmin[dpTableIdxFinal].dpTableValue;
                        const int dpTableTmaxFinalValue = dpTableTmax[dpTableIdxFinal].dpTableValue;

                        if (dpTableTminFinalValue == POS_INF && dpTableTmaxFinalValue == NEG_INF)
                        {
                            dpTableTd.dpTableValue = NEG_INF;
                            dpTableTd.dpTableSeed = UINT64_MAX;
                        }
                        else if (dpTableTminFinalValue < POS_INF && dpTableTmaxFinalValue == NEG_INF)
                        {
                            dpTableTd.dpTableValue = abs(dpTableTminFinalValue);
                            dpTableTd.dpTableSeed = dpTableTmin[dpTableIdxFinal].dpTableSeed;
                        }
                        else if (dpTableTminFinalValue == POS_INF && dpTableTmaxFinalValue > NEG_INF)
                        {
                            dpTableTd.dpTableValue = abs(dpTableTmaxFinalValue);
                            dpTableTd.dpTableSeed = dpTableTmax[dpTableIdxFinal].dpTableSeed;
                        }
                        else
                        {
                            if (abs(dpTableTminFinalValue) > abs(dpTableTmaxFinalValue))
                            {
                                dpTableTd.dpTableValue = abs(dpTableTminFinalValue);
                                dpTableTd.dpTableSeed = dpTableTmin[dpTableIdxFinal].dpTableSeed;
                            }
                            else
                            {
                                dpTableTd.dpTableValue = abs(dpTableTmaxFinalValue);
                                dpTableTd.dpTableSeed = dpTableTmax[dpTableIdxFinal].dpTableSeed;
                            }
                        }

                        if (dpTableTd.dpTableValue > dpTableT[d].dpTableValue)
                        {
                            dpTableT[d].dpTableValue = dpTableTd.dpTableValue;
                            dpTableT[d].dpTableSeed = dpTableTd.dpTableSeed;
                        }
                    }
                }
            }

            Seed seed;

            seed.psi = -1;

            for (int d = 0; d < this->paramD; d++)
            {
                if (dpTableT[d].dpTableValue > NEG_INF)
                {
                    seed.psi = d;
                    break;
                }
            }

            seed.omega = (seed.psi == -1) ? NEG_INF : dpTableT[seed.psi].dpTableValue;

            if (seed.psi == -1)
            {
                seed.seed = "X";
            }
            else
            {
                seed.seed = decodeSeedString(dpTableT[seed.psi].dpTableSeed);
            }

            seeds.push_back(seed);
        }

        delete[] dpTableTmin;
        delete[] dpTableTmax;

        return seeds;
    }
    else
    {
        DpTableCell *dpTableTdp = new DpTableCell[dpTableSize];

        for (int i = 0; i < dpTableSize; i++)
        {
            dpTableTdp[i].dpTableValue = NEG_INF;
            dpTableTdp[i].dpTableSeed = UINT64_MAX;
        }

        dpTableTdp[0].dpTableValue = 0;
        dpTableTdp[0].dpTableSeed = 0;

        for (int n = 1; n <= windowSizeN; n++)
        {
            for (int es = 0; es <= numMaxEditsE; es++)
            {
                for (int ed = 0; ed <= numMaxEditsE - es && ed <= n - 1; ed++)
                {
                    const int tableBaseOffset = (n - 1 - ed) * paramD * sigmaSize;
                    const int tableCBaseOffset = (n - 1 - ed) * sigmaSize;
                    
                    const int sequenceSymbol = sequenceSymbols[n - 1];

                    for (int d = 0; d < paramD; d++)
                    {
                        // Case-1: base s_n deleted
                        DpTableCell dpTableTdpCase1Del;

                        dpTableTdpCase1Del.dpTableValue = NEG_INF;
                        dpTableTdpCase1Del.dpTableSeed = UINT64_MAX;

                        if (ed != 0)
                        {
                            const int dpTableIdxPrevDel = dpTableIndex(n - 1, es, ed - 1, d);
                            const uint64_t dpTableTdpPrevDelSeed = dpTableTdp[dpTableIdxPrevDel].dpTableSeed;

                            dpTableTdpCase1Del.dpTableValue = dpTableTdp[dpTableIdxPrevDel].dpTableValue;
                            dpTableTdpCase1Del.dpTableSeed = (dpTableTdpPrevDelSeed == UINT64_MAX) ? UINT64_MAX : (dpTableTdpPrevDelSeed << 3) | sigmaSize;
                        }

                        // Case-2: base s_n retained
                        DpTableCell dpTableTdpCase2Ret;
                        
                        dpTableTdpCase2Ret.dpTableValue = this->tableA[tableBaseOffset + d * sigmaSize + sequenceSymbol];

                        int dPrevious = -1;

                        if (this->tableC == nullptr)
                        {
                            dPrevious = this->tablesC[d][tableCBaseOffset + sequenceSymbol];
                        }
                        else
                        {
                            dPrevious = (d - this->tableC[tableCBaseOffset + sequenceSymbol] + this->paramD) % this->paramD;
                        }

                        const int dpTableIdxPrev = dpTableIndex(n - 1, es, ed, dPrevious);
                        const int dpTableTdpPrevValue = dpTableTdp[dpTableIdxPrev].dpTableValue;
                        const uint64_t dpTableTdpPrevSeed = dpTableTdp[dpTableIdxPrev].dpTableSeed;

                        dpTableTdpCase2Ret.dpTableValue = (dpTableTdpPrevValue == NEG_INF) ? NEG_INF : (dpTableTdpCase2Ret.dpTableValue + dpTableTdpPrevValue);
                        dpTableTdpCase2Ret.dpTableSeed = (dpTableTdpPrevSeed == UINT64_MAX) ? UINT64_MAX : (dpTableTdpPrevSeed << 3) | sequenceSymbol;

                        // Case-3: base s_n substituted with s'_n
                        DpTableCell dpTableTdpCase3Sub;

                        dpTableTdpCase3Sub.dpTableValue = NEG_INF;
                        dpTableTdpCase3Sub.dpTableSeed = UINT64_MAX;

                        if (es != 0)
                        {
                            for (int sigma = 0; sigma < sigmaSize; sigma++)
                            {
                                if (sigma == sequenceSymbol)
                                {
                                    continue;
                                }

                                DpTableCell dpTableTdpCase3SubTemp;

                                dpTableTdpCase3SubTemp.dpTableValue = this->tableA[tableBaseOffset + d * sigmaSize + sigma];

                                dPrevious = -1;

                                if (this->tableC == nullptr)
                                {
                                    dPrevious = this->tablesC[d][tableCBaseOffset + sigma];
                                }
                                else
                                {
                                    dPrevious = (d - this->tableC[tableCBaseOffset + sigma] + this->paramD) % this->paramD;
                                }

                                const int dpTableIdxPrevSub = dpTableIndex(n - 1, es - 1, ed, dPrevious);
                                const int dpTableTdpPrevSubValue = dpTableTdp[dpTableIdxPrevSub].dpTableValue;
                                const uint64_t dpTableTdpPrevSubSeed = dpTableTdp[dpTableIdxPrevSub].dpTableSeed;

                                dpTableTdpCase3SubTemp.dpTableValue = (dpTableTdpPrevSubValue == NEG_INF) ? NEG_INF : (dpTableTdpCase3SubTemp.dpTableValue + dpTableTdpPrevSubValue);
                                dpTableTdpCase3SubTemp.dpTableSeed = (dpTableTdpPrevSubSeed == UINT64_MAX) ? UINT64_MAX : (dpTableTdpPrevSubSeed << 3) | sigma;

                                if (dpTableTdpCase3SubTemp.dpTableValue > dpTableTdpCase3Sub.dpTableValue)
                                {
                                    dpTableTdpCase3Sub.dpTableValue = dpTableTdpCase3SubTemp.dpTableValue;
                                    dpTableTdpCase3Sub.dpTableSeed = dpTableTdpCase3SubTemp.dpTableSeed;
                                }
                            }
                        }

                        const int dpTableIdxCur = dpTableIndex(n, es, ed, d);

                        if (dpTableTdpCase1Del.dpTableValue > dpTableTdpCase2Ret.dpTableValue)
                        {
                            if (dpTableTdpCase1Del.dpTableValue > dpTableTdpCase3Sub.dpTableValue)
                            {
                                dpTableTdp[dpTableIdxCur].dpTableValue = dpTableTdpCase1Del.dpTableValue;
                                dpTableTdp[dpTableIdxCur].dpTableSeed = dpTableTdpCase1Del.dpTableSeed;
                            }
                            else
                            {
                                dpTableTdp[dpTableIdxCur].dpTableValue = dpTableTdpCase3Sub.dpTableValue;
                                dpTableTdp[dpTableIdxCur].dpTableSeed = dpTableTdpCase3Sub.dpTableSeed;
                            }
                        }
                        else
                        {
                            if (dpTableTdpCase2Ret.dpTableValue > dpTableTdpCase3Sub.dpTableValue)
                            {
                                dpTableTdp[dpTableIdxCur].dpTableValue = dpTableTdpCase2Ret.dpTableValue;
                                dpTableTdp[dpTableIdxCur].dpTableSeed = dpTableTdpCase2Ret.dpTableSeed;
                            }
                            else
                            {
                                dpTableTdp[dpTableIdxCur].dpTableValue = dpTableTdpCase3Sub.dpTableValue;
                                dpTableTdp[dpTableIdxCur].dpTableSeed = dpTableTdpCase3Sub.dpTableSeed;
                            }
                        }
                    }
                }
            }
        }

        vector<Seed> seeds;

        if (doGenerateEplus1Seeds)
        {
            DpTableCell dpTableT[numMaxEditsE + 1][this->paramD];

            for (int d = 0; d < this->paramD; d++)
            {
                for (int es = 0; es <= numMaxEditsE; es++)
                {
                    int ed = numMaxEditsE - es;
                    const int dpTableIdxFinal = dpTableIndex(windowSizeN, es, ed, d);
                    const int dpTableTdpFinalValue = dpTableTdp[dpTableIdxFinal].dpTableValue;

                    dpTableT[es][d].dpTableValue = (dpTableTdpFinalValue == NEG_INF) ? NEG_INF : abs(dpTableTdpFinalValue);
                    dpTableT[es][d].dpTableSeed = (dpTableTdpFinalValue == NEG_INF) ? UINT64_MAX : dpTableTdp[dpTableIdxFinal].dpTableSeed;
                }
            }

            for (int es = 0; es <= numMaxEditsE; es++)
            {
                Seed seed;

                seed.psi = -1;

                for (int d = 0; d < this->paramD; d++)
                {
                    if (dpTableT[es][d].dpTableValue > NEG_INF)
                    {
                        seed.psi = d;
                        break;
                    }
                }

                seed.omega = (seed.psi == -1) ? NEG_INF : dpTableT[es][seed.psi].dpTableValue;

                if (seed.psi == -1)
                {
                    seed.seed = "X";
                }
                else
                {
                    seed.seed = decodeSeedString(dpTableT[es][seed.psi].dpTableSeed);
                }

                seeds.push_back(seed);
            }
        }
        else
        {
            DpTableCell dpTableT[this->paramD];

            for (int d = 0; d < this->paramD; d++)
            {
                dpTableT[d].dpTableValue = NEG_INF;
                dpTableT[d].dpTableSeed = UINT64_MAX;

                for (int es = 0; es <= numMaxEditsE; es++)
                {
                    for (int ed = 0; ed <= numMaxEditsE - es; ed++)
                    {
                        DpTableCell dpTableTd;

                        const int dpTableIdxFinal = dpTableIndex(windowSizeN, es, ed, d);
                        const int dpTableTdpFinalValue = dpTableTdp[dpTableIdxFinal].dpTableValue;

                        dpTableTd.dpTableValue = (dpTableTdpFinalValue == NEG_INF) ? NEG_INF : abs(dpTableTdpFinalValue);
                        dpTableTd.dpTableSeed = (dpTableTdpFinalValue == NEG_INF) ? UINT64_MAX : dpTableTdp[dpTableIdxFinal].dpTableSeed;

                        if (dpTableTd.dpTableValue > dpTableT[d].dpTableValue)
                        {
                            dpTableT[d].dpTableValue = dpTableTd.dpTableValue;
                            dpTableT[d].dpTableSeed = dpTableTd.dpTableSeed;
                        }
                    }
                }
            }

            Seed seed;

            seed.psi = -1;

            for (int d = 0; d < this->paramD; d++)
            {
                if (dpTableT[d].dpTableValue > NEG_INF)
                {
                    seed.psi = d;
                    break;
                }
            }

            seed.omega = (seed.psi == -1) ? NEG_INF : dpTableT[seed.psi].dpTableValue;

            if (seed.psi == -1)
            {
                seed.seed = "X";
            }
            else
            {
                seed.seed = decodeSeedString(dpTableT[seed.psi].dpTableSeed);
            }

            seeds.push_back(seed);
        }

        delete[] dpTableTdp;

        return seeds;
    }
}