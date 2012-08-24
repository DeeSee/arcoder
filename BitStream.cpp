#include "BitStream.h"

#include "assert.h"


static const int kBitsInByte = 8;


class OutputBitStream : public IOutputBitStream // implements basic logic of bits accumulation
{
  int m_unflushedBitsCount;

protected:

  OutputBitStream() : m_unflushedBitsCount(0),
                      m_bitsBuffer(0)
  {}

  char m_bitsBuffer;

  virtual void FlushBuffer() = 0; // should flush byte from m_bitsBuffer to data container

public:

  // inherited methods

  virtual void Finish();

  virtual void PutBit(char bit);

};

/**************************************************************************************************/
void OutputBitStream::PutBit(char bit)
{
  assert(bit == 0 || bit == 1);

  m_unflushedBitsCount++;
  m_bitsBuffer = (m_bitsBuffer << 1) | bit;

  if (m_unflushedBitsCount == kBitsInByte)
  {
    FlushBuffer();
    m_unflushedBitsCount = 0;
    m_bitsBuffer = 0;
  }
}

/**************************************************************************************************/
void OutputBitStream::Finish()
{
  if (m_unflushedBitsCount != 0)
  {
    m_bitsBuffer <<= (kBitsInByte - m_unflushedBitsCount);
    FlushBuffer();
  }
}

class StdOutputBitStream : public OutputBitStream
{
  StdOutputBitStream(); // disable default constructor

  std::ostream& m_stream;

protected:

  virtual void FlushBuffer();

public:

  // constructor

  StdOutputBitStream(std::ostream& i_stream) : m_stream(i_stream) {}
};

/**************************************************************************************************/
void StdOutputBitStream::FlushBuffer()
{
  m_stream.write(&m_bitsBuffer, sizeof(m_bitsBuffer));
}

/**************************************************************************************************/
IOutputBitStream* CreateOutputBitStream(std::ostream& i_stream)
{
  return new StdOutputBitStream(i_stream);
}


class InputBitStream : public IInputBitStream // implements basic logic of bits accumulation
{
  int m_storedBitsCount;

protected:

  InputBitStream() : m_storedBitsCount(0),
                     m_bitsBuffer(0)
  {}

  char m_bitsBuffer;

  virtual void LoadBuffer() = 0; // should load byte from data container to m_bitsBuffer

public:

  virtual char GetBit();

};

/**************************************************************************************************/
char InputBitStream::GetBit()
{
  if (m_storedBitsCount == 0)
  {
    LoadBuffer();
    m_storedBitsCount = kBitsInByte;
  }

  char result = (m_bitsBuffer & 0b10000000) >> (kBitsInByte - 1);

  m_bitsBuffer <<= 1;
  m_storedBitsCount--;

  return result;
}

class StdInputBitStream : public InputBitStream
{
  StdInputBitStream(); // disable default constructor

  std::istream& m_stream;

protected:

  virtual void LoadBuffer();

public:

  // Constructor

  StdInputBitStream(std::istream& i_stream) : m_stream(i_stream) {}
};

/**************************************************************************************************/
void StdInputBitStream::LoadBuffer()
{
  m_stream.read(&m_bitsBuffer, sizeof(m_bitsBuffer));
}

/**************************************************************************************************/
IInputBitStream* CreateInputBitStream(std::istream& i_stream)
{
  return new StdInputBitStream(i_stream);
}
