#include "../Data_Type_to_SQL.hxx"
#include "../Table.hxx"
#include "../quote_sql_string.hxx"

#include <sqlite/connection.hpp>
#include <sqlite/execute.hpp>

void tablator::Table::write_sqlite_db(const boost::filesystem::path &path,
                                      const Command_Line_Options &options) const {
    // Remove file at that location, if any; else sqlite will error out.
    boost::filesystem::remove(path);

    sqlite::connection connection(path.native());

    const std::string table_name(
            quote_sql_string(path.stem().native(), '"', Quote_SQL::IF_NEEDED));

    std::stringstream sql_stream;

    write_sql_create_table(sql_stream, table_name, Format::Enums::SQLITE_SQL);
    sql_stream << ";";
    sqlite::execute(connection, sql_stream.str(), true);

    auto num_rows = get_num_rows();
    for (size_t row_idx = 0; row_idx < num_rows; ++row_idx) {
        sql_stream.str("");
        write_sql_insert(sql_stream, table_name, row_idx, options);
        sqlite::execute(connection, sql_stream.str(), true);
    }
}
