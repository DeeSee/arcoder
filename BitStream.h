#ifndef CODING_STATE_H
#define CODING_STATE_H

#include "my_exception.h"
#include <stdio.h>

class BitStream
{
    int m_bitsToGo;
    int m_bitsToFollow;
    int m_garbageBitsCount;

    FILE* m_input;
    FILE* m_output;

    unsigned long m_top;
    int m_bitsInRegister;
    unsigned char m_buffer;

    BitStream();

public:

    BitStream(const int bitsInRegister,
               FILE* in, FILE* out);

    void StartEncoding();
    unsigned int StartDecoding();

    char InputBit();

    void OutputBitPlusFollow(int bit);
    void OutputBit(int bit);

    void FollowBit();

    void DoneEncoding(unsigned int lowBorder);
};

#endif // CODING_STATE_H
