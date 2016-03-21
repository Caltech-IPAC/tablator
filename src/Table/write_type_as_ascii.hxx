#pragma once

#include <iostream>

#include <H5Cpp.h>

namespace tablator
{
void write_type_as_ascii (std::ostream &os, const Data_Type &type,
                          const size_t &array_size,
                          const char *data,
                          const int &output_precision);
}
