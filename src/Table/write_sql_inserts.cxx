#include "../Data_Type_to_SQL.hxx"
#include "../Table.hxx"
#include "../quote_sql_string.hxx"
#include "../write_type_as_ascii.hxx"

namespace {
std::pair<size_t, tablator::Data_Type> get_offsets_and_types(
        const tablator::Table &table, const std::string &name) {
    auto column(table.find_column(name));
    if (column == table.columns.end()) {
        throw std::runtime_error("Unable to find the column '" + name +
                                 "' when creating geometries.");
    } else if (column->type == tablator::Data_Type::CHAR) {
        throw std::runtime_error(
                "Input columns to geography must be numeric.  The column '" + name +
                "' is text");
    } else if (column->array_size != 1) {
        throw std::runtime_error(
                "Input columns to geography must not be arrays.  The column '" + name +
                "' has array size=" + std::to_string(column->array_size));
    }
    return std::pair<size_t, tablator::Data_Type>(
            table.offsets[std::distance(table.columns.begin(), column)], column->type);
}

std::pair<std::pair<size_t, tablator::Data_Type>,
          std::pair<size_t, tablator::Data_Type>>
get_offsets_and_types(const tablator::Table &table,
                      const std::pair<std::string, std::string> &names) {
    return std::make_pair(get_offsets_and_types(table, names.first),
                          get_offsets_and_types(table, names.second));
}

}  // namespace

void tablator::Table::write_sql_inserts(
        std::ostream &os, const std::string &table_name,
        const std::pair<std::string, std::string> &point_input_names,
        const std::vector<std::pair<std::string, std::string>> &polygon_input_names)
        const {
    std::string quoted_table_name(
            quote_sql_string(table_name, '"', Quote_SQL::IF_NEEDED));
    std::pair<std::pair<size_t, Data_Type>, std::pair<size_t, Data_Type>> point_input;
    if (!point_input_names.first.empty()) {
        point_input = get_offsets_and_types(*this, point_input_names);
    }
    std::vector<std::pair<std::pair<size_t, Data_Type>, std::pair<size_t, Data_Type>>>
            polygon_input;
    for (auto &names : polygon_input_names) {
        polygon_input.emplace_back(get_offsets_and_types(*this, names));
    }

    for (size_t row_offset = 0; row_offset < data.size(); row_offset += row_size()) {
        write_sql_insert(os, quoted_table_name, row_offset,
                         !point_input_names.first.empty(), point_input, polygon_input);
    }
}
