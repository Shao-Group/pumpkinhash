#include "pumpkinhash.hpp"

PumpkinHash::PumpkinHash() : PumpkinHash(5, 11, {{'A', 0}, {'C', 1}, {'G', 2}, {'T', 3}})
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

    return;
}

Seed PumpkinHash::solveDP(const string sequence, const int numMaxEditsE)
{
    if (sequence.length() != this->windowSizeN)
    {
        exit(EXIT_FAILURE);
    }

    int *dpTableTmin = new int[(this->windowSizeN + 1) * (numMaxEditsE + 1) * this->paramD];
    int *dpTableTmax = new int[(this->windowSizeN + 1) * (numMaxEditsE + 1) * this->paramD];

    for (int e = 0; e <= numMaxEditsE; e++)
    {
        for (int d = 0; d < this->paramD; d++)
        {
            if (e == 0 && d == 0)
            {
                dpTableTmin[0 * (numMaxEditsE + 1) * this->paramD + e * this->paramD + d] = 0;
                dpTableTmax[0 * (numMaxEditsE + 1) * this->paramD + e * this->paramD + d] = 0;
            }
            else
            {
                dpTableTmin[0 * (numMaxEditsE + 1) * this->paramD + e * this->paramD + d] = POS_INF;
                dpTableTmax[0 * (numMaxEditsE + 1) * this->paramD + e * this->paramD + d] = NEG_INF;
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
                int dpTableTminCase1Del = POS_INF, dpTableTmaxCase1Del = NEG_INF;

                if (e != 0)
                {
                    dpTableTminCase1Del = dpTableTmin[(n - 1) * (numMaxEditsE + 1) * this->paramD + (e - 1) * this->paramD + d];
                    dpTableTmaxCase1Del = dpTableTmax[(n - 1) * (numMaxEditsE + 1) * this->paramD + (e - 1) * this->paramD + d];
                }

                // Case-2: base s_n retained
                int dpTableTminCase2Ret = this->tableA[(n - 1) * this->paramD * this->alphabet.size() + d * this->alphabet.size() + this->alphabet[sequence[n - 1]]] * this->tableB2[(n - 1) * this->paramD * this->alphabet.size() + d * this->alphabet.size() + this->alphabet[sequence[n - 1]]], dpTableTmaxCase2Ret = this->tableA[(n - 1) * this->paramD * this->alphabet.size() + d * this->alphabet.size() + this->alphabet[sequence[n - 1]]] * this->tableB2[(n - 1) * this->paramD * this->alphabet.size() + d * this->alphabet.size() + this->alphabet[sequence[n - 1]]];

                int dPrevious = (d - this->tableC[(n - 1) * this->alphabet.size() + this->alphabet[sequence[n - 1]]] + this->paramD) % this->paramD;

                if (this->tableB1[(n - 1) * this->paramD * this->alphabet.size() + d * this->alphabet.size() + this->alphabet[sequence[n - 1]]] == 1)
                {
                    dpTableTminCase2Ret = (dpTableTmin[(n - 1) * (numMaxEditsE + 1) * this->paramD + e * this->paramD + dPrevious] == POS_INF) ? POS_INF : (dpTableTminCase2Ret + dpTableTmin[(n - 1) * (numMaxEditsE + 1) * this->paramD + e * this->paramD + dPrevious]);
                    dpTableTmaxCase2Ret = (dpTableTmax[(n - 1) * (numMaxEditsE + 1) * this->paramD + e * this->paramD + dPrevious] == NEG_INF) ? NEG_INF : (dpTableTmaxCase2Ret + dpTableTmax[(n - 1) * (numMaxEditsE + 1) * this->paramD + e * this->paramD + dPrevious]);
                }
                else
                {
                    dpTableTminCase2Ret = (dpTableTmax[(n - 1) * (numMaxEditsE + 1) * this->paramD + e * this->paramD + dPrevious] == NEG_INF) ? POS_INF : (dpTableTminCase2Ret - dpTableTmax[(n - 1) * (numMaxEditsE + 1) * this->paramD + e * this->paramD + dPrevious]);
                    dpTableTmaxCase2Ret = (dpTableTmin[(n - 1) * (numMaxEditsE + 1) * this->paramD + e * this->paramD + dPrevious] == POS_INF) ? NEG_INF : (dpTableTmaxCase2Ret - dpTableTmin[(n - 1) * (numMaxEditsE + 1) * this->paramD + e * this->paramD + dPrevious]);
                }

                // Case-3: base s_n substituted with s'_n
                int dpTableTminCase3Sub = POS_INF, dpTableTmaxCase3Sub = NEG_INF;

                if (e != 0)
                {
                    for (auto const &pair : this->alphabet)
                    {
                        if (pair.first == sequence[n - 1])
                        {
                            continue;
                        }

                        int dpTableTminCase3SubTemp = this->tableA[(n - 1) * this->paramD * this->alphabet.size() + d * this->alphabet.size() + pair.second] * this->tableB2[(n - 1) * this->paramD * this->alphabet.size() + d * this->alphabet.size() + pair.second], dpTableTmaxCase3SubTemp = this->tableA[(n - 1) * this->paramD * this->alphabet.size() + d * this->alphabet.size() + pair.second] * this->tableB2[(n - 1) * this->paramD * this->alphabet.size() + d * this->alphabet.size() + pair.second];

                        dPrevious = (d - this->tableC[(n - 1) * this->alphabet.size() + pair.second] + this->paramD) % this->paramD;

                        if (this->tableB1[(n - 1) * this->paramD * this->alphabet.size() + d * this->alphabet.size() + pair.second] == 1)
                        {
                            dpTableTminCase3SubTemp = (dpTableTmin[(n - 1) * (numMaxEditsE + 1) * this->paramD + (e - 1) * this->paramD + dPrevious] == POS_INF) ? POS_INF : (dpTableTminCase3SubTemp + dpTableTmin[(n - 1) * (numMaxEditsE + 1) * this->paramD + (e - 1) * this->paramD + dPrevious]);
                            dpTableTmaxCase3SubTemp = (dpTableTmax[(n - 1) * (numMaxEditsE + 1) * this->paramD + (e - 1) * this->paramD + dPrevious] == NEG_INF) ? NEG_INF : (dpTableTmaxCase3SubTemp + dpTableTmax[(n - 1) * (numMaxEditsE + 1) * this->paramD + (e - 1) * this->paramD + dPrevious]);
                        }
                        else
                        {
                            dpTableTminCase3SubTemp = (dpTableTmax[(n - 1) * (numMaxEditsE + 1) * this->paramD + (e - 1) * this->paramD + dPrevious] == NEG_INF) ? POS_INF : (dpTableTminCase3SubTemp - dpTableTmax[(n - 1) * (numMaxEditsE + 1) * this->paramD + (e - 1) * this->paramD + dPrevious]);
                            dpTableTmaxCase3SubTemp = (dpTableTmin[(n - 1) * (numMaxEditsE + 1) * this->paramD + (e - 1) * this->paramD + dPrevious] == POS_INF) ? NEG_INF : (dpTableTmaxCase3SubTemp - dpTableTmin[(n - 1) * (numMaxEditsE + 1) * this->paramD + (e - 1) * this->paramD + dPrevious]);
                        }

                        if (dpTableTminCase3SubTemp < dpTableTminCase3Sub)
                        {
                            dpTableTminCase3Sub = dpTableTminCase3SubTemp;
                        }

                        if (dpTableTmaxCase3SubTemp > dpTableTmaxCase3Sub)
                        {
                            dpTableTmaxCase3Sub = dpTableTmaxCase3SubTemp;
                        }
                    }
                }

                dpTableTmin[n * (numMaxEditsE + 1) * this->paramD + e * this->paramD + d] = min(dpTableTminCase1Del, min(dpTableTminCase2Ret, dpTableTminCase3Sub));
                dpTableTmax[n * (numMaxEditsE + 1) * this->paramD + e * this->paramD + d] = max(dpTableTmaxCase1Del, max(dpTableTmaxCase2Ret, dpTableTmaxCase3Sub));
            }
        }
    }

    int dpTableT[this->paramD];

    for (int d = 0; d < this->paramD; d++)
    {
        dpTableT[d] = NEG_INF;

        for (int e = 0, dpTableTd; e <= numMaxEditsE; e++)
        {
            if (dpTableTmin[this->windowSizeN * (numMaxEditsE + 1) * this->paramD + e * this->paramD + d] == POS_INF && dpTableTmax[this->windowSizeN * (numMaxEditsE + 1) * this->paramD + e * this->paramD + d] == NEG_INF)
            {
                dpTableTd = NEG_INF;
            }
            else if (dpTableTmin[this->windowSizeN * (numMaxEditsE + 1) * this->paramD + e * this->paramD + d] < POS_INF && dpTableTmax[this->windowSizeN * (numMaxEditsE + 1) * this->paramD + e * this->paramD + d] == NEG_INF)
            {
                dpTableTd = abs(dpTableTmin[this->windowSizeN * (numMaxEditsE + 1) * this->paramD + e * this->paramD + d]);
            }
            else if (dpTableTmin[this->windowSizeN * (numMaxEditsE + 1) * this->paramD + e * this->paramD + d] == POS_INF && dpTableTmax[this->windowSizeN * (numMaxEditsE + 1) * this->paramD + e * this->paramD + d] > NEG_INF)
            {
                dpTableTd = abs(dpTableTmax[this->windowSizeN * (numMaxEditsE + 1) * this->paramD + e * this->paramD + d]);
            }
            else
            {
                dpTableTd = max(abs(dpTableTmin[this->windowSizeN * (numMaxEditsE + 1) * this->paramD + e * this->paramD + d]), abs(dpTableTmax[this->windowSizeN * (numMaxEditsE + 1) * this->paramD + e * this->paramD + d]));
            }

            if (dpTableTd > dpTableT[d])
            {
                dpTableT[d] = dpTableTd;
            }
        }
    }

    delete[] dpTableTmin;
    delete[] dpTableTmax;

    Seed seed;

    seed.psi = -1;

    for (int d = 0; d < this->paramD; d++)
    {
        if (dpTableT[d] > NEG_INF)
        {
            seed.psi = d;
            break;
        }
    }

    seed.omega = (seed.psi == -1) ? NEG_INF : dpTableT[seed.psi];

    return seed;
}