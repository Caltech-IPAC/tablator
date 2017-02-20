#pragma once

#include <H5Cpp.h>

namespace tablator
{
class HDF5_Min_Max
{
public:
  const char *value;
  int8_t inclusive;
};
}
