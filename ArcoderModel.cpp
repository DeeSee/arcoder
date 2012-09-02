#include "ArcoderModel.h"
#include "assert.h"

/**************************************************************************************************/
ArcoderModel::ArcoderModel()
{
  Reset();
}

/**************************************************************************************************/
void ArcoderModel::Reset()
{
  m_cumulativeFreqs.clear();
//  m_freqs.clear();

  m_cumulativeFreqs.push_back(0);

  m_symbolToIndex.clear();
  m_indexToSymbol.clear();
  m_symbols.clear();

  m_lastIndex = 1;

  AddSymbol(kNewSymbolCode);
  AddSymbol(kEOFSymbolCode);
}

/**************************************************************************************************/
void ArcoderModel::AddSymbol(int i_symbol, int i_usageCount)
{
  assert(!SymbolExists(i_symbol));

  m_symbolToIndex[i_symbol] = m_lastIndex;
  m_indexToSymbol[m_lastIndex] = i_symbol;
  m_symbols.insert(i_symbol);

//  m_freqs.push_back(i_usageCount)

  m_cumulativeFreqs.push_back(0);
  for (unsigned int i = 0; i < m_lastIndex; i++)
  {
    m_cumulativeFreqs[i] += i_usageCount;
  };

  m_lastIndex++;
}

/**************************************************************************************************/
void ArcoderModel::InitUniformModel(int i_symbolCount)
{
  Reset();

  for (int i = 0; i < i_symbolCount; i++)
  {
    AddSymbol(i);
  }
}

/**************************************************************************************************/
void ArcoderModel::Update(int i_symbol)
{
  if (i_symbol == kNewSymbolCode ||
      i_symbol == kEOFSymbolCode)
  {
    // Avoid updating new symbol and EOF codes
    // because their usage frequencies are not used
    // in Save and Load methods
    return;
  }

  if (SymbolExists(i_symbol))
  {
//    m_freqs[m_symbolToIndex[i_symbol]]++;
    for (int i = 0; i < m_symbolToIndex[i_symbol]; i++)
    {
      m_cumulativeFreqs[i]++;
    }
  }
  else
  {
    AddSymbol(i_symbol);
  }
}

/**************************************************************************************************/
void ArcoderModel::CalculateBounds(int i_symbol,
                                   unsigned long& io_lowerBound,
                                   unsigned long& io_upperBound) const
{
  assert(SymbolExists(i_symbol));

  unsigned long long range = io_upperBound - io_lowerBound + 1;

  std::map<int, int>::const_iterator it = m_symbolToIndex.find(i_symbol);

  assert(it != m_symbolToIndex.end());

  unsigned int symbolIndex = (*it).second;

  assert(m_cumulativeFreqs.size() > symbolIndex);

  io_upperBound = (unsigned long)(io_lowerBound +
                  range * m_cumulativeFreqs[symbolIndex - 1] / m_cumulativeFreqs[0] - 1);
  io_lowerBound += (unsigned long)(range * m_cumulativeFreqs[symbolIndex] / m_cumulativeFreqs[0]);
}

/**************************************************************************************************/
int ArcoderModel::GetSymbol(unsigned long i_value,
                            unsigned long &io_lowerBound,
                            unsigned long &io_upperBound) const
{
  assert(io_lowerBound <= i_value && i_value <= io_upperBound);

//    декодирование одного символа по указанной таблице вероятностей
//    (возвращает индекс символа в таблице)
//   DWORD range, cum;
   unsigned long long range = io_upperBound - io_lowerBound + 1;

   // число cum - это число value, пересчитанное из интервала
   // low..high в интервал 0..context[0]

   int cumulativeFreq = (int)((((unsigned long long ) (i_value - io_lowerBound + 1)) * m_cumulativeFreqs[0] - 1)
						/ range);

   int symbolIndex = 1;
   // поиск интервала, соответствующего числу cum (начиная с самого частого символа)
   while (m_cumulativeFreqs[symbolIndex] > cumulativeFreq)
   {
     symbolIndex++;
   }

   // пересчет границ интервала арифметического кодирования
   // symbol==1 - самый частый символ
   io_upperBound = (unsigned long)(io_lowerBound + range * m_cumulativeFreqs[symbolIndex - 1] / m_cumulativeFreqs[0] - 1);

   io_lowerBound += (unsigned long)(range * m_cumulativeFreqs[symbolIndex] / m_cumulativeFreqs[0]);

   std::map<int, int>::const_iterator it = m_indexToSymbol.find(symbolIndex);

   assert(it != m_indexToSymbol.end());

   int result = (*it).second;

   assert(SymbolExists(result));

   return result;
}

/**************************************************************************************************/
int ArcoderModel::NewSymbolCode() const
{
  return kNewSymbolCode;
}

/**************************************************************************************************/
int ArcoderModel::EOFSymbolCode() const
{
  return kEOFSymbolCode;
}

/**************************************************************************************************/
bool ArcoderModel::SymbolExists(int i_symbol) const
{
  return m_symbols.find(i_symbol) != m_symbols.end();
}

/**************************************************************************************************/
void ArcoderModel::Load(const std::vector<int>& i_symbols,
                        const std::vector<int>& i_freqs)
{
  Reset();

  // Used to sort frequencies in ascending order
  std::map<int, int> sortingMap;

  for (unsigned int i = 0; i < i_freqs.size(); i++)
  {
    assert(i_symbols[i] != kNewSymbolCode && i_symbols[i] != kEOFSymbolCode);

    sortingMap[i_freqs[i]] = i_symbols[i];
  }

  for (std::map<int, int>::reverse_iterator it = sortingMap.rbegin(); it != sortingMap.rend(); it++)
  {
    AddSymbol(it->second, it->first);
  }
}

/**************************************************************************************************/
void ArcoderModel::Save(std::vector<int> &o_symbols,
                        std::vector<int> &o_freqs)
{
  o_symbols.clear();
  o_freqs.clear();

  for (std::set<int>::iterator it = m_symbols.begin(); it != m_symbols.end(); it++)
  {
    if (*it != kNewSymbolCode &&
        *it != kEOFSymbolCode)
    {
      int index = m_symbolToIndex[*it];

      int freq = m_cumulativeFreqs[index - 1] - m_cumulativeFreqs[index];

      o_symbols.push_back(*it);
      o_freqs.push_back(freq);
    }
  }
}

