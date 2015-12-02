#include <stdexcept>

#include "../Table.hxx"
#include "../to_string.hxx"

std::vector<tablator::Table::Column> tablator::Table::columns () const
{
  std::vector<tablator::Table::Column> result;
  for (int index=1; index<compound_type.getNmembers (); ++index)
    {
      size_t size=1;
      auto datatype=compound_type.getMemberDataType (index);
      if (datatype.getClass ()==H5T_STRING)
        {
          size=datatype.getSize ();
          datatype=H5::PredType::C_S1;
        }
      else if (datatype.getClass () == H5T_ARRAY)
        {
          auto predtype=datatype.getSuper ();
          size=datatype.getSize ()/ predtype.getSize ();
          datatype=predtype;
        }
      // FIXME: There should be one of these arrays with properties
      // corresponding to to_string, to_ipac_string,
      // CCfits::ValueType, etc.
      std::vector<H5::PredType> predtypes=
        {H5::PredType::STD_I8LE, H5::PredType::STD_U8LE,
         H5::PredType::STD_I16LE, H5::PredType::STD_U16LE,
         H5::PredType::STD_I32LE, H5::PredType::STD_U32LE,
         H5::PredType::STD_I64LE, H5::PredType::STD_U64LE,
         H5::PredType::IEEE_F32LE, H5::PredType::IEEE_F64LE,
         H5::PredType::C_S1};
      auto p=std::find (predtypes.begin (), predtypes.end (), datatype);
      if (p==predtypes.end ())
        throw std::runtime_error ("Invalid data type in " __FILE__ ": "
                                  + to_string (datatype));
      result.push_back
        ({ compound_type.getMemberName (index),
            { { *p, size}, fields_properties[index]}});
    }
  return result;
}
