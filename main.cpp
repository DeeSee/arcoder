#include <iostream>
#include <fstream>
#include <string.h>
#include <assert.h>
#include <limits.h>
#include <stdlib.h>

#include "Arcoder.h"


struct ArcoderSettings
{
	bool compress;
  int symbolsCount;
	std::string inputFileName;
	std::string outputFileName;
	std::string modelFileName;
	std::string modelChoosingRulesFileName;
	std::string modelToSaveFileName;

	ArcoderSettings()
	{
		compress = true;
    symbolsCount = -1;
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
        if (o_settings.symbolsCount != -1)
        {
          std::cout << "Characters count and model file must not be set simulteniously" << std::endl;
          return 1;
        }
				o_settings.modelFileName = std::string(i_argv[i + 1]);
			}
			else if (i_argv[i][1] == 'c')
			{
				o_settings.modelChoosingRulesFileName = std::string(i_argv[i + 1]);
			}
      else if (i_argv[i][1] == 'a')
      {
        if (!o_settings.modelFileName.empty())
        {
          std::cout << "Characters count and model file must not be set simulteniously" << std::endl;
          return 1;
        }
        o_settings.symbolsCount = atoi(i_argv[i + 1]);
      }
      else if (i_argv[i][1] == 's')
      {
        o_settings.modelToSaveFileName = std::string(i_argv[i + 1]);
      }
      else
      {
        std::cout << "Unrecognized option: " << i_argv[i] << std::endl;
        return 1;
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

int choosingCallback(int i_symbol, unsigned int i_symbolIndex, void* i_choosingTable)
{
  (void)i_symbolIndex;
  int* choosingTable = ((int *)i_choosingTable) + 1;
  int tableSize = *((int *)i_choosingTable);

  if (i_symbolIndex >= tableSize)
  {
    std::cout << "Reached the end of table, returning 0" << std::endl;
    return 0;
  }

  return choosingTable[i_symbolIndex];
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

  std::ifstream in;
  in.open(settings.inputFileName.c_str(), std::ifstream::in | std::ifstream::binary);
  std::ofstream out;
  out.open(settings.outputFileName.c_str(), std::ofstream::out | std::ofstream::binary);

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
    std::ifstream model;
    model.open(settings.modelFileName.c_str(), std::ifstream::in | std::ifstream::binary);
    if (!model.good())
    {
      std::cout << "Couldn't open " << settings.modelFileName.c_str() << " for reading" << std::endl;
      return 2;
    }

		coder = CreateArcoder(model);

    model.close();
  }
  else
  {
    coder = CreateArcoder();
  }

  assert(coder != NULL);

  int* modelChoosingTable = NULL;

  if (!settings.modelChoosingRulesFileName.empty())
  {
    std::ifstream modelChoosingRules;
    modelChoosingRules.open(settings.modelChoosingRulesFileName.c_str(), std::ifstream::in | std::ifstream::binary);
    if (!modelChoosingRules.good())
    {
      std::cout << "Couldn't open " << settings.modelChoosingRulesFileName.c_str() << " for reading" << std::endl;
      return 2;
    }

    int choosingRulesSize = 0;

    modelChoosingRules.read((char *)&choosingRulesSize, sizeof(choosingRulesSize));

    if (choosingRulesSize <= 0)
    {
      std::cout << "Invalid model choosing rules file" << std::endl;
      return 2;
    }

    modelChoosingTable = new int[2 * choosingRulesSize + 1];
    modelChoosingTable[0] = choosingRulesSize;

    modelChoosingRules.read((char *)(modelChoosingTable + 1), 2 * choosingRulesSize * sizeof(int));

    if (!(modelChoosingRules.good() || modelChoosingRules.eof()))
    {
      std::cout << "Failed to read file " << settings.modelChoosingRulesFileName.c_str() << std::endl;
      return 2;
    }

    modelChoosingRules.close();

    coder->SetModelChoosingCallback(choosingCallback, modelChoosingTable);
  }

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

  if (!settings.modelToSaveFileName.empty())
  {
    std::ofstream modelOut;
    modelOut.open(settings.modelToSaveFileName.c_str(), std::ofstream::out | std::ofstream::binary);
    coder->SaveModel(modelOut);
    modelOut.close();
  }

  if (modelChoosingTable)
  {
    delete [] modelChoosingTable;
  }

  return 0;
}
