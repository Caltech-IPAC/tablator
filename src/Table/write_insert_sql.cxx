#include "../quote_sql_string.hxx"
#include "../Table.hxx"
#include "../Data_Type_to_SQL.hxx"
#include "../write_type_as_ascii.hxx"

void tablator::Table::write_insert_sql (std::ostream &os,
                                        const std::string &table_name) const
  // ,
  //                                       const std::pair<std::string,std::string> &point_input,
  //                                       const std::string &point_column_name,
  //                                       const std::vector<std::pair<std::string,std::string>> &poly_input,
  //                                       const std::string &poly_column_name) const
{
  std::string quoted_table_name (quote_sql_string (table_name, '"'));
  // std::pair<size_t,size_t> point_input_offsets;
  // std::pair<Data_Type,Data_Type> point_input_types;
  // std::vector<std::pair<size_t,size_t>> poly_offsets;
  // if (!point_column_name.empty ())
  //   {
  //     auto
  //     point_offsets.first = column_offset (point_input.first);
  //     point_offsets.second = column_offset (point_input.second);
  //   }
  for (size_t row_offset = 0; row_offset < data.size ();
       row_offset += row_size ())
    {
      os << "INSERT INTO " << quoted_table_name << "\nVALUES (";
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
