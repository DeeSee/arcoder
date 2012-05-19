#include "AdaptiveModel.h"

static const unsigned int kSymbolsBeforeRestart = 20000;

/*****************************************************************************/
void AdaptiveModel::BuildZeroLevelModel(int symbolsCount)
{
  m_freq.assign(symbolsCount, 1);

  m_freq.push_back(symbolsCount);
}

/*****************************************************************************/
AdaptiveModel::AdaptiveModel(int symbolsCount)
{
  BuildZeroLevelModel(symbolsCount);
}

/*****************************************************************************/
void AdaptiveModel::Update(int symbol)
{
    m_freq[symbol]++;
    m_freq.back()++;

    while (m_freq.back() > kSymbolsBeforeRestart)
    {
      for (unsigned int i = 0; i < m_freq.size(); i++)
      {
        m_freq[i] >>= 1;
      }

      m_freq.back() = 0;

      for (unsigned int i = 0; i < m_freq.size() - 1; i++)
      {
        if (m_freq[i] == 0)
        {
          m_freq[i]++;
        }

        m_freq.back() += m_freq[i];
      }

    }
}

/*****************************************************************************/
unsigned long AdaptiveModel::UpperBound(int symbol,
                                        const unsigned long& range)
{
  unsigned int ub = 0;

  for (int i = 0; i <= symbol; i++)
  {
    ub += m_freq[i];
  }

  return range * ub / m_freq.back() - 1;
}

/*****************************************************************************/
unsigned long AdaptiveModel::LowerBound(int symbol,
                                        const unsigned long& range)
{
  unsigned int lb = 0;

  for (int i = 0; i < symbol; i++)
  {
    lb += m_freq[i];
  }

  return range * lb / m_freq.back();
}

/*****************************************************************************/
int AdaptiveModel::SymbolsCount()
{
  return m_freq.back();
}

/*****************************************************************************/
int AdaptiveModel::GetSymbol(const unsigned long& cumulativem_frequency)
{
  int result;
  unsigned int sum = 0;

  for (result = 0; (sum += m_freq[result]) <= cumulativem_frequency; result++);

  return result;

}

/*****************************************************************************/
void AdaptiveModel::LoadFromFile(FILE* file)
{
  if (file == NULL)
  {
    throw my_exception(
          "AdaptiveModel.LoadFromFile: received NULL file pointer");
  }

  LoadZeroLevel(file);
}

/*****************************************************************************/
void AdaptiveModel::LoadZeroLevel(FILE *in)
{
  for (unsigned int i = 0; i < m_freq.size() - 1; i++)
  {
    if (!fread(&m_freq[i], sizeof(unsigned int), 1, in))
    {
      throw my_exception(
            "AdaptiveModel.LoadZeroLevel: couldn't read from file");
    }

//    if (i > 0 && m_freq[i] <= m_freq[i - 1])
//    {
//      throw my_exception(
//            "Invalid model file");
//    }

    m_freq.back() += m_freq[i];
  }
}

/*****************************************************************************/
void AdaptiveModel::SaveToFile(FILE* file)
{
  if (file == NULL)
  {
    throw my_exception(
          "AdaptiveModel.SaveToFile: received NULL file pointer");
  }

  SaveZeroLevel(file);
}

/*****************************************************************************/
void AdaptiveModel::SaveZeroLevel(FILE *out)
{
//  while (m_freq.back() > USHRT_MAX)
//  {
//    for (unsigned int i = 0; i < m_freq.size(); i++)
//    {
//      m_freq[i] >>= 1;
//    }

//    for (unsigned int i = 1; i < m_freq.size(); i++)
//    {
//      if (m_freq[i] == m_freq[i - 1])
//      {
//        for (unsigned int j = i; j < m_freq.size(); j++)
//        {
//          m_freq[j]++;
//        }
//      }
//    }
//  }

  for (unsigned int i = 1; i < m_freq.size() - 1; i++)
  {
    if (!fwrite(&m_freq[i], sizeof(unsigned int), 1, out))
    {
      throw my_exception(
            "AdaptiveModel.SaveZeroLevel: couldn't write to file");
    }
  }
}
