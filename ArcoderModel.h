#ifndef ARCODERMODEL_H
#define ARCODERMODEL_H

#include <vector>
#include <map>
#include <set>
#include <limits.h>

class ArcoderModel
{
  static const int kNewSymbolCode = INT_MAX;
  static const int kEOFSymbolCode = kNewSymbolCode - 1;

  std::vector<int> m_cumulativeFreqs;
  std::vector<int> m_freqs;

  std::map<int, int> m_symbolToIndex;
  std::map<int, int> m_indexToSymbol;
  std::set<int> m_symbols;

  unsigned int m_lastIndex;

  void Reset();
  void AddSymbol(int i_symbol, int i_usageCount = 1);

public:

  ArcoderModel();
  ~ArcoderModel() {}

  void InitUniformModel(int i_symbolCount);

  void Update(int i_symbol);

  void CalculateBounds(int i_symbol,
                       unsigned long& io_lowerBound,
                       unsigned long& io_upperBound) const;
  int GetSymbol(unsigned long i_value,
                unsigned long& io_lowerBound,
                unsigned long& io_upperBound) const;

  int NewSymbolCode() const;
  int EOFSymbolCode() const;
  bool SymbolExists(int) const;

  void Load(const std::vector<int>& i_symbols,
            const std::vector<int>& i_freqs);
  void Save(std::vector<int>& o_symbols,
            std::vector<int>& o_freqs);
};

#endif // ARCODERMODEL_H
