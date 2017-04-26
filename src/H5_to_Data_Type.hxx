#pragma once

#include "Data_Type.hxx"
#include <H5Cpp.h>

namespace tablator
{
Data_Type H5_to_Data_Type (const H5::DataType &H5_type);
}
