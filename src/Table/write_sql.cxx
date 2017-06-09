#include "../Table.hxx"
#include "../Data_Type_to_SQL.hxx"
#include "../write_type_as_ascii.hxx"

namespace
{
std::string quote_string (const std::string &input, const char &quote)
{
  std::stringstream stream;
  stream << quote;
  for (auto &c: input)
    {
      stream << c;
      if (c==quote)
        {
          stream << quote;
        }
    }
  stream << quote;
  return stream.str ();
}
}

void tablator::Table::write_sql (std::ostream &os,
                                 const std::string &table_name,
                                 const Format::Enums &output_type) const
{
  std::string quoted_table_name (quote_string(table_name,'"'));
  os << "CREATE TABLE " << quoted_table_name << "(\n";
  for (size_t i = 1; i < columns.size (); ++i)
    {
      os << quote_string (boost::to_upper_copy(columns[i].name),'"') << " "
         << Data_Type_to_SQL(columns[i].type,output_type);
      if (i + 1 != columns.size ())
        {
          os << ",";
        }
      os << "\n";
    }
  os << ");\n";

  for (size_t row_offset = 0; row_offset < data.size (); row_offset+=row_size ())
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
                  write_type_as_ascii (ss, columns[column].type,
                                       columns[column].array_size,
                                       data.data () + row_offset + offsets[column],
                                       output_precision);
                  os << quote_string(ss.str (),'\'');
                }
              else
                {
                  write_type_as_ascii (os, columns[column].type,
                                       columns[column].array_size,
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
