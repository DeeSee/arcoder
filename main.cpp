#include <iostream>
#include <fstream>
#include <string.h>
#include <assert.h>

#include "Arcoder.h"


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

int ParseOptions(int i_argc, const char** i_argv, ArcoderSettings& o_settings)
{
	int i = 1;
	bool compressOptionIsSet = false;

  o_settings = ArcoderSettings();
	
	while (i < i_argc)
	{
		const int kOptionLength = 2;
		if (i_argv[i][0] != '-' || strlen(i_argv[i]) != kOptionLength)
		{
			std::cout << "Error: unknown option " << i_argv[i] << std::endl;
			return 1;
		}

		if ((i_argv[i][1] == 'e') || (i_argv[i][1] == 'd'))
		{
			if (compressOptionIsSet)
			{
				std::cout << "Error: -e and -d options must not be presented simultaniously or more than once!" << std::endl;
				return 1;
			}

			compressOptionIsSet = true;
			o_settings.compress = (i_argv[i][1] == 'e');
		} 
		else // all other options must have argument
		{
			if (i + 1 == i_argc)
			{
				std::cout << "Error: option " << i_argv[i] << " must have an argument" << std::endl;
				return 1;
			}

			if (i_argv[i][1] == 'i')
			{
				o_settings.inputFileName = std::string(i_argv[i + 1]);
			}
			else if (i_argv[i][1] == 'o')
			{
				o_settings.outputFileName = std::string(i_argv[i + 1]);
			}
			else if (i_argv[i][1] == 'm')
			{
				o_settings.modelFileName = std::string(i_argv[i + 1]);
			}
			else if (i_argv[i][1] == 'c')
			{
				o_settings.modelFileName = std::string(i_argv[i + 1]);
			}

			i++;
		}

		i++;
	}

	if (o_settings.inputFileName.empty())
	{
		std::cout << "Input file name must be specified!" << std::endl;
		return 1;
	}

	if (o_settings.outputFileName.empty())
	{
		std::cout << "Output file name must be specified!" << std::endl;
		return 1;
	}

	return 0;
}

int main(int argc, char** argv)
{
  ArcoderSettings settings;

  if (ParseOptions(argc, (const char **)argv, settings) != 0)
  {
	  std::cout << "Failed to parse options." << std::endl;
	  PrintUsage(argv[0]);
    return 1;
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
