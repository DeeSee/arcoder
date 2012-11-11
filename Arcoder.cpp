#include "Arcoder.h"

#include "vector"
#include "assert.h"

#include "ArcoderModel.h"
#include "BitStream.h"

/* для избежания переполнения:	MAX_FREQUENCY * (TOP_VALUE+1) < ULONG_MAX
*  число MAX_FREQUENCY должно быть не менее, чем в 8 раза меньше TOP_VALUE
*  число символов NO_OF_CHARS должно быть много меньше MAX_FREQUENCY
*/
// MAX_FREQUENCY=2^SHIFT определяет макс. размер пополняемых таблиц вероятностей
#define SHIFT                       1	// log2(NO_OF_CHARS)+1<=SHIFT<=15
// Частоты каждого символа уменьшаются в 2^SCALE_SHIFT раз после наполнения таблицы
// 1(минимальное. обновление таблицы)<=SCALE_SHIFT<=SHIFT(полное обновление)
#define SCALE_SHIFT                 1
#define BITS_IN_REGISTER            (32 - SHIFT)	// размер битового буфера кодового потока-value
#define TOP_VALUE                   (((unsigned long) 1 << BITS_IN_REGISTER) - 1) // 1111...1
// Константы для анализа значений high и low
#define ONE_QTR                     ((TOP_VALUE >> 2) + 1)                        // 0100...0
#define HALF                        (ONE_QTR << 1)                                // 1000...0
#define THREE_QTRS                  (HALF + ONE_QTR)                              // 1100...0
// Максимальный размер пополняемых таблиц вероятностей
#define MAX_FREQUENCY               ((unsigned) 1 << SHIFT)                       // 2^SHIFT

#define DO_NOTHING // define for empty action


int DefaultModelChoosingCallback(int i_symbol, unsigned int i_serialIndex, void* i_data)
{
  (void) i_symbol; // to avoid warning
  (void) i_serialIndex; // to avoid warning
  (void) i_data; // to avoid warning
  return 0;
}

class ArcoderImpl : public Arcoder
{
  static const int kInvalidSymbol = -1;
  static const int kDefaultElementSize = 4; // in bytes

  unsigned long m_upperBound;
  unsigned long m_lowerBound;
  unsigned long m_value; //used for decoding only

  std::vector<ArcoderModel> m_models;
  std::vector<ArcoderModel> m_escapes; // escapes can differ only by symbol count

  unsigned int m_currentModelIndex;

  unsigned int m_delayedBitCount;

  unsigned int m_elementSize;
  modelChoosingCallback m_modelChoosingCallback;
  void* m_modelChoosingCallbackUserData;

  IOutputBitStream* m_outputBitStream;
  IInputBitStream* m_inputBitStream;

  void PutBit(char i_bit);
  char GetBit();
  void DelayBit();

  void EncodeSymbol(int i_symbol, const ArcoderModel &i_model);
  int DecodeSymbol(ArcoderModel& i_model);

  ArcoderImpl();
  void Init();

public:

  ArcoderImpl(std::istream& i_modelsSource);
  ArcoderImpl(const std::vector<int>& i_symbolCount);

  // Inherited methods

  virtual bool LoadModel(std::istream&);
  virtual void SaveModel(std::ostream&);

  virtual void InitUniformModel(unsigned int);

  virtual void SetModelChoosingCallback(modelChoosingCallback, void*);
  virtual void SetElementSize(int);

  virtual void Compress(std::istream& i_input,
                        std::ostream& i_output);

  virtual void Decompress(std::istream& i_input,
                          std::ostream& i_output);
};

/**************************************************************************************************/
void ArcoderImpl::Init()
{
  m_elementSize = kDefaultElementSize;
  m_modelChoosingCallback = NULL;
}

/**************************************************************************************************/
ArcoderImpl::ArcoderImpl(std::istream& i_modelsSource)
{
  Init();
  LoadModel(i_modelsSource);
}

/**************************************************************************************************/
ArcoderImpl::ArcoderImpl(const std::vector<int>& i_symbolCount)
{
  Init();

  for (unsigned int i = 0; i < i_symbolCount.size(); i++)
  {
    m_models.push_back(ArcoderModel());

    m_escapes.push_back(ArcoderModel());
    m_escapes[i].InitUniformModel(i_symbolCount[i]);
  }
}

/**************************************************************************************************/
void ArcoderImpl::SetModelChoosingCallback(modelChoosingCallback i_callback, void* i_userData)
{
  m_modelChoosingCallback = i_callback;
  m_modelChoosingCallbackUserData = i_userData;
}

/**************************************************************************************************/
void ArcoderImpl::SetElementSize(int i_size)
{
  m_elementSize = i_size;
}

/**************************************************************************************************/
bool ArcoderImpl::LoadModel(std::istream& i_input)
{
  int modelCount = 0;
  i_input.read((char *) &modelCount, sizeof(modelCount));

  if (i_input.eof())
  {
    return false;
  }

  m_models.clear();
  m_models.assign(modelCount, ArcoderModel());

  m_escapes.clear();
  m_escapes.assign(modelCount, ArcoderModel());

  for (int i = 0; i < modelCount; i++)
  {
    int symbolCount = 0;
    i_input.read((char *) &symbolCount, sizeof(symbolCount));

    int maxSymbolValue = 0;
    i_input.read((char *) &maxSymbolValue, sizeof(maxSymbolValue));

    m_escapes[i].InitUniformModel(maxSymbolValue);

    if (i_input.eof() || symbolCount <= 0)
    {
      return false;
    }

    std::vector<int> symbols;
    symbols.assign(symbolCount, 0);

    // std::vector guarantees sequental data storage
    // in file, symbols table is followed by freqs table
    i_input.read((char *) &symbols[0], sizeof(symbols[0]) * symbolCount);

    if (i_input.eof())
    {
      return false;
    }

    std::vector<int> freqs;
    freqs.assign(symbolCount, 0);
    i_input.read((char *) &freqs[0], sizeof(freqs[0]) * symbolCount);

    m_models[i].Load(symbols, freqs);
  }

  if (i_input.peek() != EOF)
  {
    return false;
  }

  return true;
}

/**************************************************************************************************/
void ArcoderImpl::SaveModel(std::ostream& i_output)
{
  int modelCount = m_models.size();

  i_output.write((char *) &modelCount, sizeof(modelCount));

  for (int i = 0; i < modelCount; i++)
  {
    std::vector<int> symbols;
    std::vector<int> freqs;

    m_models[i].Save(symbols, freqs);

    assert(symbols.size() == freqs.size());

    int symbolCount = symbols.size();

    int maxSymbolValue = symbols[0];
    for (int j = 1; j < symbolCount; j++)
    {
      if (symbols[j] > maxSymbolValue)
      {
        maxSymbolValue = symbols[j];
      }
    }

    i_output.write((const char *) &symbolCount, sizeof(symbolCount));
    i_output.write((const char *) &maxSymbolValue, sizeof(maxSymbolValue));
    i_output.write((const char *) &symbols[0], sizeof(symbols[0]) * symbolCount);
    i_output.write((const char *) &freqs[0], sizeof(freqs[0]) * symbolCount);
  }
}

/**************************************************************************************************/
void ArcoderImpl::InitUniformModel(unsigned int i_symbolsCount)
{
  /*m_models.clear();
  m_models.push_back(ArcoderModel());
  m_models[0].InitUniformModel(i_symbolsCount);

  m_escapes.clear();
  m_escapes.push_back(ArcoderModel());
  m_escapes[0].InitUniformModel(i_symbolsCount);*/
  for (int i = 0; i < m_models.size(); i++)
  {
    m_models[i].InitUniformModel(i_symbolsCount);
    m_escapes[i].InitUniformModel(i_symbolsCount);
  }
}

/**************************************************************************************************/
void ArcoderImpl::EncodeSymbol(int i_symbol, const ArcoderModel& i_model)
{
  i_model.CalculateBounds(i_symbol,
                           m_lowerBound,
                           m_upperBound);
  
  // m_lowerBound MUST be less than m_upperBound
  assert(m_lowerBound < m_upperBound);

  // далее при необходимости - вывод бита или меры от зацикливания
  for (;;)
  {
    // m_lowerBound MUST be less than m_upperBound
    assert(m_lowerBound < m_upperBound);

    if (m_upperBound < HALF)
    { // Старшие биты low и high - нулевые (оба)
      PutBit(0);	//вывод совпадающего старшего бита
    }
    else if (m_lowerBound >= HALF)
    { // Старшие биты low и high - единичные
      PutBit(1);	//вывод старшего бита
      m_lowerBound -= HALF;					//сброс старшего бита в 0
      m_upperBound -= HALF;					//сброс старшего бита в 0
    }
    else if (m_lowerBound >= ONE_QTR && m_upperBound < THREE_QTRS )
    {
      /* возможно зацикливание, т.к.
          HALF<=high<THREE_QTR, i.e. high=10...
          ONE_QTR<=low<HALF,	i.e. low =01...
          выбрасываем второй по старшинству бит	*/
      m_upperBound -= ONE_QTR;		// high	=01...
      m_lowerBound -= ONE_QTR;			// low	=00...
      DelayBit();		//откладываем вывод (еще) одного бита
      // младший бит будет втянут далее
    }
    else
    {
      return;	// втягивать новый бит рано
    }

    // старший бит в low и high нулевой, втягиваем новый бит в младший разряд
    m_lowerBound <<= 1;		// втягиваем 0
    (m_upperBound <<= 1)++;	// втягиваем 1
  }
}

/**************************************************************************************************/
int ArcoderImpl::DecodeSymbol(ArcoderModel &i_model)
{
  int result = i_model.GetSymbol(m_value, m_lowerBound, m_upperBound);

  // подготовка к декодированию следующих символов
  for (;;)
  {
    if (m_upperBound < HALF) // Старшие биты low и high - нулевые
    {
      DO_NOTHING;
    }
    else if (m_lowerBound >= HALF)
    { // Старшие биты low и high - единичные, сбрасываем
      m_value -= HALF;
      m_lowerBound -= HALF;
      m_upperBound -= HALF;
    }
    else if (m_lowerBound >= ONE_QTR && m_upperBound < THREE_QTRS)
    {
      // Поступаем так же, как при кодировании
      m_value -= ONE_QTR;	// удалить отложенный бит из value !
      m_lowerBound -= ONE_QTR;
      m_upperBound -= ONE_QTR;
    }
    else
    {
      return result;	// втягивать новый бит рано
    }

    m_lowerBound <<= 1;							// втягиваем новый бит 0
    (m_upperBound <<= 1)++;						// втягиваем новый бит 1
    m_value = (m_value << 1) + GetBit();		// втягиваем новый бит информации
  }
}

/**************************************************************************************************/
void ArcoderImpl::PutBit(char i_bit)
{
  m_outputBitStream->PutBit(i_bit);

  while (m_delayedBitCount != 0)
  {
    m_delayedBitCount--;
    m_outputBitStream->PutBit(!i_bit);
  }
}

/**************************************************************************************************/
char ArcoderImpl::GetBit()
{
  return m_inputBitStream->GetBit();
}

/**************************************************************************************************/
void ArcoderImpl::DelayBit()
{
  m_delayedBitCount++;
}

/**************************************************************************************************/
void ArcoderImpl::Compress(std::istream& i_input,
                           std::ostream& i_output)
{
  if (!i_input.good())
  {
    return;
  }

  if (m_modelChoosingCallback == NULL)
  {
    m_modelChoosingCallback = DefaultModelChoosingCallback;
  }

  m_delayedBitCount = 0;
  m_upperBound = TOP_VALUE;
  m_lowerBound = 0;

  m_outputBitStream = CreateOutputBitStream(i_output);

  union
  {
    int intBuf;
    unsigned char byteBuf[sizeof(int)];
  } buf;

  buf.intBuf = 0;
  int prevSymbol = kInvalidSymbol;
  unsigned int symbolIndex = 0;

  while (!(i_input.read((char *) buf.byteBuf, m_elementSize).eof()))
  {
    if (prevSymbol != kInvalidSymbol)
    {
      m_currentModelIndex = m_modelChoosingCallback(prevSymbol, symbolIndex, m_modelChoosingCallbackUserData);
    }
    else
    {
      m_currentModelIndex = 0; // model with index 0 must exist in all cases
    }

    assert(m_currentModelIndex < m_models.size());

    if (m_models[m_currentModelIndex].SymbolExists(buf.intBuf))
    {
      EncodeSymbol(buf.intBuf,
                   m_models[m_currentModelIndex]);
    }
    else
    {
      EncodeSymbol(m_models[m_currentModelIndex].NewSymbolCode(),
                   m_models[m_currentModelIndex]);
      EncodeSymbol(buf.intBuf,
                   m_escapes[m_currentModelIndex]);
    }

    m_models[m_currentModelIndex].Update(buf.intBuf);
    prevSymbol = buf.intBuf;
    symbolIndex++;
  }

  m_currentModelIndex = 0;
  EncodeSymbol(m_models[m_currentModelIndex].EOFSymbolCode(),
               m_models[m_currentModelIndex]);

  DelayBit();
  PutBit(!(m_lowerBound < (TOP_VALUE >> 2) + 1)); // wtf?

  m_outputBitStream->Finish();

  delete m_outputBitStream;
  m_outputBitStream = NULL;
}

/**************************************************************************************************/
void ArcoderImpl::Decompress(std::istream& i_input,
                             std::ostream& i_output)
{
  if (!i_input.good())
  {
    return;
  }

  if (m_modelChoosingCallback == NULL)
  {
    m_modelChoosingCallback = DefaultModelChoosingCallback;
  }

  m_inputBitStream = CreateInputBitStream(i_input);

  m_delayedBitCount = 0;
  m_upperBound = TOP_VALUE;
  m_lowerBound = 0;

  m_value = 0;
  for (int i = 0; i < BITS_IN_REGISTER; i++)
  {
    m_value = (m_value << 1) + m_inputBitStream->GetBit();
  }

  union
  {
    int intBuf;
    unsigned char byteBuf[sizeof(int)];
  } buf;

  buf.intBuf = 0;
  int prevSymbol = kInvalidSymbol;
  unsigned int symbolIndex = 0;

  while (true)
  {
    if (prevSymbol != kInvalidSymbol)
    {
      m_currentModelIndex = m_modelChoosingCallback(prevSymbol, symbolIndex, m_modelChoosingCallbackUserData);
    }
    else
    {
      m_currentModelIndex = 0; // model with index 0 must exist in all cases
    }

    assert(m_currentModelIndex < m_models.size());

    buf.intBuf = DecodeSymbol(m_models[m_currentModelIndex]);

    if (buf.intBuf == m_models[m_currentModelIndex].NewSymbolCode())
    {
      buf.intBuf = DecodeSymbol(m_escapes[m_currentModelIndex]);
    }

    if (buf.intBuf > 255)
    {
      int i = 1;
      i++;
    }

    if (buf.intBuf == m_models[m_currentModelIndex].EOFSymbolCode())
    {
      return;
    }

    m_models[m_currentModelIndex].Update(buf.intBuf);
    prevSymbol = buf.intBuf;

    i_output.write((const char *) buf.byteBuf, m_elementSize);
    symbolIndex++;
  }
}


Arcoder* CreateArcoder(int i_symbolCount)
{
  std::vector<int> symbolCount;
  symbolCount.push_back(i_symbolCount);

  return CreateArcoder(symbolCount);
}

Arcoder* CreateArcoder(const std::vector<int>& i_symbolCount)
{
  return new ArcoderImpl(i_symbolCount);
}

Arcoder* CreateArcoder(std::istream& i_modelsSource)
{
  return new ArcoderImpl(i_modelsSource);
}
