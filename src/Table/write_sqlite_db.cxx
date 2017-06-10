#include "../Table.hxx"
#include "../Data_Type_to_SQL.hxx"
#include "../write_type_as_ascii.hxx"

#include <sqlite/connection.hpp>
#include <sqlite/execute.hpp>
#include <sqlite/query.hpp>

void
tablator::Table::write_sqlite_db (const boost::filesystem::path &path) const
{
  std::stringstream create_table_stream;
  write_sql (create_table_stream, path.stem ().native (),
             Format::Enums::SQLITE_SQL);
  std::string create_table (create_table_stream.str ());

  sqlite::connection connection (path.native ());
  sqlite::execute create (connection, create_table, true);
}
