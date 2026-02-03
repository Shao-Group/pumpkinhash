#include "pumpkinhash.hpp"

PumpkinHash::PumpkinHash() : PumpkinHash(20, 11, {{'A', 0}, {'C', 1}, {'G', 2}, {'T', 3}})
{
    // Calling parameterized constructor from default constructor with default arguments
}

PumpkinHash::PumpkinHash(const int windowSizeN, const int paramD, const map<char, int> alphabet)
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
    random_device seedSource;

    mt19937 pseudoRandomNumberEngine(seedSource());

    uniform_int_distribution<int> uniformIntegerDistribution(1 << 5, 1 << 10);

    int tableANumbersSum = 0, tableANumbersCount = this->windowSizeN * this->paramD * this->alphabet.size();

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
                this->tableB1[n * this->paramD * this->alphabet.size() + d * this->alphabet.size() + sigma] = (pairValues[sigma] >> 1) % 2;
                this->tableB2[n * this->paramD * this->alphabet.size() + d * this->alphabet.size() + sigma] = (pairValues[sigma] >> 0) % 2;
            }
        }
    }

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

    string tablesFileFolderPath = string("..") + filesystem::path::preferred_separator + string("tables");

    filesystem::path tablesFileFolder(tablesFileFolderPath);

    if (!filesystem::exists(tablesFileFolder))
    {
        if (!filesystem::create_directories(tablesFileFolder))
        {
            exit(EXIT_FAILURE);
        }
    }

    string tablesFilePath = tablesFileFolderPath + filesystem::path::preferred_separator + string("tables_N") + to_string(this->windowSizeN) + string("_D") + to_string(this->paramD) + string("_Sigma") + to_string(this->alphabet.size());

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

    for (int n = 0; n < this->windowSizeN; n++)
    {
        for (int sigma = 0; sigma < this->alphabet.size(); sigma++)
        {
            tablesFile << this->tableC[n * this->alphabet.size() + sigma] << " ";
        }

        tablesFile << endl;
    }

    tablesFile.close();

    return;
}

void PumpkinHash::loadTables()
{
    string tablesFilePath = string("..") + filesystem::path::preferred_separator + string("tables") + filesystem::path::preferred_separator + string("tables_N") + to_string(this->windowSizeN) + string("_D") + to_string(this->paramD) + string("_Sigma") + to_string(this->alphabet.size());

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

    for (int n = 0; n < this->windowSizeN; n++)
    {
        for (int sigma = 0; sigma < this->alphabet.size(); sigma++)
        {
            tablesFile >> this->tableC[n * this->alphabet.size() + sigma];
        }
    }

    tablesFile.close();

    return;
}

Seed PumpkinHash::solveDP(const string sequence, const int numMaxEditsE)
{
    if (sequence.length() != this->windowSizeN)
    {
        exit(EXIT_FAILURE);
    }

    DpTableCell *dpTableTmin = new DpTableCell[(this->windowSizeN + 1) * (numMaxEditsE + 1) * this->paramD];
    DpTableCell *dpTableTmax = new DpTableCell[(this->windowSizeN + 1) * (numMaxEditsE + 1) * this->paramD];

    for (int e = 0; e <= numMaxEditsE; e++)
    {
        for (int d = 0; d < this->paramD; d++)
        {
            if (e == 0 && d == 0)
            {
                dpTableTmin[0 * (numMaxEditsE + 1) * this->paramD + e * this->paramD + d].dpTableValue = 0;
                dpTableTmax[0 * (numMaxEditsE + 1) * this->paramD + e * this->paramD + d].dpTableValue = 0;

                dpTableTmin[0 * (numMaxEditsE + 1) * this->paramD + e * this->paramD + d].dpTableSeed = 0;
                dpTableTmax[0 * (numMaxEditsE + 1) * this->paramD + e * this->paramD + d].dpTableSeed = 0;
            }
            else
            {
                dpTableTmin[0 * (numMaxEditsE + 1) * this->paramD + e * this->paramD + d].dpTableValue = POS_INF;
                dpTableTmax[0 * (numMaxEditsE + 1) * this->paramD + e * this->paramD + d].dpTableValue = NEG_INF;

                dpTableTmin[0 * (numMaxEditsE + 1) * this->paramD + e * this->paramD + d].dpTableSeed = UINT64_MAX;
                dpTableTmax[0 * (numMaxEditsE + 1) * this->paramD + e * this->paramD + d].dpTableSeed = UINT64_MAX;
            }
        }
    }

    for (int n = 1; n <= this->windowSizeN; n++)
    {
        for (int e = 0; e <= numMaxEditsE; e++)
        {
            for (int d = 0; d < this->paramD; d++)
            {
                // Case-1: base s_n deleted
                DpTableCell dpTableTminCase1Del, dpTableTmaxCase1Del;

                dpTableTminCase1Del.dpTableValue = POS_INF;
                dpTableTmaxCase1Del.dpTableValue = NEG_INF;

                dpTableTminCase1Del.dpTableSeed = UINT64_MAX;
                dpTableTmaxCase1Del.dpTableSeed = UINT64_MAX;

                if (e != 0)
                {
                    dpTableTminCase1Del.dpTableValue = dpTableTmin[(n - 1) * (numMaxEditsE + 1) * this->paramD + (e - 1) * this->paramD + d].dpTableValue;
                    dpTableTmaxCase1Del.dpTableValue = dpTableTmax[(n - 1) * (numMaxEditsE + 1) * this->paramD + (e - 1) * this->paramD + d].dpTableValue;

                    dpTableTminCase1Del.dpTableSeed = (dpTableTmin[(n - 1) * (numMaxEditsE + 1) * this->paramD + (e - 1) * this->paramD + d].dpTableSeed == UINT64_MAX) ? UINT64_MAX : (dpTableTmin[(n - 1) * (numMaxEditsE + 1) * this->paramD + (e - 1) * this->paramD + d].dpTableSeed << 3) | this->alphabet.size();
                    dpTableTmaxCase1Del.dpTableSeed = (dpTableTmax[(n - 1) * (numMaxEditsE + 1) * this->paramD + (e - 1) * this->paramD + d].dpTableSeed == UINT64_MAX) ? UINT64_MAX : (dpTableTmax[(n - 1) * (numMaxEditsE + 1) * this->paramD + (e - 1) * this->paramD + d].dpTableSeed << 3) | this->alphabet.size();
                }

                // Case-2: base s_n retained
                DpTableCell dpTableTminCase2Ret, dpTableTmaxCase2Ret;

                dpTableTminCase2Ret.dpTableValue = this->tableA[(n - 1) * this->paramD * this->alphabet.size() + d * this->alphabet.size() + this->alphabet[sequence[n - 1]]] * this->tableB2[(n - 1) * this->paramD * this->alphabet.size() + d * this->alphabet.size() + this->alphabet[sequence[n - 1]]];
                dpTableTmaxCase2Ret.dpTableValue = this->tableA[(n - 1) * this->paramD * this->alphabet.size() + d * this->alphabet.size() + this->alphabet[sequence[n - 1]]] * this->tableB2[(n - 1) * this->paramD * this->alphabet.size() + d * this->alphabet.size() + this->alphabet[sequence[n - 1]]];

                int dPrevious = (d - this->tableC[(n - 1) * this->alphabet.size() + this->alphabet[sequence[n - 1]]] + this->paramD) % this->paramD;

                if (this->tableB1[(n - 1) * this->paramD * this->alphabet.size() + d * this->alphabet.size() + this->alphabet[sequence[n - 1]]] == 1)
                {
                    dpTableTminCase2Ret.dpTableValue = (dpTableTmin[(n - 1) * (numMaxEditsE + 1) * this->paramD + e * this->paramD + dPrevious].dpTableValue == POS_INF) ? POS_INF : (dpTableTminCase2Ret.dpTableValue + dpTableTmin[(n - 1) * (numMaxEditsE + 1) * this->paramD + e * this->paramD + dPrevious].dpTableValue);
                    dpTableTmaxCase2Ret.dpTableValue = (dpTableTmax[(n - 1) * (numMaxEditsE + 1) * this->paramD + e * this->paramD + dPrevious].dpTableValue == NEG_INF) ? NEG_INF : (dpTableTmaxCase2Ret.dpTableValue + dpTableTmax[(n - 1) * (numMaxEditsE + 1) * this->paramD + e * this->paramD + dPrevious].dpTableValue);

                    dpTableTminCase2Ret.dpTableSeed = (dpTableTmin[(n - 1) * (numMaxEditsE + 1) * this->paramD + e * this->paramD + dPrevious].dpTableSeed == UINT64_MAX) ? UINT64_MAX : (dpTableTmin[(n - 1) * (numMaxEditsE + 1) * this->paramD + e * this->paramD + dPrevious].dpTableSeed << 3) | this->alphabet[sequence[n - 1]];
                    dpTableTmaxCase2Ret.dpTableSeed = (dpTableTmax[(n - 1) * (numMaxEditsE + 1) * this->paramD + e * this->paramD + dPrevious].dpTableSeed == UINT64_MAX) ? UINT64_MAX : (dpTableTmax[(n - 1) * (numMaxEditsE + 1) * this->paramD + e * this->paramD + dPrevious].dpTableSeed << 3) | this->alphabet[sequence[n - 1]];
                }
                else
                {
                    dpTableTminCase2Ret.dpTableValue = (dpTableTmax[(n - 1) * (numMaxEditsE + 1) * this->paramD + e * this->paramD + dPrevious].dpTableValue == NEG_INF) ? POS_INF : (dpTableTminCase2Ret.dpTableValue - dpTableTmax[(n - 1) * (numMaxEditsE + 1) * this->paramD + e * this->paramD + dPrevious].dpTableValue);
                    dpTableTmaxCase2Ret.dpTableValue = (dpTableTmin[(n - 1) * (numMaxEditsE + 1) * this->paramD + e * this->paramD + dPrevious].dpTableValue == POS_INF) ? NEG_INF : (dpTableTmaxCase2Ret.dpTableValue - dpTableTmin[(n - 1) * (numMaxEditsE + 1) * this->paramD + e * this->paramD + dPrevious].dpTableValue);

                    dpTableTminCase2Ret.dpTableSeed = (dpTableTmax[(n - 1) * (numMaxEditsE + 1) * this->paramD + e * this->paramD + dPrevious].dpTableSeed == UINT64_MAX) ? UINT64_MAX : (dpTableTmax[(n - 1) * (numMaxEditsE + 1) * this->paramD + e * this->paramD + dPrevious].dpTableSeed << 3) | this->alphabet[sequence[n - 1]];
                    dpTableTmaxCase2Ret.dpTableSeed = (dpTableTmin[(n - 1) * (numMaxEditsE + 1) * this->paramD + e * this->paramD + dPrevious].dpTableSeed == UINT64_MAX) ? UINT64_MAX : (dpTableTmin[(n - 1) * (numMaxEditsE + 1) * this->paramD + e * this->paramD + dPrevious].dpTableSeed << 3) | this->alphabet[sequence[n - 1]];
                }

                // Case-3: base s_n substituted with s'_n
                DpTableCell dpTableTminCase3Sub, dpTableTmaxCase3Sub;

                dpTableTminCase3Sub.dpTableValue = POS_INF;
                dpTableTmaxCase3Sub.dpTableValue = NEG_INF;

                dpTableTminCase3Sub.dpTableSeed = UINT64_MAX;
                dpTableTmaxCase3Sub.dpTableSeed = UINT64_MAX;

                if (e != 0)
                {
                    for (auto const &pair : this->alphabet)
                    {
                        if (pair.first == sequence[n - 1])
                        {
                            continue;
                        }

                        DpTableCell dpTableTminCase3SubTemp, dpTableTmaxCase3SubTemp;

                        dpTableTminCase3SubTemp.dpTableValue = this->tableA[(n - 1) * this->paramD * this->alphabet.size() + d * this->alphabet.size() + pair.second] * this->tableB2[(n - 1) * this->paramD * this->alphabet.size() + d * this->alphabet.size() + pair.second];
                        dpTableTmaxCase3SubTemp.dpTableValue = this->tableA[(n - 1) * this->paramD * this->alphabet.size() + d * this->alphabet.size() + pair.second] * this->tableB2[(n - 1) * this->paramD * this->alphabet.size() + d * this->alphabet.size() + pair.second];

                        dPrevious = (d - this->tableC[(n - 1) * this->alphabet.size() + pair.second] + this->paramD) % this->paramD;

                        if (this->tableB1[(n - 1) * this->paramD * this->alphabet.size() + d * this->alphabet.size() + pair.second] == 1)
                        {
                            dpTableTminCase3SubTemp.dpTableValue = (dpTableTmin[(n - 1) * (numMaxEditsE + 1) * this->paramD + (e - 1) * this->paramD + dPrevious].dpTableValue == POS_INF) ? POS_INF : (dpTableTminCase3SubTemp.dpTableValue + dpTableTmin[(n - 1) * (numMaxEditsE + 1) * this->paramD + (e - 1) * this->paramD + dPrevious].dpTableValue);
                            dpTableTmaxCase3SubTemp.dpTableValue = (dpTableTmax[(n - 1) * (numMaxEditsE + 1) * this->paramD + (e - 1) * this->paramD + dPrevious].dpTableValue == NEG_INF) ? NEG_INF : (dpTableTmaxCase3SubTemp.dpTableValue + dpTableTmax[(n - 1) * (numMaxEditsE + 1) * this->paramD + (e - 1) * this->paramD + dPrevious].dpTableValue);

                            dpTableTminCase3SubTemp.dpTableSeed = (dpTableTmin[(n - 1) * (numMaxEditsE + 1) * this->paramD + (e - 1) * this->paramD + dPrevious].dpTableSeed == UINT64_MAX) ? UINT64_MAX : (dpTableTmin[(n - 1) * (numMaxEditsE + 1) * this->paramD + (e - 1) * this->paramD + dPrevious].dpTableSeed << 3) | pair.second;
                            dpTableTmaxCase3SubTemp.dpTableSeed = (dpTableTmax[(n - 1) * (numMaxEditsE + 1) * this->paramD + (e - 1) * this->paramD + dPrevious].dpTableSeed == UINT64_MAX) ? UINT64_MAX : (dpTableTmax[(n - 1) * (numMaxEditsE + 1) * this->paramD + (e - 1) * this->paramD + dPrevious].dpTableSeed << 3) | pair.second;
                        }
                        else
                        {
                            dpTableTminCase3SubTemp.dpTableValue = (dpTableTmax[(n - 1) * (numMaxEditsE + 1) * this->paramD + (e - 1) * this->paramD + dPrevious].dpTableValue == NEG_INF) ? POS_INF : (dpTableTminCase3SubTemp.dpTableValue - dpTableTmax[(n - 1) * (numMaxEditsE + 1) * this->paramD + (e - 1) * this->paramD + dPrevious].dpTableValue);
                            dpTableTmaxCase3SubTemp.dpTableValue = (dpTableTmin[(n - 1) * (numMaxEditsE + 1) * this->paramD + (e - 1) * this->paramD + dPrevious].dpTableValue == POS_INF) ? NEG_INF : (dpTableTmaxCase3SubTemp.dpTableValue - dpTableTmin[(n - 1) * (numMaxEditsE + 1) * this->paramD + (e - 1) * this->paramD + dPrevious].dpTableValue);

                            dpTableTminCase3SubTemp.dpTableSeed = (dpTableTmax[(n - 1) * (numMaxEditsE + 1) * this->paramD + (e - 1) * this->paramD + dPrevious].dpTableSeed == UINT64_MAX) ? UINT64_MAX : (dpTableTmax[(n - 1) * (numMaxEditsE + 1) * this->paramD + (e - 1) * this->paramD + dPrevious].dpTableSeed << 3) | pair.second;
                            dpTableTmaxCase3SubTemp.dpTableSeed = (dpTableTmin[(n - 1) * (numMaxEditsE + 1) * this->paramD + (e - 1) * this->paramD + dPrevious].dpTableSeed == UINT64_MAX) ? UINT64_MAX : (dpTableTmin[(n - 1) * (numMaxEditsE + 1) * this->paramD + (e - 1) * this->paramD + dPrevious].dpTableSeed << 3) | pair.second;
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

                if (dpTableTminCase1Del.dpTableValue < dpTableTminCase2Ret.dpTableValue)
                {
                    if (dpTableTminCase1Del.dpTableValue < dpTableTminCase3Sub.dpTableValue)
                    {
                        dpTableTmin[n * (numMaxEditsE + 1) * this->paramD + e * this->paramD + d].dpTableValue = dpTableTminCase1Del.dpTableValue;
                        dpTableTmin[n * (numMaxEditsE + 1) * this->paramD + e * this->paramD + d].dpTableSeed = dpTableTminCase1Del.dpTableSeed;
                    }
                    else
                    {
                        dpTableTmin[n * (numMaxEditsE + 1) * this->paramD + e * this->paramD + d].dpTableValue = dpTableTminCase3Sub.dpTableValue;
                        dpTableTmin[n * (numMaxEditsE + 1) * this->paramD + e * this->paramD + d].dpTableSeed = dpTableTminCase3Sub.dpTableSeed;
                    }
                }
                else
                {
                    if (dpTableTminCase2Ret.dpTableValue < dpTableTminCase3Sub.dpTableValue)
                    {
                        dpTableTmin[n * (numMaxEditsE + 1) * this->paramD + e * this->paramD + d].dpTableValue = dpTableTminCase2Ret.dpTableValue;
                        dpTableTmin[n * (numMaxEditsE + 1) * this->paramD + e * this->paramD + d].dpTableSeed = dpTableTminCase2Ret.dpTableSeed;
                    }
                    else
                    {
                        dpTableTmin[n * (numMaxEditsE + 1) * this->paramD + e * this->paramD + d].dpTableValue = dpTableTminCase3Sub.dpTableValue;
                        dpTableTmin[n * (numMaxEditsE + 1) * this->paramD + e * this->paramD + d].dpTableSeed = dpTableTminCase3Sub.dpTableSeed;
                    }
                }

                if (dpTableTmaxCase1Del.dpTableValue > dpTableTmaxCase2Ret.dpTableValue)
                {
                    if (dpTableTmaxCase1Del.dpTableValue > dpTableTmaxCase3Sub.dpTableValue)
                    {
                        dpTableTmax[n * (numMaxEditsE + 1) * this->paramD + e * this->paramD + d].dpTableValue = dpTableTmaxCase1Del.dpTableValue;
                        dpTableTmax[n * (numMaxEditsE + 1) * this->paramD + e * this->paramD + d].dpTableSeed = dpTableTmaxCase1Del.dpTableSeed;
                    }
                    else
                    {
                        dpTableTmax[n * (numMaxEditsE + 1) * this->paramD + e * this->paramD + d].dpTableValue = dpTableTmaxCase3Sub.dpTableValue;
                        dpTableTmax[n * (numMaxEditsE + 1) * this->paramD + e * this->paramD + d].dpTableSeed = dpTableTmaxCase3Sub.dpTableSeed;
                    }
                }
                else
                {
                    if (dpTableTmaxCase2Ret.dpTableValue > dpTableTmaxCase3Sub.dpTableValue)
                    {
                        dpTableTmax[n * (numMaxEditsE + 1) * this->paramD + e * this->paramD + d].dpTableValue = dpTableTmaxCase2Ret.dpTableValue;
                        dpTableTmax[n * (numMaxEditsE + 1) * this->paramD + e * this->paramD + d].dpTableSeed = dpTableTmaxCase2Ret.dpTableSeed;
                    }
                    else
                    {
                        dpTableTmax[n * (numMaxEditsE + 1) * this->paramD + e * this->paramD + d].dpTableValue = dpTableTmaxCase3Sub.dpTableValue;
                        dpTableTmax[n * (numMaxEditsE + 1) * this->paramD + e * this->paramD + d].dpTableSeed = dpTableTmaxCase3Sub.dpTableSeed;
                    }
                }
            }
        }
    }

    DpTableCell dpTableT[this->paramD];

    for (int d = 0; d < this->paramD; d++)
    {
        dpTableT[d].dpTableValue = NEG_INF;
        dpTableT[d].dpTableSeed = UINT64_MAX;

        for (int e = 0; e <= numMaxEditsE; e++)
        {
            DpTableCell dpTableTd;

            if (dpTableTmin[this->windowSizeN * (numMaxEditsE + 1) * this->paramD + e * this->paramD + d].dpTableValue == POS_INF && dpTableTmax[this->windowSizeN * (numMaxEditsE + 1) * this->paramD + e * this->paramD + d].dpTableValue == NEG_INF)
            {
                dpTableTd.dpTableValue = NEG_INF;
                dpTableTd.dpTableSeed = UINT64_MAX;
            }
            else if (dpTableTmin[this->windowSizeN * (numMaxEditsE + 1) * this->paramD + e * this->paramD + d].dpTableValue < POS_INF && dpTableTmax[this->windowSizeN * (numMaxEditsE + 1) * this->paramD + e * this->paramD + d].dpTableValue == NEG_INF)
            {
                dpTableTd.dpTableValue = abs(dpTableTmin[this->windowSizeN * (numMaxEditsE + 1) * this->paramD + e * this->paramD + d].dpTableValue);
                dpTableTd.dpTableSeed = dpTableTmin[this->windowSizeN * (numMaxEditsE + 1) * this->paramD + e * this->paramD + d].dpTableSeed;
            }
            else if (dpTableTmin[this->windowSizeN * (numMaxEditsE + 1) * this->paramD + e * this->paramD + d].dpTableValue == POS_INF && dpTableTmax[this->windowSizeN * (numMaxEditsE + 1) * this->paramD + e * this->paramD + d].dpTableValue > NEG_INF)
            {
                dpTableTd.dpTableValue = abs(dpTableTmax[this->windowSizeN * (numMaxEditsE + 1) * this->paramD + e * this->paramD + d].dpTableValue);
                dpTableTd.dpTableSeed = dpTableTmax[this->windowSizeN * (numMaxEditsE + 1) * this->paramD + e * this->paramD + d].dpTableSeed;
            }
            else
            {
                if (abs(dpTableTmin[this->windowSizeN * (numMaxEditsE + 1) * this->paramD + e * this->paramD + d].dpTableValue) > abs(dpTableTmax[this->windowSizeN * (numMaxEditsE + 1) * this->paramD + e * this->paramD + d].dpTableValue))
                {
                    dpTableTd.dpTableValue = abs(dpTableTmin[this->windowSizeN * (numMaxEditsE + 1) * this->paramD + e * this->paramD + d].dpTableValue);
                    dpTableTd.dpTableSeed = dpTableTmin[this->windowSizeN * (numMaxEditsE + 1) * this->paramD + e * this->paramD + d].dpTableSeed;
                }
                else
                {
                    dpTableTd.dpTableValue = abs(dpTableTmax[this->windowSizeN * (numMaxEditsE + 1) * this->paramD + e * this->paramD + d].dpTableValue);
                    dpTableTd.dpTableSeed = dpTableTmax[this->windowSizeN * (numMaxEditsE + 1) * this->paramD + e * this->paramD + d].dpTableSeed;
                }
            }

            if (dpTableTd.dpTableValue > dpTableT[d].dpTableValue)
            {
                dpTableT[d].dpTableValue = dpTableTd.dpTableValue;
                dpTableT[d].dpTableSeed = dpTableTd.dpTableSeed;
            }
        }
    }

    delete[] dpTableTmin;
    delete[] dpTableTmax;

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
        seed.seed = "";

        uint64_t dpTableTSeedCopy = dpTableT[seed.psi].dpTableSeed;

        map<int, char> reverseAlphabet;

        for (auto const &pair : this->alphabet)
        {
            reverseAlphabet[pair.second] = pair.first;
        }

        reverseAlphabet[this->alphabet.size()] = '-';

        for (int n = 0; n < this->windowSizeN; n++)
        {
            seed.seed = reverseAlphabet[dpTableTSeedCopy & 7] + seed.seed;

            dpTableTSeedCopy = dpTableTSeedCopy >> 3;
        }
    }

    return seed;
}