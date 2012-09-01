#include <iostream>
#include <fstream>
#include <string.h>

#include "Arcoder.h"


#define PRINT_USAGE_AND_EXIT PrintUsage(argv[0]); return 1;

void PrintUsage(const char* i_pathToProgram)
{
  std::cout << "Usage: " << i_pathToProgram << " [e|d] /path/to/input /path/to/output" << std::endl;
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

  Arcoder* coder = CreateArcoder();

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
