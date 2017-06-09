#include "../Table.hxx"
#include "../Data_Type_to_SQL.hxx"

void tablator::Table::write_sql (std::ostream &os,
                                 const std::string &table_name,
                                 const Format::Enums &output_type) const
{
  os << "CREATE TABLE " << table_name << "{\n";
  for (size_t i = 1; i < columns.size (); ++i)
    {
      std::string upper_cased_name (boost::to_upper_copy(columns[i].name));
      os << "\"";
      for (auto &c: boost::to_upper_copy(columns[i].name))
        {
          os << c;
          if (c=='\"')
            {
              os << c;
            }
        }
      os << "\" " << Data_Type_to_SQL(columns[i].type,output_type);
      if (i + 1 != columns.size ())
        {
          os << ",";
        }
      os << "\n";
    }
  os << "};";
  
}
