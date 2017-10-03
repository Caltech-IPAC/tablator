#include "../Table.hxx"
#include "../Data_Type_to_SQL.hxx"
#include "../quote_sql_string.hxx"

#include <sqlite/connection.hpp>
#include <sqlite/execute.hpp>

void
tablator::Table::write_sqlite_db (const boost::filesystem::path &path) const
{
  sqlite::connection connection (path.native ());

  const std::string table_name (quote_sql_string(path.stem ().native (),'"',
                                                 Quote_SQL::IF_NEEDED));
  std::stringstream sql_stream;
  write_sql_create_table (sql_stream, table_name, Format::Enums::SQLITE_SQL);
  sql_stream << ";";
  sqlite::execute (connection, sql_stream.str (), true);

  for (size_t row_offset = 0; row_offset < data.size ();
       row_offset += row_size ())
    {
      sql_stream.str ("");
      write_sql_insert (sql_stream, table_name, row_offset);
      sqlite::execute (connection, sql_stream.str (), true);
    }
}
