//#include <iostream>
//#include <exception>
//#include <string>
//#include <fstream>
//#include "Include/ArCodec.h"
//#include "Include/ArBitStream.h"
//#include <stdlib.h>
//#include <exception>
//#include <string>

//using namespace std;

//////для избежания переполнения: MAX_FREQUENCY * (TOP_VALUE+1) < ULONG_MAX
//////число MAX_FREQUENCY должно быть не менее, чем в 4 раза меньше TOP_VALUE
//////число символов NO_OF_CHARS должно быть много меньше MAX_FREQUENCY
////#define BITS_IN_REGISTER      30
////#define TOP_VALUE                     (((unsigned long)1<<BITS_IN_REGISTER)-1)        // 1111...1
////#define FIRST_QTR                     ((TOP_VALUE>>2) +1)                                                     // 0100...0
////#define HALF                          (2*FIRST_QTR)                                                           // 1000...0
////#define THIRD_QTR                     (3*FIRST_QTR)
//#define NO_OF_CHARS                     256


//class my_exception: public std::exception
//{
//    std::string msg;
//    my_exception();

//public:

//    my_exception(const char* message): msg(message)
//    {
//    }

//    const char* what() const throw()
//    {
//        return msg.c_str();
//    }

//    ~my_exception() throw()
//    {
//    }
//};

//struct settings
//{
//  char code_mode;
//  bool adaptive;
//  string input_file_name;
//  string output_file_name;
//  string input_model_file_name;
//  string output_model_file_name;
//  string context;
//  int context_size;
//  int model_rank;
//  int eof_symbol;
//  int bits_in_register;
//  int symbols_count;
//  bool verbose;

//  settings()
//  {
//    context_size = 40;
//    code_mode = 0;
//    adaptive = true;
//    input_file_name = "";
//    output_file_name = "";
//    input_model_file_name = "";
//    output_model_file_name = "";
//    model_rank = 0;
//    symbols_count = NO_OF_CHARS;
//    eof_symbol = NO_OF_CHARS;
//    bits_in_register = 30;
//    verbose = false;
//  }
//};

//void show_help(char* prog_name)
//{
//  cout << "Usage: " << prog_name << " [-e|d] " << endl
//       << "\t\t[-nh]" << endl
//       << "\t\t-i input_file_name " << endl
//       << "\t\t-o output_file_name" << endl
//       << "\t\t-m stored_model_filename" << endl
//       << "\t\t-s model_filename" << endl << endl;
//  cout << "\t -h \t\t print this help and exit" << endl;
//  cout << "\t"
//       << "-e or -d" << endl
//       << "\t set mode 'encode' or 'decode'. Default 'encode'"
//       << endl;
//  cout << "\t"
//       << "-n" << endl
//       << "\t disable adaptive mode. "
//       << "In this case coder will use static model." << endl;
//  cout << "\t"
//       << "-r rank" << endl
//       << "\t specifies model rank. -r0 means that " << endl
//       << "\t program will work with memoryless source, " << endl
//       << "\t -r n means that model will build cumulative histogramm " <<endl
//       << "for each of possible combinations of n symbols." << endl;
//  cout << "\t"
//       << "-i input_file_name" << endl
//       << "\t specifies input binary file" << endl;
//  cout << "\t"
//       << "-o output_file_name" << endl
//       << "\t specifies output binary file" << endl;
//  cout << "\t"
//       << "-m input_model_file_name" << endl
//       << "\t specifies file containing model with specified rank." << endl
//       << "\t File must contain zero-level models for each of 256^n" << endl
//       << "\t combinations of symbols in lexicographic order." << endl
//       << "\t Simplest element of file is binary unsigned int" << endl;
//  cout << "\t"
//       << "-s output_model_file_name" << endl
//       << "\t saves built model to specified file. " << endl
//       << "Note that if you wish to use it for encoding, " << endl
//       << "you must run coder again and specify file with saved model."<<endl
//       << "Consider that if you won't specify flag '-r' at second run,"<<endl
//       << "model will be adapted again from specified initial state." <<endl;
//  cout << endl;
//  cout << "WARNING: consider that model with rank 3 and higher " << endl
//       << "uses huge amount of RAM, so don't try it, if you haven't " << endl
//       << "enough memory (about 16Gb for rank 3)." << endl;
//  cout << "WARNING: be sure to specify same options to decode " << endl
//       << "encoded file, or program can crash, go to inifnite loop " << endl
//       << "and decoded file won't be same as initial." << endl;
//}

//int parse_options(settings& flags, int argc, char** argv)
//{
//  if (argc == 1)
//  {
//    show_help(argv[0]);
//    return 1;
//  }

//  char c;
//  while ((c = getopt(argc, argv, "edhi:o:m:s:r:nvc:q:a:")) != -1)
//    switch (c)
//    {
//    case 'h':
//      show_help(argv[0]);
//    return 1;
//    break;
//    case 'i':
//      flags.input_file_name = optarg;
//    break;
//    case 'c':
//      flags.context = optarg;
//    break;
//    case 'q':
//      flags.context_size = atoi(optarg);
//    break;
//    case 'a':
//      flags.symbols_count = atoi(optarg);
//    break;
//    case 'o':
//      flags.output_file_name = optarg;
//    break;
//    case 'm':
//      flags.input_model_file_name = optarg;
//    break;
//    case 's':
//      flags.output_model_file_name = optarg;
//    break;
//    case 'n':
//      flags.adaptive = false;
//    break;
//    case 'v':
//      flags.verbose = true;
//    break;
//    case 'r':
//      flags.model_rank = atoi(optarg);
//      if (flags.model_rank < 0)
//        throw my_exception(
//            "parse_options: invalid model rank");
//    break;
//    case 'e':
//      if (flags.code_mode != 0)
//        throw my_exception(
//            "'-e' and '-d' flags cannot be specified simultaniously");
//      flags.code_mode = 'e';
//    break;
//    case 'd':
//      if (flags.code_mode != 0)
//        throw my_exception(
//            "'-e' and '-d' flags cannot be specified simultaniously");
//      flags.code_mode = 'd';
//    break;
//    case '?':
//    return 1;
//    default:
//    return 1;
//    }
//  if (flags.input_file_name == "")
//    throw my_exception("parse_options: input file not specified");
//  if (flags.output_file_name == "")
//    throw my_exception("parse_options: output file not specified");
//  if (flags.code_mode == 0)
//    flags.code_mode = 'e';
//  return 0;
//}


//int main(int argc, char** argv)
//{
//  settings flags;

//  if (parse_options(flags, argc, argv))
//  {
//    return 1;
//  }

//  const char* modelFilename = flags.input_model_file_name.c_str();

//  if (modelFilename[0] == '\0')
//  {
//    modelFilename = NULL;
//  }

//  if (flags.code_mode == 'e')
//  {
//    ofstream out(flags.output_file_name.c_str());

//    BITOutFileStream outputStream(out);
//    ArCoder<BITOutFileStream> arCoder(1, &flags.symbols_count, outputStream);

//    outputStream.StartPacking();
//    arCoder.StartPacking();

//    ifstream in(flags.input_file_name.c_str());

//    unsigned char buf;
//    while (in.read((char *) &buf, sizeof(buf)))
//    {
//      arCoder << buf;
//    }
//    in.close();
//    arCoder.PutEOF();
//    arCoder.EndPacking();
//    outputStream.EndPacking();
//    out.close();
//  }
//  else if (flags.code_mode == 'd')
//  {
//    ifstream in(flags.input_file_name.c_str());

//    BITInFileStream inputStream(in);
//    ArDecoder<BITInFileStream> arDecoder(1, &flags.symbols_count, inputStream);

//    inputStream.StartUnpacking();
//    arDecoder.StartUnpacking();

//    ofstream out(flags.output_file_name.c_str());

//    int buf;
//    arDecoder >> buf;
//    while (buf != arDecoder.EOF_SYMBOL())
//    {
//      out.write((char *) &buf, sizeof(char));
//      arDecoder >> buf;
//    }

//    in.close();
//    arDecoder.EndUnpacking();
//    inputStream.EndUnpacking();
//    out.close();
//  }
//  else
//  {
//    return -1;
//  }

//  return 0;
//}

#include <iostream>
#include <fstream>
#include <string.h>
#include <Arcoder.h>


#define PRINT_USAGE_AND_EXIT PrintUsage(argv[0]); return 1;

void PrintUsage(const char* i_pathToProgram)
{
  std::cout << "Usage: " << i_pathToProgram << " [e|d] /path/to/input /path/to/output" << std::endl;
}

int stupidModelChoosingCallback (int i_symbol)
{
  return i_symbol % 3;
}

int main(int argc, char** argv)
{
  if (argc != 4)
  {
    PRINT_USAGE_AND_EXIT;
  }

  bool compress = true;

  if (!strcmp(argv[1], "e"))
  {
    compress = true;
  }
  else if (!strcmp(argv[1], "d"))
  {
    compress = false;
  }
  else
  {
    PRINT_USAGE_AND_EXIT;
  }

  std::ifstream in(argv[2]);
  std::ofstream out(argv[3]);

  if (!in.good())
  {
    std::cout << "Couldn't open " << argv[2] << " for reading" << std::endl;
    return 2;
  }

  if (!out.good())
  {
    std::cout << "Couldn't open " << argv[3] << " for writing" << std::endl;
  }

//  std::vector<int> counts;
//  counts.push_back(256);
//  counts.push_back(256);
//  counts.push_back(256);

  Arcoder* coder = CreateArcoder();//counts);

//  coder->SetModelChoosingCallback(stupidModelChoosingCallback);

  if (compress)
  {
    coder->Compress(in, out);
  }
  else
  {
    coder->Decompress(in, out);
  }

  in.close();
  out.close();

  return 0;
}
