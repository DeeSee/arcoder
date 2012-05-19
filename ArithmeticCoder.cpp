#include "ArithmeticCoder.h"
#include "ArithmeticCoderImplementation.h"

ArithmeticCoder* CreateCoder()
{
  return new ArithmeticCoderImplementation();
}
