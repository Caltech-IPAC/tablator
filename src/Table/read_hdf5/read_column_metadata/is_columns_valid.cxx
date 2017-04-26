#include <H5Cpp.h>

#include <iostream>
namespace
{
bool is_key_value_type (const H5::CompType &key_value)
{
  return key_value.getNmembers () == 2
         && key_value.getMemberClass (0) == H5T_STRING
         && key_value.getMemberClass (1) == H5T_STRING;
}

bool is_min_max_type (const H5::CompType &min_max)
{
  return min_max.getNmembers () == 2
         && min_max.getMemberClass (0) == H5T_STRING
         && min_max.getMemberClass (1) == H5T_INTEGER
         && min_max.getMemberDataType (1).getSize ()
            == H5::PredType::STD_I8LE.getSize ();
}
}

namespace tablator
{
bool is_columns_valid (H5::VarLenType &columns)
{
  H5::DataType columns_super = columns.getSuper ();
  if (columns_super.getClass () != H5T_COMPOUND)
    {
      return false;
    }

  H5::CompType column (columns_super.getId ());
  if (!(column.getNmembers () == 4 && column.getMemberClass (0) == H5T_STRING
        && column.getMemberClass (1) == H5T_STRING
        && column.getMemberClass (2) == H5T_INTEGER
        && column.getMemberDataType (2).getSize ()
           == H5::PredType::STD_U64LE.getSize ()
        && column.getMemberClass (3) == H5T_COMPOUND))
    {
      return false;
    }

  H5::CompType field_properties (column.getMemberCompType (3));
  if (!(field_properties.getNmembers () == 4
        && field_properties.getMemberClass (0) == H5T_STRING
        && field_properties.getMemberClass (1) == H5T_VLEN
        && field_properties.getMemberClass (2) == H5T_VLEN
        && field_properties.getMemberClass (3) == H5T_COMPOUND))
    {
      return false;
    }

  H5::DataType attributes_super (
      field_properties.getMemberVarLenType (1).getSuper ());
  if (attributes_super.getClass () != H5T_COMPOUND
      || !is_key_value_type (H5::CompType (attributes_super.getId ())))
    {
      return false;
    }

  H5::DataType links_super (
      field_properties.getMemberVarLenType (1).getSuper ());
  if (links_super.getClass () != H5T_COMPOUND
      || !is_key_value_type (H5::CompType (links_super.getId ())))
    {
      return false;
    }

  H5::CompType values (field_properties.getMemberCompType (3));
  if (!(values.getNmembers () == 7 && values.getMemberClass (0) == H5T_COMPOUND
        && values.getMemberClass (1) == H5T_COMPOUND
        && values.getMemberClass (2) == H5T_STRING
        && values.getMemberClass (3) == H5T_STRING
        && values.getMemberClass (4) == H5T_STRING
        && values.getMemberClass (5) == H5T_STRING
        && values.getMemberClass (6) == H5T_VLEN))
    {
      return false;
    }

  if (!is_min_max_type (values.getMemberCompType (0))
      || !is_min_max_type (values.getMemberCompType (1)))
    {
      return false;
    }

  H5::VarLenType options = values.getMemberVarLenType (6);
  H5::DataType options_super = options.getSuper ();
  if (options_super.getClass () != H5T_COMPOUND)
    {
      return false;
    }
  if (!is_key_value_type (H5::CompType (options_super.getId ())))
    {
      return false;
    }

  return true;
}
}
