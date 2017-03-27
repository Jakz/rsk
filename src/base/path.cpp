#include "path.h"

#include <sys/stat.h>


bool path::exists() const
{
  struct stat buffer;
  return stat(data.c_str(), &buffer) == 0;
}
