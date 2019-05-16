#pragma once

#include <H5Cpp.h>
#include "Data_Type.hxx"

namespace tablator {
Data_Type H5_to_Data_Type(const H5::DataType &H5_type);
}
