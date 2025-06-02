#include "../../../../../ptree_readers.hxx"

#include "../../../../../Common.hxx"
#include "../../../../../Data_Element.hxx"
#include "../../../../../Utils/Table_Utils.hxx"
#include "../../../../../to_string.hxx"

namespace tablator {
size_t count_elements(const std::string &entry, const Data_Type &type);
}

tablator::Data_Element tablator::ptree_readers::read_tabledata(
        const boost::property_tree::ptree &tabledata,
        const std::vector<Field_And_Flag> &field_flag_pairs) {
    std::vector<std::vector<std::string> > rows;
    size_t num_fields = field_flag_pairs.size();

	std::vector<std::vector<uint32_t>> row_column_array_sizes(num_fields);
    // Need to set the size to at least 1, because H5::StrType can not
    // handle zero sized strings.
    std::vector<size_t> column_array_sizes(num_fields, 1);
    const size_t null_flags_size((num_fields + 6) / 8);
    column_array_sizes.at(0) = null_flags_size;
    for (auto &tr : tabledata) {
        if (tr.first == "TR" || tr.first.empty()) {
            // Add something for the null_bitfields_flag
            rows.push_back({});
            auto td = tr.second.begin();
            while (td != tr.second.end() && td->first == XMLATTR_DOT + ID) {
                ++td;
            }
            for (std::size_t c = 1; c < num_fields; ++c) {
                if (td == tr.second.end())
                    throw std::runtime_error(
                            "Not enough columns in row " + std::to_string(rows.size()) +
                            ".  Expected " + std::to_string(num_fields - 1) +
                            ", but only got " + std::to_string(c - 1) + ".");

                const auto &field = field_flag_pairs.at(c).get_field();
                if (td->first == "TD" || td->first.empty()) {
                    std::string temp = td->second.get_value<std::string>();
					// std::cout << "field.get_array_size(): " << field.get_array_size() << std::endl;
                    if (field.get_array_size() != 1) {
                        column_array_sizes[c] =
                                std::max(column_array_sizes[c],
                                         count_elements(temp, field.get_type()));
					}
						// std::cout << "updated array_size: " << column_array_sizes[c] << std::endl;
                    rows.rbegin()->emplace_back(temp);
                } else {
                    throw std::runtime_error(
                            "Expected TD inside RESOURCE.TABLE.DATA.TABLEDATA.TR, "
                            "but found: " +
                            td->first);
                }
                // FIXME: Check encoding
                ++td;
            }
            if (td != tr.second.end()) {
                throw std::runtime_error(
                        "read_tabldate(): Too many elements in row " + std::to_string(rows.size()) +
                        ".  Only expected " + std::to_string(num_fields - 1) + ".");
            }
        } else if (tr.first != XMLATTR_DOT + "encoding" && tr.first != XMLCOMMENT) {
            throw std::runtime_error(
                    "Expected TR inside RESOURCE.TABLE.DATA.TABLEDATA, but found: " +
                    tr.first);
        }
    }

    std::vector<Column> columns;
    std::vector<size_t> offsets = {0};
    std::vector<uint8_t> data;

    for (std::size_t c = 0; c < num_fields; ++c) {
        const auto &field = field_flag_pairs.at(c).get_field();
        append_column(columns, offsets, field.get_name(), field.get_type(),
                      column_array_sizes[c], field.get_field_properties(),
                      field_flag_pairs.at(c).get_dynamic_array_flag());
    }

    Row row_string(*offsets.rbegin());

	// JTODO Are we allowing for non-CHAR dynamic arrays?  Should all arrays end in '\0'?


    for (size_t current_row = 0; current_row < rows.size(); ++current_row) {
        auto &row(rows[current_row]);
        row_string.fill_with_zeros();
        for (size_t col_idx = 1; col_idx < num_fields; ++col_idx) {
			const auto &column = columns[col_idx];
            auto &element(row[col_idx - 1]);
            if (element.empty()) {
			  // JTODO dynamic?
                row_string.set_null(column.get_type(),
                                    column.get_array_size(), col_idx,
                                    offsets[col_idx], offsets[col_idx + 1]);
            } else
                try {
				  // std::cout << "read_tabledata(), before insert_ascii(), array_size: " << column.get_array_size() << std::endl;
                    insert_ascii_in_row(row_string, column.get_type(),
                                        column.get_array_size(), col_idx,
                                        element, offsets[col_idx], offsets[col_idx + 1], column.get_dynamic_array_flag());
                } catch (std::exception &error) {
                    throw std::runtime_error(
                            "Invalid " +
                            to_string(field_flag_pairs[col_idx].get_field().get_type()) +
                            " value " + element + " in row " +
                            std::to_string(current_row + 1) + ", field " +
                            std::to_string(col_idx) + ", array_size: " +
                            std::to_string(column.get_array_size()) +
                            ". Error message: " + error.what());
                }
        }
        append_row(data, row_string);
    }
    return Data_Element(columns, offsets, data);
}
