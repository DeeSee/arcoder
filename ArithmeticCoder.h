#ifndef ARITHMETICCODER_H
#define ARITHMETICCODER_H

class ArithmeticCoder
{
public:

  virtual ~ArithmeticCoder() {}

  virtual void Encode(const char* inputFilePath,
                      const char* outputFilePath,
                      const char* modelFilePath,
                      int symbolsCount) = 0;

  virtual void Decode(const char* inputFilePath,
                      const char* outputFilePath,
                      const char* modelFilePath,
                      int symbolsCount) = 0;

  virtual void SetAdaptivity(bool adaptive) = 0; // Default is true
};

ArithmeticCoder* CreateCoder();

#endif // ARITHMETICCODER_H
