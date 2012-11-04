#ifndef ARCODER_H
#define ARCODER_H

#include <istream>
#include <ostream>
#include <vector>

/*! \brief Callback to choose model index.
 *
 *  If not set, model with index 0 will be always used.
 *
 *  \param i_symbol The symbol previous to the one being encoded.
 *  \param i_userData Pointer to the data that used wants to pass to this func.
 *  \return The index of the model that should be chosen for this symbol.
 */
typedef int (*modelChoosingCallback) (int i_symbol, unsigned int i_serialIndex, void* i_userData);

class Arcoder
{
public:

  virtual ~Arcoder() {}

  virtual bool LoadModel(std::istream& i_stream) = 0;
  virtual void SaveModel(std::ostream& i_stream) = 0;

  virtual void InitUniformModel(unsigned int i_symbolsCount) = 0;

  virtual void SetModelChoosingCallback(modelChoosingCallback i_callback, void* i_userData) = 0;
  virtual void SetElementSize(int i_size) = 0;

  virtual void Compress(std::istream& i_input,
                        std::ostream& i_output) = 0;

  virtual void Decompress(std::istream& i_input,
                          std::ostream& i_output) = 0;

};

Arcoder* CreateArcoder(int i_symbolCount = 256); // single model

Arcoder* CreateArcoder(const std::vector<int>& i_symbolCount);

Arcoder* CreateArcoder(std::istream& i_modelsSource);

#endif // ARCODER_H
