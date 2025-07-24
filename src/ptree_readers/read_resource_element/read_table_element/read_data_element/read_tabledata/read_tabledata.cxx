#include "../../../../../ptree_readers.hxx"

#include "../../../../../Common.hxx"
#include "../../../../../Data_Element.hxx"
#include "../../../../../Utils/Table_Utils.hxx"
#include "../../../../../to_string.hxx"

namespace tablator {
size_t count_elements(const std::string &entry, const Data_Type &type);
}
// JTODO std::cout
tablator::Data_Element tablator::ptree_readers::read_tabledata(
        const boost::property_tree::ptree &tabledata,
        const std::vector<Field> &fields) {
  // std::cout << "read_tabledata(), enter" << std::endl;
    std::vector<std::vector<std::string> > element_lists_by_row;
    size_t num_fields = fields.size();

    // Need to set the size to at least 1, because H5::StrType can not
    // handle zero sized strings.
    std::vector<size_t> column_array_sizes(num_fields, 1);
    const size_t null_flags_size((num_fields + 6) / 8);
    column_array_sizes.at(0) = null_flags_size;

    for (auto &tr : tabledata) {
			  // std::cout << "read_tabledata(), top of tr loop" << std::endl;
        if (tr.first == "TR" || tr.first.empty()) {
            // Add something for the null_bitfields_flag
            element_lists_by_row.push_back({});

            auto td = tr.second.begin();
            while (td != tr.second.end() && td->first == XMLATTR_DOT + ID) {
                ++td;
            }
            for (std::size_t c = 1; c < num_fields; ++c) {
                if (td == tr.second.end()) {
                    throw std::runtime_error(
                            "Not enough columns in row " +
                            std::to_string(element_lists_by_row.size()) +
                            ".  Expected " + std::to_string(num_fields - 1) +
                            ", but only got " + std::to_string(c - 1) + ".");
                }
                const auto &field = fields.at(c);
                if (td->first == "TD" || td->first.empty()) {
                    std::string temp = td->second.get_value<std::string>();
                    if (field.get_array_size() != 1) {
                        column_array_sizes[c] =
                                std::max(column_array_sizes[c],
                                         count_elements(temp, field.get_type()));
                    }
                    element_lists_by_row.rbegin()->emplace_back(temp);
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
                throw std::runtime_error("Too many elements in row " +
                                         std::to_string(element_lists_by_row.size()) +
                                         ".  Expected only " +
                                         std::to_string(num_fields - 1) + ".");
            }
        } else if (tr.first != XMLATTR_DOT + "encoding" && tr.first != XMLCOMMENT) {
            throw std::runtime_error(
                    "Expected TR inside RESOURCE.TABLE.DATA.TABLEDATA, but found: " +
                    tr.first);
        }
    }

    std::vector<Column> orig_columns;
	// std::cout << "read_tabledata(), num_fields: " << num_fields << std::endl;
    for (std::size_t c = 0; c < num_fields; ++c) {
        const auto &field = fields.at(c);
        orig_columns.emplace_back(field.get_name(), field.get_type(),
                                  column_array_sizes[c], field.get_field_properties(),
                                  field.get_dynamic_array_flag());
    }

    Field_Framework field_framework(orig_columns, true /* got_null_bitfields_column */);
    std::vector<Column> &columns = field_framework.get_columns();
    std::vector<size_t> &offsets = field_framework.get_offsets();

    size_t num_rows = element_lists_by_row.size();
    Data_Details data_details(field_framework, num_rows);

    size_t num_dynamic_columns = field_framework.get_num_dynamic_columns();
	Row single_row(field_framework.get_row_size(), num_dynamic_columns);
    // JTODO Are we allowing for non-CHAR dynamic arrays?  Should all arrays end in
    // '\0'?
		// std::cout << "read_tabledata(), before loop,  num_rows per elt list: " << num_rows << std::endl << std::flush;
    for (size_t row_idx = 0; row_idx < num_rows; ++row_idx) {
	  // std::cout << "read_tabledata(), top of loop, row_idx: " << row_idx << std::endl;
        auto &element_list = element_lists_by_row[row_idx];
		//		Row single_row(field_framework.get_row_size(), num_dynamic_columns);
        single_row.fill_with_zeros();
        for (size_t col_idx = 1; col_idx < num_fields; ++col_idx) {
            const auto &column = columns[col_idx];
            auto &element = element_list[col_idx - 1];
            if (element.empty()) {
			  // std::cout << "read_tabledata(), before insert_null" << std::endl;
                single_row.insert_null(column.get_type(), column.get_array_size(),
                                       col_idx, offsets[col_idx], offsets[col_idx + 1]);
            } else
                try {
			  // std::cout << "read_tabledata(), before insert_from_ascii" << std::endl;
                    single_row.insert_from_ascii(
                            element, column.get_type(), column.get_array_size(),
                            col_idx, offsets[col_idx], offsets[col_idx + 1],
                            field_framework.get_idx_in_dynamic_cols_list(col_idx));
                } catch (std::exception &error) {
                    throw std::runtime_error(
                            "Invalid " + to_string(fields[col_idx].get_type()) +
                            " value " + element + " in element_list " +
                            std::to_string(row_idx + 1) + ", field " +
                            std::to_string(col_idx) +
                            ", array_size: " + std::to_string(column.get_array_size()) +
                            ". Error message: " + error.what());
                }
        }
		// std::cout << "read_tabledata(), before append_row()" << std::endl;
        data_details.append_row(single_row);
		// std::cout << "read_tabledata(), bottom of loop, num_rows: " << data_details.get_num_rows() << std::endl << std::flush;
		// std::cout << "... row_size: " << data_details.get_data().back().size() << std::endl;
    }
	// std::cout << "read_tabledata(), exit, num_rows: " << data_details.get_num_rows() << std::endl << std::flush;
	// std::cout << "... row_size: " << data_details.get_row_size() << std::endl;
    return Data_Element(field_framework, data_details);
}
