#pragma once

#include <iostream>

#include <H5Cpp.h>

namespace tablator
{
void write_type_as_ascii (std::ostream &os, const H5::DataType &type,
                          const char *data, const size_t &size,
                          const int &output_precision);
}
