#include "../../Table.hxx"

namespace {
std::vector<size_t> get_ipac_column_widths (std::vector<size_t> &ipac_column_offsets)
{
  const size_t num_columns=ipac_column_offsets.size () - 1;
  std::vector<size_t> ipac_column_widths;
  /// Add a column for null flags.
  ipac_column_widths.push_back ((num_columns + 7)/8);
  for (size_t i = 0; i < num_columns; ++i)
    ipac_column_widths.push_back (ipac_column_offsets[i + 1]
                                  - ipac_column_offsets[i] - 1);
  return ipac_column_widths;
}

std::vector<H5::PredType>
get_data_types (std::vector<std::string> &data_types)
{
  std::vector<H5::PredType> types;
  for (auto &data_type : data_types)
    {
      std::string t=boost::to_lower_copy(data_type);
      if (t=="int")
        {
          types.push_back (H5::PredType::STD_I32LE);
        }
      else if (t=="long")
        {
          types.push_back (H5::PredType::STD_I64LE);
        }
      else if (t=="float")
        {
          types.push_back (H5::PredType::IEEE_F32LE);
        }
      else if (t=="double" || t=="real")
        {
          types.push_back (H5::PredType::IEEE_F64LE);
        }
      else if (t=="char" || t=="date")
        {
          types.push_back (H5::PredType::C_S1);
        }
      else
        {
          throw std::runtime_error ("Unknown data type in IPAC table: " + data_type);
        }
    }
  return types;
}
}

void tablator::Table::create_types_from_ipac_headers
(std::array<std::vector<std::string>,4> &columns,
 std::vector<size_t> &ipac_column_offsets,
 std::vector<size_t> &ipac_column_widths)
{
  types = get_data_types (columns[1]);

  row_size = 0;
  const size_t num_columns = columns[0].size ();
  ipac_column_widths = get_ipac_column_widths (ipac_column_offsets);

  offsets.push_back (0);
  for (size_t i = 0; i < num_columns; ++i)
    {
      if (types[i]==H5::PredType::C_S1)
        {
          string_types.emplace_back (0,ipac_column_widths.at(i));
          append_member (columns[0].at(i), *string_types.rbegin ());
        }
      else
        {
          append_member (columns[0].at(i), types[i]);
        }
    }

  fields_properties.push_back (Field_Properties ("Packed bit array indicating "
                                                 "whether a column is null",{}));
        
  for (size_t column = 1; column < num_columns; ++column)
    {
      Field_Properties p({});
      if (!columns[2].at(column).empty ())
        p.attributes.insert
          (std::make_pair
           ("unit",boost::algorithm::trim_copy (columns[2].at(column))));
      if (!columns[3].at(column).empty ())
        p.values.null=boost::algorithm::trim_copy (columns[3].at(column));

      fields_properties.emplace_back (p);
    }
}

