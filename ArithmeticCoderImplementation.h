#ifndef ARITHMETICCODERIMPLEMENTATION_H
#define ARITHMETICCODERIMPLEMENTATION_H

#include "ArithmeticCoder.h"
#include "my_exception.h"

class AdaptiveModel;
class BitStream;

class ArithmeticCoderImplementation : public ArithmeticCoder
{
    unsigned int m_lowBorder;
    unsigned int m_highBorder;
    unsigned int m_currentValue; // used for decoding

    FILE* m_inputFile;
    FILE* m_outputFile;

    bool m_adaptive;

    void PrepareForOperation(const char* inputFilePath,
                             const char* outputFilePath);

    void FinishOperation();

    void EncodeSymbol(AdaptiveModel& model,
                      int symbol,
                      BitStream& bitstream);
    int DecodeSymbol(AdaptiveModel& model,
                     BitStream& bitstream);

public:

    ArithmeticCoderImplementation();
    virtual ~ArithmeticCoderImplementation() {}

    virtual void Encode(const char* inputFilePath,
                        const char* outputFilePath,
                        const char* modelFilePath,
                        int symbolsCount);

    virtual void Decode(const char* inputFilePath,
                        const char* outputFilePath,
                        const char* modelFilePath,
                        int symbolsCount);

    virtual void SetAdaptivity(bool adaptive);
};

#endif // ARITHMETICCODERIMPLEMENTATION_H
