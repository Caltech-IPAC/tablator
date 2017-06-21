#include "../quote_sql_string.hxx"
#include "../Table.hxx"
#include "../Data_Type_to_SQL.hxx"

namespace
{
  std::string append_number (const std::string &column_name)
  {
    auto underscore (column_name.rfind ("_"));
    if (underscore != std::string::npos)
      {
        try
          {
            int current_number (std::stoull (column_name.substr(underscore + 1)));
            return column_name.substr (0, underscore)
              + "_" + std::to_string (current_number + 1);
          }
        catch (...)
          {
          }
      }
    return column_name + "_1";
  }
  
}

void
tablator::Table::write_create_table_sql (std::ostream &os,
                                         const std::string &table_name,
                                         const Format::Enums &sql_type,
                                         const bool &append_numbers_to_columns) const
{
  std::string quoted_table_name (quote_sql_string (table_name, '"'));
  os << "CREATE TABLE " << quoted_table_name << " (\n";
  for (size_t i = 1; i < columns.size (); ++i)
    {
      std::string column_name (boost::to_upper_copy (columns[i].name));
      if (append_numbers_to_columns)
        {
          column_name = append_number (column_name);
        }
      os << quote_sql_string (column_name, '"')
         << " " << Data_Type_to_SQL (columns[i].type, sql_type);
      if (i + 1 != columns.size ())
        {
          os << ",";
        }
      os << "\n";
    }
  os << ");\n";
}
