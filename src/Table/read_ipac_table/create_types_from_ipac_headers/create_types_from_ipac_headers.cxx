#include "../../../Table.hxx"

namespace {
std::vector<size_t> get_ipac_column_widths(
        const std::vector<size_t> &ipac_column_offsets) {
    const size_t num_columns = ipac_column_offsets.size() - 1;
    std::vector<size_t> ipac_column_widths;
    /// Add a column for null flags.
    ipac_column_widths.push_back((num_columns + 7) / 8);
    for (size_t i = 0; i < num_columns; ++i)
        ipac_column_widths.push_back(ipac_column_offsets[i + 1] -
                                     ipac_column_offsets[i] - 1);
    return ipac_column_widths;
}
}  // namespace

void tablator::Table::create_types_from_ipac_headers(
        std::array<std::vector<std::string>, 4> &ipac_columns,
        const std::vector<size_t> &ipac_column_offsets,
        std::vector<size_t> &ipac_column_widths) {
    ipac_column_widths = get_ipac_column_widths(ipac_column_offsets);
    const size_t num_columns = ipac_columns[0].size();

    append_column(ipac_columns.at(0).at(0), Data_Type::UINT8_LE,
                  ipac_column_widths.at(0),
                  Field_Properties(null_bitfield_flags_description));
    for (size_t i = 1; i < num_columns; ++i) {
        append_ipac_data_member(ipac_columns.at(0).at(i), ipac_columns.at(1).at(i),
                                ipac_column_widths.at(i));
    }

    for (size_t column = 1; column < num_columns; ++column) {
        Field_Properties p({});
        if (!ipac_columns[2].at(column).empty()) {
            p.get_attributes().insert(std::make_pair(
                    "unit", boost::algorithm::trim_copy(ipac_columns[2].at(column))));
        }
        if (!ipac_columns[3].at(column).empty()) {
            p.get_values().null =
                    boost::algorithm::trim_copy(ipac_columns[3].at(column));
        }
        get_columns().at(column).set_field_properties(p);
    }
}
