#ifndef ADAPTIVE_MODEL_H
#define ADAPTIVE_MODEL_H

#include <vector>
#include <queue>
#include <iostream>
#include "my_exception.h"
#include <math.h>
#include <stdio.h>
#include <limits.h>

using namespace std;

class AdaptiveModel
{
    vector<unsigned int> m_freq;

    AdaptiveModel(); // Disable default constructor

    void BuildZeroLevelModel(int symbolsCount);

    void LoadZeroLevel(FILE* in);
    void SaveZeroLevel(FILE* out);

public:
    AdaptiveModel(int symbolsCount);

    void LoadFromFile(FILE* file);
    void SaveToFile(FILE* file);

    int GetSymbol(const unsigned long& cumulativeFrequency);

    void Update(int symbol);

    unsigned long UpperBound(int symbol,
                             const unsigned long& range);

    unsigned long LowerBound(int symbol,
                             const unsigned long& range);

    int SymbolsCount();

};

#endif // ADAPTIVE_MODEL_H
