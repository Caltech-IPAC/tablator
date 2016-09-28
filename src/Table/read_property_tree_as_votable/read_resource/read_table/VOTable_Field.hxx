#pragma once

#include "../../../../Data_Type.hxx"
#include "../../../../Field_Properties.hxx"

namespace tablator
{
class VOTable_Field
{
public:
  std::string name;
  Data_Type type = Data_Type::UINT8_LE;
  size_t array_size = 1;
  bool is_array_dynamic = false;
  Field_Properties properties;

  VOTable_Field () = default;
  VOTable_Field (const std::string &Name, const Data_Type &Type,
                 const bool &Array_size, const Field_Properties &Properties)
      : name (Name), type (Type), array_size (Array_size),
        properties (Properties)
  {
  }
};
}
