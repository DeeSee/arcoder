#include <iostream>
#include <fstream>
#include <string.h>
#include <assert.h>

#include "Arcoder.h"


#define PRINT_USAGE_AND_EXIT PrintUsage(argv[0]); return 1;

struct ArcoderSettings
{
	bool compress;
	std::string inputFileName;
	std::string outputFileName;
	std::string modelFileName;
	std::string modelChoosingRulesFileName;

	ArcoderSettings()
	{
		compress = true;
		inputFileName = std::string();
		outputFileName = std::string();
		inputFileName = std::string();
		modelChoosingRulesFileName = std::string();
	}
};

void PrintUsage(const char* i_pathToProgram)
{
  std::cout << "Usage: " << i_pathToProgram << " -e|d -i /path/to/input -o /path/to/output [-m /path/to/model] [-c /path/to/model/choosing/table]" << std::endl;
}

int ParseOptions(int argc, char** argv, ArcoderSettings& settings)
{
	int i = 1;
	bool compressOptionIsSet = false;
	
	while (i < argc)
	{
		const int kOptionLength = 2;
		if (argv[i][0] != '-' || strlen(argv[i]) != kOptionLength)
		{
			std::cout << "Error: unknown option " << argv[i] << std::endl;
			return 1;
		}

		if ((argv[i][1] == 'e') || (argv[i][1] == 'd'))
		{
			if (compressOptionIsSet)
			{
				std::cout << "Error: -e and -d options must not be presented simultaniously or more than once!" << std::endl;
				return 1;
			}

			compressOptionIsSet = true;
			settings.compress = (argv[i][1] == 'e');
		} 
		else // all other options must have argument
		{
			if (i + 1 == argc)
			{
				std::cout << "Error: option " << argv[i] << " must have an argument" << std::endl;
				return 1;
			}

			if (argv[i][1] == 'i')
			{
				settings.inputFileName = std::string(argv[i + 1]);
			}
			else if (argv[i][1] == 'o')
			{
				settings.outputFileName = std::string(argv[i + 1]);
			}
			else if (argv[i][1] == 'm')
			{
				settings.modelFileName = std::string(argv[i + 1]);
			}
			else if (argv[i][1] == 'c')
			{
				settings.modelFileName = std::string(argv[i + 1]);
			}

			i++;
		}

		i++;
	}

	if (settings.inputFileName.empty())
	{
		std::cout << "Input file name must be specified!" << std::endl;
		return 1;
	}

	if (settings.outputFileName.empty())
	{
		std::cout << "Output file name must be specified!" << std::endl;
		return 1;
	}

	return 0;
}

int main(int argc, char** argv)
{
  ArcoderSettings settings;

  if (ParseOptions(argc, argv, settings) != 0)
  {
	  std::cout << "Failed to parse options." << std::endl;
	  PRINT_USAGE_AND_EXIT;
  }

  std::ifstream in(settings.inputFileName.c_str());
  std::ofstream out(settings.outputFileName.c_str());

  if (!in.good())
  {
    std::cout << "Couldn't open " << settings.inputFileName.c_str() << " for reading" << std::endl;
    return 2;
  }

  if (!out.good())
  {
    std::cout << "Couldn't open " << settings.outputFileName.c_str() << " for writing" << std::endl;
    return 2;
  }

  Arcoder* coder = NULL;
  
  if (!settings.modelFileName.empty())
  {
    std::ifstream model(settings.modelFileName.c_str());
    if (!model.good())
    {
      std::cout << "Couldn't open " << settings.modelFileName.c_str() << " for reading" << std::endl;
      return 2;
    }

		coder = CreateArcoder(model);
  }
  else
  {
    coder = CreateArcoder();
  }

  assert(coder != NULL);

  if (settings.compress)
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
