#include "../quote_sql_string.hxx"
#include "../Table.hxx"
#include "../Data_Type_to_SQL.hxx"
#include "../write_type_as_ascii.hxx"

namespace
{
void write_point (std::ostream &os,
                  const std::pair<std::pair<size_t,tablator::Data_Type>,
                  std::pair<size_t,tablator::Data_Type>> &point_input,
                  const uint8_t *row_start)
{
  os << "ST_MakePoint(";
  write_type_as_ascii (os, point_input.first.second, 1,
                       row_start + point_input.first.first);
  os << ", ";
  write_type_as_ascii (os, point_input.second.second, 1,
                       row_start + point_input.second.first);
  os << "),\n";
}

std::pair<size_t,tablator::Data_Type> get_offsets_and_types (const tablator::Table &table,
                                                             const std::string &name)
{
  auto column (table.find_column (name));
  if (column == table.columns.end ())
    {
      throw std::runtime_error ("Unable to find the column '"
                                + name + "' when creating geometries.");
    }
  else if (column->type == tablator::Data_Type::CHAR)
    {
      throw std::runtime_error ("Input columns to geography must be numeric.  The column '"
                                + name + "' is text");
    }
  else if (column->array_size != 1)
    {
      throw std::runtime_error ("Input columns to geography must not be arrays.  The column '"
                                + name + "' has array size="
                                + std::to_string (column->array_size));
    }
  return std::pair<size_t,tablator::Data_Type>
    (table.offsets[std::distance (table.columns.begin (), column)], column->type);
}

std::pair<std::pair<size_t,tablator::Data_Type>,std::pair<size_t,tablator::Data_Type>>
get_offsets_and_types (const tablator::Table &table,
                       const std::pair<std::string,std::string> &names)
{
  return std::make_pair(get_offsets_and_types (table, names.first),
                        get_offsets_and_types (table, names.second));  
}
  
}

void tablator::Table::write_insert_sql
(std::ostream &os,
 const std::string &table_name,
 const std::pair<std::string,std::string> &point_input_names,
 const std::vector<std::pair<std::string,std::string>> &polygon_input_names) const
{
  std::string quoted_table_name (quote_sql_string (table_name, '"',
                                                   Quote_SQL::IF_NEEDED));
  std::pair<std::pair<size_t,Data_Type>,std::pair<size_t,Data_Type>> point_input;
  if (!point_input_names.first.empty ())
    {
      point_input = get_offsets_and_types (*this, point_input_names);
    }
  std::vector<std::pair<std::pair<size_t,Data_Type>,std::pair<size_t,Data_Type>>> polygon_input;
  for (auto &names: polygon_input_names)
    {
      polygon_input.emplace_back (get_offsets_and_types (*this, names));
    }
  
  for (size_t row_offset = 0; row_offset < data.size ();
       row_offset += row_size ())
    {
      os << "INSERT INTO " << quoted_table_name << "\nVALUES (";
      if (!point_input_names.first.empty ())
        {
          write_point (os, point_input, data.data () + row_offset);
        }
      for (auto &point: polygon_input)
        {
          write_point (os, point, data.data () + row_offset);
        }

      for (size_t column = 1; column < columns.size (); ++column)
        {
          if (is_null (row_offset, column))
            {
              os << "NULL";
            }
          else
            {
              if (columns[column].type == Data_Type::CHAR)
                {
                  std::stringstream ss;
                  write_type_as_ascii (
                      ss, columns[column].type, columns[column].array_size,
                      data.data () + row_offset + offsets[column]);
                  os << quote_sql_string (ss.str (), '\'');
                }
              else
                {
                  write_type_as_ascii (
                      os, columns[column].type, columns[column].array_size,
                      data.data () + row_offset + offsets[column]);
                }
            }
          if (column + 1 != columns.size ())
            {
              os << ", ";
            }
          else
            {
              os << ");\n";
            }
        }
    }
}
