#include "BitStream.h"

static const int kBitsInByte = 8;

BitStream::BitStream(const int bitsInRegister,
                       FILE* in,
                       FILE* out)
{
    m_top = (1 << bitsInRegister) - 1;

    m_bitsInRegister = bitsInRegister;
    m_input = in;
    m_output = out;
    m_bitsToFollow = 0;
}

void BitStream::StartEncoding()
{
    // свободно бит в битовом буфере вывода
    m_bitsToGo = 8;
    // число бит, вывод которых отложен
    m_bitsToFollow = 0;
}


unsigned int BitStream::StartDecoding()
{
    // свободно бит в битовом буфере ввода
    m_bitsToGo = 0;
    // контроль числа "мусорных" бит в конце сжатого файла
    m_garbageBitsCount = 0;

    unsigned int result = 0;

    for (int i = 0; i < m_bitsInRegister; i++)
    {
            result = (result << 1) + InputBit();
    }

    return result;
}

char BitStream::InputBit() // ввод 1 бита из сжатого файла
{
    if (m_bitsToGo == 0)
    {
        m_buffer = getc(m_input);	// заполняем буфер битового ввода
        if (m_buffer == EOF)	// входной поток сжатых данных исчерпан
        {
            // Причина попытки дальнейшего чтения: следующим
            // декодируемым символом должен быть EOF_SYMBOL,
            // но декодер об этом пока не знает и может готовиться
            // к дальнейшему декодированию, втягивая новые биты
            // (см. цикл for(;;) в процедуре decode_symbol). Эти
            // биты — "мусор", реально не несут никакой
            // информации и их можно выдать любыми
            m_garbageBitsCount++;
            // больше максимально возможного числа мусорных битов
            if (m_garbageBitsCount > m_bitsInRegister - 2)
            {
                throw my_exception("Error in compressed file");
            }

            m_bitsToGo = 1;
        }
        else m_bitsToGo = kBitsInByte;
    }

    char result = m_buffer & 1;

    m_buffer >>= 1;

    m_bitsToGo--;

    return result;
}

void BitStream::OutputBitPlusFollow(int bit) // вывод одного очередного бита и тех, которые были отложены
{
    OutputBit(bit);

    while (m_bitsToFollow > 0)
    {
        OutputBit(!bit);
        m_bitsToFollow--;
    }
}

void BitStream::OutputBit(int bit) // вывод одного бита в сжатый файл
{
    m_buffer = (m_buffer >> 1) + (bit << 7); // в битовый буфер (один байт)
    m_bitsToGo--;

    if (m_bitsToGo == 0) // битовый буфер заполнен, сброс буфера
    {
        putc(m_buffer, m_output);
        m_bitsToGo = kBitsInByte;
    }
}
void BitStream::FollowBit()
{
    m_bitsToFollow++;
}

void BitStream::DoneEncoding(unsigned int lowBorder)
{
    m_bitsToFollow++;

    if (lowBorder < (m_top >> 2) + 1)
    {
        OutputBitPlusFollow(0);
    }
    else
    {
        OutputBitPlusFollow(1);
    }

    putc(m_buffer >> m_bitsToGo, m_output); // записать незаполненный буфер
}
