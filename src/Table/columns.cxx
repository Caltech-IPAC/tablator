#include "../Table.hxx"

std::vector<tablator::Table::Column> tablator::Table::columns () const
{
  std::vector<tablator::Table::Column> result;
  for (size_t index=1; index<compound_type.getNmembers (); ++index)
    {
      size_t size=1;
      auto datatype=compound_type.getMemberDataType (index);
      H5::PredType predtype=dynamic_cast<H5::PredType>(datatype.getSuper ());
      if (datatype.getClass ()==H5T_STRING)
        {
          size=datatype.getSize ();
        }
      else if (datatype.getClass () == H5T_ARRAY)
        {
          auto predtype=datatype.getSuper ();
          size=datatype.getSize ()/ predtype.getSize ();
          datatype=predtype;
        }
      result.emplace_back
            ({ compound_type.getMemberName (index),
                { { predtype, size}
                    fields_properties[index]}});
    }
}
