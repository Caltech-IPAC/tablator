#pragma once

namespace tablator
{
class VOTable_Field
{
public:
  std::string name;
  H5::PredType predtype=H5::PredType::STD_U8LE;
  bool is_array=false;
  Field_Properties properties;

  VOTable_Field () = default;
  VOTable_Field (const std::string &Name, const H5::PredType &Predtype,
                 const bool &Is_array, const Field_Properties &Properties) :
    name (Name), predtype (Predtype), is_array (Is_array),
    properties (Properties) {}
};
}
