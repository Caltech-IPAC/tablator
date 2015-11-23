#include "../Table.hxx"

std::vector<tablator::Table::Column> tablator::Table::columns () const
{
  std::vector<tablator::Table::Column> result;
  H5::PredType type;
  for (size_t index=1; index<types.size (); ++index)
    {
      size_t size=(type[index]!=Type::STRING ? 1
                   : offsets.at (index+1) - offsets.at (index));
      result.emplace_back
            ({ compound_type.getMemberName (index),
                { { H5_PredType (type[index]), size}
                    fields_properties[index]}});
    }
}
