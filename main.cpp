// Arithmetic coder. Simplest & slowest (demo version)
#include <stdio.h>
#include <iostream>
//#include <fstream>
//#include <vector>
//#include <queue>
//#include <string>
#include "ArithmeticCoder.h"
#include "my_exception.h"
#include <stdlib.h>

using namespace std;

////для избежания переполнения:	MAX_FREQUENCY * (TOP_VALUE+1) < ULONG_MAX
////число MAX_FREQUENCY должно быть не менее, чем в 4 раза меньше TOP_VALUE
////число символов NO_OF_CHARS должно быть много меньше MAX_FREQUENCY
//#define BITS_IN_REGISTER	30
//#define TOP_VALUE			(((unsigned long)1<<BITS_IN_REGISTER)-1)	// 1111...1
//#define FIRST_QTR			((TOP_VALUE>>2) +1)							// 0100...0
//#define HALF				(2*FIRST_QTR)								// 1000...0
//#define THIRD_QTR			(3*FIRST_QTR)
#define NO_OF_CHARS			256

struct settings
{
  char code_mode;
  bool adaptive;
  string input_file_name;
  string output_file_name;
  string input_model_file_name;
  string output_model_file_name;
  string context;
  int context_size;
  int model_rank;
  int eof_symbol;
  int bits_in_register;
  int symbols_count;
  bool verbose;

  settings()
  {
    context_size = 40;
    code_mode = 0;
    adaptive = true;
    input_file_name = "";
    output_file_name = "";
    input_model_file_name = "";
    output_model_file_name = "";
    model_rank = 0;
    symbols_count = NO_OF_CHARS;
    eof_symbol = NO_OF_CHARS;
    bits_in_register = 30;
    verbose = false;
  }
};

void show_help(char* prog_name)
{
  cout << "Usage: " << prog_name << " [-e|d] " << endl
       << "\t\t[-nh]" << endl
       << "\t\t-i input_file_name " << endl
       << "\t\t-o output_file_name" << endl
       << "\t\t-m stored_model_filename" << endl
       << "\t\t-s model_filename" << endl << endl;
  cout << "\t -h \t\t print this help and exit" << endl;
  cout << "\t"
       << "-e or -d" << endl
       << "\t set mode 'encode' or 'decode'. Default 'encode'"
       << endl;
  cout << "\t"
       << "-n" << endl
       << "\t disable adaptive mode. "
       << "In this case coder will use static model." << endl;
  cout << "\t"
       << "-r rank" << endl
       << "\t specifies model rank. -r0 means that " << endl
       << "\t program will work with memoryless source, " << endl
       << "\t -r n means that model will build cumulative histogramm " <<endl
       << "for each of possible combinations of n symbols." << endl;
  cout << "\t"
       << "-i input_file_name" << endl
       << "\t specifies input binary file" << endl;
  cout << "\t"
       << "-o output_file_name" << endl
       << "\t specifies output binary file" << endl;
  cout << "\t"
       << "-m input_model_file_name" << endl
       << "\t specifies file containing model with specified rank." << endl
       << "\t File must contain zero-level models for each of 256^n" << endl
       << "\t combinations of symbols in lexicographic order." << endl
       << "\t Simplest element of file is binary unsigned int" << endl;
  cout << "\t"
       << "-s output_model_file_name" << endl
       << "\t saves built model to specified file. " << endl
       << "Note that if you wish to use it for encoding, " << endl
       << "you must run coder again and specify file with saved model."<<endl
       << "Consider that if you won't specify flag '-r' at second run,"<<endl
       << "model will be adapted again from specified initial state." <<endl;
  cout << endl;
  cout << "WARNING: consider that model with rank 3 and higher " << endl
       << "uses huge amount of RAM, so don't try it, if you haven't " << endl
       << "enough memory (about 16Gb for rank 3)." << endl;
  cout << "WARNING: be sure to specify same options to decode " << endl
       << "encoded file, or program can crash, go to inifnite loop " << endl
       << "and decoded file won't be same as initial." << endl;
}

int parse_options(settings& flags, int argc, char** argv)
{
  if (argc == 1)
  {
    show_help(argv[0]);
    return 1;
  }

  char c;
  while ((c = getopt(argc, argv, "edhi:o:m:s:r:nvc:q:a:")) != -1)
    switch (c)
    {
    case 'h':
      show_help(argv[0]);
    return 1;
    break;
    case 'i':
      flags.input_file_name = optarg;
    break;
    case 'c':
      flags.context = optarg;
    break;
    case 'q':
      flags.context_size = atoi(optarg);
    break;
    case 'a':
      flags.symbols_count = atoi(optarg);
    break;
    case 'o':
      flags.output_file_name = optarg;
    break;
    case 'm':
      flags.input_model_file_name = optarg;
    break;
    case 's':
      flags.output_model_file_name = optarg;
    break;
    case 'n':
      flags.adaptive = false;
    break;
    case 'v':
      flags.verbose = true;
    break;
    case 'r':
      flags.model_rank = atoi(optarg);
      if (flags.model_rank < 0)
        throw my_exception(
            "parse_options: invalid model rank");
    break;
    case 'e':
      if (flags.code_mode != 0)
        throw my_exception(
            "'-e' and '-d' flags cannot be specified simultaniously");
      flags.code_mode = 'e';
    break;
    case 'd':
      if (flags.code_mode != 0)
        throw my_exception(
            "'-e' and '-d' flags cannot be specified simultaniously");
      flags.code_mode = 'd';
    break;
    case '?':
    return 1;
    default:
    return 1;
    }
  if (flags.input_file_name == "")
    throw my_exception("parse_options: input file not specified");
  if (flags.output_file_name == "")
    throw my_exception("parse_options: output file not specified");
  if (flags.code_mode == 0)
    flags.code_mode = 'e';
  return 0;
}

//void encode_symbol(adaptive_model& model,
//                   const queue<int>& history,
//                   bit_stream& bitstream,
//                   const vector<int>& nums)
//{
//    // пересчет границ интервала
//    unsigned long range;
//    unsigned int low = bitstream.get_low();
//    unsigned int high = bitstream.get_high();
//    range = high - low + 1;
//    bitstream.set_high(low + model.upper_bound_context(history, range, nums));
//    bitstream.set_low(low + model.lower_bound_context(history, range, nums));
//    low = bitstream.get_low();
//    high = bitstream.get_high();
//    // далее при необходимости — вывод бита или меры от зацикливания
//    for (;;)
//    {       // Замечание: всегда low < high
//        if (high < HALF) // Старшие биты low и high — нулевые (оба)
//                bitstream.output_bit_plus_follow(0); //вывод совпадающего старшего бита
//        else if (low >= HALF) // старшие биты low и high - единичные
//        {
//                bitstream.output_bit_plus_follow(1);      // вывод старшего бита
//                low  -= HALF;                           // сброс старшего бита в 0
//                high -= HALF;                           // сброс старшего бита в 0
//        }
//        else if (low >= FIRST_QTR && high < THIRD_QTR)
//        {/* возможно зацикливание, т.к.
//                HALF <= high < THIRD_QTR,       i.e. high=10...
//                FIRST_QTR <= low < HALF,        i.e. low =01...
//                выбрасываем второй по старшинству бит   */
//                high -= FIRST_QTR;              // high =01...
//                low -= FIRST_QTR;               // low  =00...
//                bitstream.follow_bit();               //откладываем вывод (еще) одного бита
//                // младший бит будет втянут далее
//        }
//        else break;             // втягивать новый бит рано
//    // старший бит в low и high нулевой, втягиваем новый бит в младший разряд
//        low <<= 1;      // втягиваем 0
//        high <<= 1;
//        high++;         // втягиваем 1
//        bitstream.set_high(high);
//        bitstream.set_low(low);
//    }
//    return;
//}

//int decode_symbol(adaptive_model& model,
//                  queue<int> history,
//                  bit_stream& bitstream,
//                  const vector<int>& nums)
//{
//    unsigned long range, cum;
//    unsigned int low = bitstream.get_low();
//    unsigned int high = bitstream.get_high();
//    unsigned int value = bitstream.get_value();
//    range = high - low + 1;
//    queue<int> tmp = history;
//    tmp.pop();
//    tmp.push(0);
//    unsigned long cnt = model.symbols_count_context(tmp, nums);
//    // число cum - это число value, пересчитанное из интервала
//    // low..high в интервал 0..CUM_FREQUENCY[NO_OF_SYMBOLS]
//    cum = ((value - low + 1) * cnt - 1) / range;
//    // поиск интервала, соответствующего числу cum
//    int symbol = model.get_symbol_context(tmp, cum, nums);
//    // пересчет границ
//    history.pop();
//    history.push(symbol);
//    bitstream.set_high(low + model.upper_bound_context(history, range, nums));
//    bitstream.set_low(low + model.lower_bound_context(history, range, nums));
//    low = bitstream.get_low();
//    high = bitstream.get_high();
//    for (;;)
//    {	// подготовка к декодированию следующих символов
//        if (high < HALF) {/* cтаршие биты low и high - нулевые */}
//        else if (low >= HALF)
//        {	// cтаршие биты low и high - единичные, сбрасываем
//            value -= HALF;
//            low -= HALF;
//            high -= HALF;
//        }
//        else if (low >= FIRST_QTR && high < THIRD_QTR)
//        {	// поступаем так же, как при кодировании
//            value -= FIRST_QTR;
//            low -= FIRST_QTR;
//            high -= FIRST_QTR;
//        }
//        else break;	// втягивать новый бит рано
//        low <<= 1; // втягиваем новый бит 0
//        high <<= 1;
//        high++;	// втягиваем новый бит 1
//        value = (value<<1) + bitstream.input_bit(); // втягиваем новый бит информации
//        bitstream.set_high(high);
//        bitstream.set_low(low);
//        bitstream.set_value(value);
//    }
//    return symbol;
//}

//void code(const settings& flags)
//{

//    FILE* input = fopen(flags.input_file_name.c_str(), "r+b");
//    FILE* output = fopen(flags.output_file_name.c_str(), "w+b");
//    if (input == 0)
//        throw my_exception("couldn't open input file");
//    if (output == 0)
//        throw my_exception("couldn't open output file");

//    FILE* contexts = fopen(flags.context.c_str(), "r+b");

//    vector<vector<int> > context;
//    context.assign(flags.symbols_count, vector<int>());
//    for (int i = 0; i < context.size(); i++)
//    {
//        for (int j = 0; j < flags.context_size; j++)
//        {
//            unsigned char buf = getc(contexts);
//            context[i].push_back(buf);
////            cout << context[i][j] << endl;
//        }
////        cout << context[i][0] << endl;
//        context[i].push_back(flags.symbols_count - 1);
//    }
//    vector<int> full_book;
//    for (int i = 0; i < context.size(); i++)
//        full_book.push_back(i);

//    queue<int> history;
//    int symbol;
//    int prev_symbol = -1;

//    adaptive_model model(flags.symbols_count, flags.model_rank);
//    bit_stream bitstream(flags.bits_in_register, input, output);

//    if (flags.input_model_file_name != "")
//    {
//        FILE* in = fopen(flags.input_model_file_name.c_str(), "r");
//        if (in == NULL)
//            throw my_exception("couldn't open file to load model");
//        model.load_from_file(in);
//        fclose(in);
//    }

//    if (flags.verbose)
//    {
//        cout << "Initial entropies per symbol of model are:" << endl;
//        for (int i = 0; i <= flags.model_rank + 1; i++)
//            cout << "\tDegree " << i << ": entropy is "
//                 << model.entropy_per_symbol(i) << endl;
//    }

////    ofstream fout("test1");
//    int counter = 0;
//    switch (flags.code_mode)
//    {
//    case 'e':
//        for (int i = 0; i < flags.model_rank; i++)
//            history.push(0);
//        bitstream.start_encoding();
//        while ((symbol = getc(input)) != EOF)
//        {
////            if (counter == 645)
////                cout << "ololo" << endl;
//            history.push(symbol);
//            if (prev_symbol != -1)
//                encode_symbol(model, history, bitstream, context[prev_symbol]);
//            else
//                encode_symbol(model, history, bitstream, full_book);
//            if (flags.adaptive)
//                model.update(history);
//            history.pop();
//            counter++;
//            prev_symbol = (counter % 128 == 0) ? -1 : symbol;
////		if (counter % 100 == 0)
////{
////            double tmp = model.entropy_per_symbol(1);
////            fout.write((char*) &tmp, 8);}
////		counter++;
//        }
////        fout.close();
//        history.push(flags.eof_symbol);
//        if (prev_symbol != -1)
//            encode_symbol(model, history, bitstream, context[prev_symbol]);
//        else
//            encode_symbol(model, history, bitstream, full_book);
//        bitstream.done_encoding();
//        break;
//    case 'd':
//        for (int i = 0; i < flags.model_rank + 1; i++)
//            history.push(0);
//        bitstream.start_decoding();
//        while (true)
//        {
//            if (counter == 645)
//                cout << "ololo" << endl;
//            if (prev_symbol != -1)
//                symbol = decode_symbol(model, history, bitstream, context[prev_symbol]);
//            else
//                symbol = decode_symbol(model, history, bitstream, full_book);
//            if (symbol == flags.eof_symbol)
//                break;
//            counter++;
//            prev_symbol = (counter % 128 == 0) ? -1 : symbol;
//            history.pop();
//            history.push(symbol);
//            if (flags.adaptive)
//                model.update(history);
//            putc(symbol, output);
//        }
//        break;
//    default:
//        throw my_exception("code: unknown code mode");
//    }

//    if (flags.output_model_file_name != "")
//    {
//        FILE* model_file = fopen(flags.output_model_file_name.c_str(), "w+b");
//        if (model_file == NULL)
//            throw my_exception("couldn't open file to save model");
//        model.save_to_file(model_file);
//        fclose(model_file);
//    }

//    if (flags.verbose)
//    {
//        cout << endl;
//        cout << "Resulting entropies per symbol of model are:" << endl;
//        for (int i = 0; i <= flags.model_rank + 1; i++)
//            cout << "\tDegree " << i << ": entropy is "
//                 << model.entropy_per_symbol(i) << endl;
//    }

////    cout << model.entropy_per_symbol(3) << endl;

//    fclose(input);
//    fclose(output);

//}


int main(int argc, char **argv)
{
  settings flags;

  try
  {
    if (parse_options(flags, argc, argv))
    {
      return 1;
    }

    ArithmeticCoder* coder = CreateCoder();

    coder->SetAdaptivity(flags.adaptive);

    const char* modelFilename = flags.input_model_file_name.c_str();

    if (modelFilename[0] == '\0')
    {
      modelFilename = NULL;
    }

    if (flags.code_mode == 'e')
    {

      coder->Encode(flags.input_file_name.c_str(),
                    flags.output_file_name.c_str(),
                    modelFilename,
                    flags.symbols_count);
    }
    else if (flags.code_mode == 'd')
    {
      coder->Decode(flags.input_file_name.c_str(),
                    flags.output_file_name.c_str(),
                    modelFilename,
                    flags.symbols_count);
    }
    else
    {
      throw my_exception("Invalid command");
    }

    delete coder;
  }
  catch (exception& e)
  {
    cerr << "Catched exception with following message:" << endl
         << "\t" << e.what() << endl;
  }

  return 0;
}
