#ifndef BITSTREAM_H
#define BITSTREAM_H

#include "fstream"

class IOutputBitStream
{
public:

  virtual void PutBit(char i_bit) = 0; // puts one bit into the stream

  virtual void Finish() = 0; // finalizes stream

  virtual ~IOutputBitStream() {} // virtual destructor
};

class IInputBitStream
{
public:

  virtual char GetBit() = 0; // gets one bit from the stream

  virtual ~IInputBitStream() {} // virtual destructor
};

IOutputBitStream* CreateOutputBitStream(std::ostream& i_stream);
IInputBitStream* CreateInputBitStream(std::istream& i_stream);

#endif // BITSTREAM_H
