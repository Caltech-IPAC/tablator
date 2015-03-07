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

std::vector<Tablator::Table::Type>
get_data_types (std::vector<std::string> &data_types)
{
  std::vector<Tablator::Table::Type> types;
  for (auto &data_type : data_types)
    {
      std::string t=boost::to_lower_copy(data_type);
      auto size=t.size ();
    
      if (t.compare(0,size,"int")==0)
        {
          types.push_back (Tablator::Table::Type::INT);
        }
      else if (t.compare(0,size,"long")==0)
        {
          types.push_back (Tablator::Table::Type::LONG);
        }
      else if (t.compare(0,size,"float")==0)
        {
          types.push_back (Tablator::Table::Type::FLOAT);
        }
      else if (t.compare(0,size,"double")==0 || t.compare(0,size,"real")==0)
        {
          types.push_back (Tablator::Table::Type::DOUBLE);
        }
      else if (t.compare(0,size,"char")==0 || t.compare(0,size,"date")==0)
        {
          types.push_back (Tablator::Table::Type::STRING);
        }
      else
        {
          throw std::runtime_error ("Unknown data type in IPAC table: " + data_type);
        }
    }
  return types;
}

}

void Tablator::Table::create_types_from_ipac_headers
(std::array<std::vector<std::string>,4> &columns,
 std::vector<size_t> &ipac_column_offsets,
 std::vector<size_t> &ipac_column_widths)
{
  types = get_data_types (columns[1]);

  row_size = 0;
  const size_t num_columns = columns[0].size ();
  ipac_column_widths = get_ipac_column_widths (ipac_column_offsets);

  for (size_t i = 0; i < num_columns; ++i)
    {
      offsets.push_back (row_size);
      switch (types.at(i))
        {
          /// BOOLEAN and SHORT should never be used, but we include
          /// them here just in case
        case Type::BOOLEAN:
          row_size += H5::PredType::NATIVE_UCHAR.getSize ();
          break;
        case Type::SHORT:
          row_size += H5::PredType::NATIVE_INT16.getSize ();
          break;
        case Type::INT:
          row_size += H5::PredType::NATIVE_INT32.getSize ();
          break;
        case Type::LONG:
          row_size += H5::PredType::NATIVE_INT64.getSize ();
          break;
        case Type::FLOAT:
          row_size += H5::PredType::NATIVE_FLOAT.getSize ();
          break;
        case Type::DOUBLE:
          row_size += H5::PredType::NATIVE_DOUBLE.getSize ();
          break;
        case Type::STRING:
          row_size += ipac_column_widths.at(i);
          break;

        default:
          throw std::runtime_error ("Unsupported data type for column "
                                    + columns[0].at(i));
        }
    }
  offsets.push_back (row_size);

  compound_type = H5::CompType (row_size);
  for (size_t i = 0; i < num_columns; ++i)
    {
      switch (types[i])
        {
        case Type::BOOLEAN:
          compound_type.insertMember (columns[0].at(i), offsets[i],
                                      H5::PredType::NATIVE_UCHAR);
          break;

        case Type::SHORT:
          compound_type.insertMember (columns[0].at(i), offsets[i],
                                      H5::PredType::NATIVE_INT16);
          break;

        case Type::INT:
          compound_type.insertMember (columns[0].at(i), offsets[i],
                                      H5::PredType::NATIVE_INT32);
          break;

        case Type::LONG:
          compound_type.insertMember (columns[0].at(i), offsets[i],
                                      H5::PredType::NATIVE_INT64);
          break;

        case Type::FLOAT:
          compound_type.insertMember (columns[0].at(i), offsets[i],
                                      H5::PredType::NATIVE_FLOAT);
          break;
        case Type::DOUBLE:
          compound_type.insertMember (columns[0].at(i), offsets[i],
                                      H5::PredType::NATIVE_DOUBLE);
          break;

        case Type::STRING:
          string_types.emplace_back (0,ipac_column_widths.at(i));
          compound_type.insertMember (columns[0].at(i), offsets[i],
                                      *string_types.rbegin ());
          break;
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
        p.attributes.insert (std::make_pair
                             ("null",boost::algorithm::trim_copy (columns[3].at(column))));

      fields_properties.emplace_back (p);
    }
}

