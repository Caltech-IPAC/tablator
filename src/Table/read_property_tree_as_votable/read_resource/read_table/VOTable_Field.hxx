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
  bool is_array = false;
  Field_Properties properties;

  VOTable_Field () = default;
  VOTable_Field (const std::string &Name, const Data_Type &Type,
                 const bool &Is_array, const Field_Properties &Properties)
      : name (Name), type (Type), is_array (Is_array), properties (Properties)
  {
  }
};
}
