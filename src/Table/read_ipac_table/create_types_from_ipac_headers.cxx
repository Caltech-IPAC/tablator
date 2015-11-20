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

std::vector<tablator::Type>
get_data_types (std::vector<std::string> &data_types)
{
  std::vector<tablator::Type> types;
  for (auto &data_type : data_types)
    {
      std::string t=boost::to_lower_copy(data_type);
      auto size=t.size ();
    
      if (t.compare(0,size,"int")==0)
        {
          types.push_back (tablator::Type::INT);
        }
      else if (t.compare(0,size,"long")==0)
        {
          types.push_back (tablator::Type::LONG);
        }
      else if (t.compare(0,size,"float")==0)
        {
          types.push_back (tablator::Type::FLOAT);
        }
      else if (t.compare(0,size,"double")==0 || t.compare(0,size,"real")==0)
        {
          types.push_back (tablator::Type::DOUBLE);
        }
      else if (t.compare(0,size,"char")==0 || t.compare(0,size,"date")==0)
        {
          types.push_back (tablator::Type::STRING);
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
      switch (types[i])
        {
        case Type::BOOLEAN:
          append_member (columns[0].at(i), H5::PredType::NATIVE_UCHAR);
          break;

        case Type::SHORT:
          append_member (columns[0].at(i), H5::PredType::NATIVE_INT16);
          break;

        case Type::INT:
          append_member (columns[0].at(i), H5::PredType::NATIVE_INT32);
          break;

        case Type::LONG:
          append_member (columns[0].at(i), H5::PredType::NATIVE_INT64);
          break;

        case Type::FLOAT:
          append_member (columns[0].at(i), H5::PredType::NATIVE_FLOAT);
          break;
        case Type::DOUBLE:
          append_member (columns[0].at(i), H5::PredType::NATIVE_DOUBLE);
          break;

        case Type::STRING:
          string_types.emplace_back (0,ipac_column_widths.at(i));
          append_member (columns[0].at(i), *string_types.rbegin ());
          break;

        default:
          throw std::runtime_error ("Unsupported data type for column "
                                    + columns[0].at(i));
        }
    }

  fields_properties.push_back(Field_Properties("Packed bit array indicating whether a "
                                               "column is null",{}));
        
  for (size_t column = 1; column < num_columns; ++column)
    {
      Field_Properties p({});
      if (!columns[2].at(column).empty ())
        p.attributes.insert (std::make_pair
                             ("unit",boost::algorithm::trim_copy (columns[2].at(column))));
      if (!columns[3].at(column).empty ())
        p.values.null=boost::algorithm::trim_copy (columns[3].at(column));

      fields_properties.emplace_back (p);
    }
}

