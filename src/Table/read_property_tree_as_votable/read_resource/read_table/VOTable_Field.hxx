#pragma once

#include "../../../../Column.hxx"

namespace tablator
{
/// This is almost the same as Column
class VOTable_Field : public Column
{
public:
  bool is_array_dynamic = false;

  VOTable_Field () : Column("", Data_Type::UINT8_LE, 1) {}
  VOTable_Field (const std::string &Name, const Data_Type &Type,
                 const bool &Array_size, const Field_Properties &Properties)
    : Column(Name,Type,Array_size,Properties)
  {
  }
};
}
