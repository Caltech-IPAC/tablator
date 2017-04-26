#pragma once

#include <iostream>

#include "Data_Type.hxx"

namespace tablator
{
void write_type_as_ascii (std::ostream &os, const Data_Type &type,
                          const size_t &array_size, const uint8_t *data,
                          const int &output_precision);
}
