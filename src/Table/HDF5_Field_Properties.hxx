#pragma once

#include "HDF5_Values.hxx"

namespace tablator
{
class HDF5_Field_Properties
{
public:
  const char *description;
  hvl_t attributes, links;
  HDF5_Values values;
};
}
