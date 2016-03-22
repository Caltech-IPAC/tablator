#pragma once

#include "data_size.hxx"
#include "Field_Properties.hxx"

namespace tablator
{
class Column
{
public:
  std::string name;
  Data_Type type;
  size_t array_size;
  Field_Properties field_properties;

  Column (const std::string &Name, const Data_Type &Type,
          const size_t &Array_size)
    : Column (Name, Type, Array_size, Field_Properties ()) {}

  Column (const std::string &Name, const Data_Type &Type,
          const size_t &Array_size, const Field_Properties &Field_properties)
    : name (Name), type (Type), array_size (Array_size),
      field_properties (Field_properties) {}

  size_t data_size () const
  {
    return tablator::data_size (type) * array_size;
  }
};
}
