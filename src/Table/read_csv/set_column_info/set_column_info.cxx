#include "../../../Table.hxx"

namespace tablator
{
  Data_Type get_best_data_type (const Data_Type &current_type,
                                const std::string &element);
}

// FIXME: A bit icky.  This modifies the csv document (trims
// whitespace) while extracting metadata.
void tablator::Table::set_column_info (CSV::CSVDocument &csv)
{
  auto row (csv.begin());
  std::vector<std::string> names;
  for (auto &name: *row)
    names.push_back(boost::trim_copy(name));
  
  append_column("null_bitfield_flags", Data_Type::UINT8_LE,
                (names.size() + 7) / 8,
                Field_Properties (null_bitfield_flags_description));
  
  /// Try to infer the types of the columns.  Supported are INT8_LE
  /// (bool), INT64_LE, UINT64_LE, FLOAT64_LE, and CHAR.

  std::vector<Data_Type> types (names.size(), Data_Type::INT8_LE);
  std::vector<size_t> sizes (names.size(), 1);
  std::vector<std::vector<std::string> > strings;
  
  size_t line_number (1);
  ++row;
  for (; row!=csv.end(); ++row)
    {
      if (row->size()!=names.size())
        throw std::runtime_error("In line " + std::to_string(line_number)
                                 + ", expected "
                                 + std::to_string(names.size())
                                 + " elements, but only found "
                                 + std::to_string(row->size()));
      std::vector<std::string> row_strings;
      for (size_t elem=0; elem < row->size(); ++elem)
        {
          std::string &element ((*row)[elem]);
          boost::algorithm::trim (element);
          types[elem]=get_best_data_type(types[elem], element);
          sizes[elem] = std::max(sizes[elem], element.size());
        }
    }
  
  for (size_t elem=0; elem < names.size(); ++elem)
    append_column(names[elem], types[elem],
                  types[elem]==Data_Type::CHAR ? sizes[elem] : 1);
}