#include "../Table.hxx"
#include "../Data_Type_to_SQL.hxx"
#include "../write_type_as_ascii.hxx"

#include <sqlite3.h>

void tablator::Table::write_sqlite_db (const boost::filesystem::path &path) const
{
  std::stringstream create_table_stream;
  write_sql (create_table_stream, path.stem ().native (), Format::Enums::SQLITE_SQL);
  std::string create_table (create_table_stream.str ());

  const int error_message_size (1024);
  char error_message[error_message_size+2];
  int error_flag (0);
  {
    char *sqlite_error_message = static_cast<char*>(sqlite3_malloc (error_message_size));
    if (error_message == nullptr)
      {
        throw std::runtime_error ("Unable to allocate error message when creating an sqlite3 database");
      }
    /// None of the rest of the lines in this block can throw, so 'db'
    /// will be cleaned up properly.
    sqlite3 *db;
    sqlite3_open (path.c_str (), &db);
    error_flag=sqlite3_exec (db, create_table.c_str (), nullptr, nullptr,
                             &sqlite_error_message);
    sqlite3_close (db);
    if (error_flag!=0)
      {
        strncpy(error_message, sqlite_error_message, error_message_size);
      }
    sqlite3_free (sqlite_error_message);
  }
  if (error_flag!=0)
    {
      throw std::runtime_error ("SQLite error: " + std::string (error_message));
    }
}
