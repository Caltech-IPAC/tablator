#include "../quote_sql_string.hxx"
#include "../Table.hxx"
#include "../Data_Type_to_SQL.hxx"
#include "../write_type_as_ascii.hxx"

void tablator::Table::write_insert_sql (std::ostream &os,
                                        const std::string &table_name) const
{
  std::string quoted_table_name (quote_sql_string (table_name, '"'));
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
                      data.data () + row_offset + offsets[column],
                      output_precision);
                  os << quote_sql_string (ss.str (), '\'');
                }
              else
                {
                  write_type_as_ascii (
                      os, columns[column].type, columns[column].array_size,
                      data.data () + row_offset + offsets[column],
                      output_precision);
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
