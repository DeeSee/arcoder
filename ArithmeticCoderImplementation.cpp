#include "ArithmeticCoderImplementation.h"

#include "AdaptiveModel.h"
#include "BitStream.h"


//для избежания переполнения:	MAX_FREQUENCY * (TOP_VALUE+1) < ULONG_MAX
//число MAX_FREQUENCY должно быть не менее, чем в 4 раза меньше TOP_VALUE
//число символов NO_OF_CHARS должно быть много меньше MAX_FREQUENCY

static const int kBitsInRegister = 30;
static const int kMaximumBorderValue =
    (((unsigned long) 1 << kBitsInRegister) - 1);

static const unsigned int kFirstIntervalQuarter = (kMaximumBorderValue >> 2) + 1;
static const unsigned int kIntervalMiddle =       2 * kFirstIntervalQuarter;
static const unsigned int kThirdIntervalQuarter = 3 * kFirstIntervalQuarter;

static const int kEOFSymbol = 256; // number of chars

/*****************************************************************************/
ArithmeticCoderImplementation::ArithmeticCoderImplementation()
{
  m_adaptive = true;
}

/*****************************************************************************/
void ArithmeticCoderImplementation::EncodeSymbol(AdaptiveModel& model,
                                                 int symbol,
                                                 BitStream& bitstream)
{
  // пересчет границ интервала
  unsigned long range = m_highBorder - m_lowBorder + 1;

  m_highBorder = m_lowBorder + model.UpperBound(symbol, range);
  m_lowBorder = m_lowBorder + model.LowerBound(symbol, range);

  // далее при необходимости — вывод бита или меры от зацикливания
  while (true)
  {       // Замечание: всегда low < high
    if (m_highBorder < kIntervalMiddle) // Старшие биты low и high — нулевые (оба)
    {
      bitstream.OutputBitPlusFollow(0); //вывод совпадающего старшего бита
    }
    else if (m_lowBorder >= kIntervalMiddle) // старшие биты low и high - единичные
    {
      bitstream.OutputBitPlusFollow(1);      // вывод старшего бита
      m_lowBorder  -= kIntervalMiddle;                           // сброс старшего бита в 0
      m_highBorder -= kIntervalMiddle;                           // сброс старшего бита в 0
    }
    else if (m_lowBorder >= kFirstIntervalQuarter &&
             m_highBorder < kThirdIntervalQuarter)
    {
      /* возможно зацикливание, т.к.
                HALF <= high < THIRD_QTR,       i.e. high=10...
                FIRST_QTR <= low < HALF,        i.e. low =01...
                выбрасываем второй по старшинству бит   */
      m_highBorder -= kFirstIntervalQuarter;              // high =01...
      m_lowBorder -= kFirstIntervalQuarter;               // low  =00...
      bitstream.FollowBit();               //откладываем вывод (еще) одного бита
      // младший бит будет втянут далее
    }
    else
    {
      break;             // втягивать новый бит рано
    }

    // старший бит в low и high нулевой, втягиваем новый бит в младший разряд
    m_lowBorder <<= 1;      // втягиваем 0
    m_highBorder <<= 1;
    m_highBorder++;         // втягиваем 1
  }
  return;
}

/*****************************************************************************/
int ArithmeticCoderImplementation::DecodeSymbol(AdaptiveModel& model,
                                                BitStream& bitstream)
{
  unsigned long range = m_highBorder - m_lowBorder + 1;

  unsigned long cnt = model.SymbolsCount();

  // число cum - это число value, пересчитанное из интервала
  // low..high в интервал 0..CUM_FREQUENCY[NO_OF_SYMBOLS]
  unsigned long cumulativeFrequency =
      ((m_currentValue - m_lowBorder + 1) * cnt - 1) / range;

  // поиск интервала, соответствующего числу cum
  int symbol = model.GetSymbol(cumulativeFrequency);

  // пересчет границ
  m_highBorder = m_lowBorder + model.UpperBound(symbol, range);
  m_lowBorder = m_lowBorder + model.LowerBound(symbol, range);

  while (true)
  {	// подготовка к декодированию следующих символов
    if (m_highBorder < kIntervalMiddle)
    {
      /* cтаршие биты low и high - нулевые */
    }
    else if (m_lowBorder >= kIntervalMiddle)
    {
      // cтаршие биты low и high - единичные, сбрасываем
      m_currentValue -= kIntervalMiddle;
      m_lowBorder -= kIntervalMiddle;
      m_highBorder -= kIntervalMiddle;
    }
    else if (m_lowBorder >= kFirstIntervalQuarter &&
             m_highBorder < kThirdIntervalQuarter)
    {
      // поступаем так же, как при кодировании
      m_currentValue -= kFirstIntervalQuarter;
      m_lowBorder -= kFirstIntervalQuarter;
      m_highBorder -= kFirstIntervalQuarter;
    }
    else
    {
      break;	// втягивать новый бит рано
    }

    m_lowBorder <<= 1; // втягиваем новый бит 0
    m_highBorder <<= 1;
    m_highBorder++;	// втягиваем новый бит 1
    m_currentValue = (m_currentValue << 1) + bitstream.InputBit(); // втягиваем новый бит информации
  }

  return symbol;
}

/*****************************************************************************/
void ArithmeticCoderImplementation::PrepareForOperation(const char* inputFilePath,
                                                        const char* outputFilePath)
{
  m_inputFile = fopen(inputFilePath, "r+b");
  m_outputFile = fopen(outputFilePath, "w+b");

  if (!m_outputFile || !m_inputFile)
  {
    throw my_exception("couldn't open input file");
  }

  m_highBorder = kMaximumBorderValue;
  m_lowBorder = 0;
}

/*****************************************************************************/
void ArithmeticCoderImplementation::FinishOperation()
{
  fclose(m_inputFile);
  fclose(m_outputFile);
}

/*****************************************************************************/
void ArithmeticCoderImplementation::Encode(const char* inputFilePath,
                                           const char* outputFilePath,
                                           const char* modelFilePath,
                                           int symbolsCount)
{
  PrepareForOperation(inputFilePath,
                      outputFilePath);

  AdaptiveModel model(symbolsCount + 1);

  if (modelFilePath != NULL)
  {
    FILE* in = fopen(modelFilePath, "r");

    if (in == NULL)
    {
      throw my_exception("couldn't open file to load model");
    }

    model.LoadFromFile(in);
    fclose(in);
  }

  BitStream bitstream(kBitsInRegister,
                      m_inputFile,
                      m_outputFile);

  bitstream.StartEncoding();

  int currentSymbol;

  while ((currentSymbol = getc(m_inputFile)) != EOF)
  {
    EncodeSymbol(model, currentSymbol, bitstream);

    if (m_adaptive)
    {
      model.Update(currentSymbol);
    }
  }

  EncodeSymbol(model, symbolsCount, bitstream);

  bitstream.DoneEncoding(m_lowBorder);
}

/*****************************************************************************/
void ArithmeticCoderImplementation::Decode(const char *inputFilePath,
                                           const char *outputFilePath,
                                           const char *modelFilePath,
                                           int symbolsCount)
{
  PrepareForOperation(inputFilePath,
                      outputFilePath);

  AdaptiveModel model(symbolsCount + 1);

  if (modelFilePath != NULL)
  {
    FILE* in = fopen(modelFilePath, "r");

    if (in == NULL)
    {
      throw my_exception("couldn't open file to load model");
    }

    model.LoadFromFile(in);
    fclose(in);
  }

  BitStream bitstream(kBitsInRegister,
                      m_inputFile,
                      m_outputFile);


  m_currentValue = bitstream.StartDecoding();

  while (true)
  {
    int symbol = DecodeSymbol(model, bitstream);

    if (symbol == symbolsCount)
    {
      break;
    }

    if (m_adaptive)
    {
      model.Update(symbol);
    }

    putc(symbol, m_outputFile);
  }


}

void ArithmeticCoderImplementation::SetAdaptivity(bool adaptive)
{
  m_adaptive = adaptive;
}
