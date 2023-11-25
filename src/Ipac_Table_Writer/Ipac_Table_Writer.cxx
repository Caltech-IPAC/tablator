#include "../Ipac_Table_Writer.hxx"

#include <boost/lexical_cast.hpp>
#include <boost/range/algorithm/replace_copy_if.hpp>
#include <boost/range/algorithm/replace_if.hpp>

#include "../Ascii_Writer.hxx"
#include "../Data_Type_Adjuster.hxx"
#include "../Table.hxx"

// This file contains (high-level) implementations of public functions of the
// Ipac_Table_Writer class.

namespace {

size_t compute_max_column_width_for_double(const tablator::Table &table, size_t col_idx,
                                           size_t col_width_from_headers) {
    size_t max_width_sofar = col_width_from_headers;

    const std::vector<uint8_t> &table_data = table.get_data();
    size_t base_offset = table.get_offsets().at(col_idx);
    size_t curr_row_start_offset = 0;
    uint8_t const *curr_data = table_data.data() + base_offset;

    for (size_t row_idx = 0; row_idx < table.num_rows(); ++row_idx,
                curr_data += table.row_size(),
                curr_row_start_offset += table.row_size()) {
        if (table.is_null(curr_row_start_offset, col_idx)) {
            continue;
        }
        size_t curr_width =
                tablator::Ascii_Writer::get_adjusted_string_length_for_double(
                        *reinterpret_cast<const double *>(curr_data));
        max_width_sofar = std::max(max_width_sofar, curr_width);
    }
    return max_width_sofar;
}

  //====================================================

template <typename T>
size_t compute_max_column_width(const tablator::Table &table, size_t col_idx,
                                size_t col_width_from_headers) {
    size_t max_width_sofar = col_width_from_headers;

    const std::vector<uint8_t> &table_data = table.get_data();
    size_t base_offset = table.get_offsets().at(col_idx);
    size_t curr_row_start_offset = 0;
    uint8_t const *curr_data = table_data.data() + base_offset;

    for (size_t row_idx = 0; row_idx < table.num_rows(); ++row_idx,
                curr_data += table.row_size(),
                curr_row_start_offset += table.row_size()) {
        if (table.is_null(curr_row_start_offset, col_idx)) {
            continue;
        }
        T curr_val = *reinterpret_cast<const T *>(curr_data);
        std::string string_val = boost::lexical_cast<std::string>(curr_val);
        max_width_sofar = std::max(max_width_sofar, string_val.size());
    }
    return max_width_sofar;
}


}  // namespace


/**********************************************************/
/* Auxiliary functions exposed through Table */
/**********************************************************/

std::string tablator::Ipac_Table_Writer::to_ipac_string(const Data_Type &type) {
    /// Write out unsigned integers as integers for backwards compatibility
    switch (type) {
        case Data_Type::INT8_LE:
        case Data_Type::UINT8_LE:
        case Data_Type::INT16_LE:
        case Data_Type::UINT16_LE:
        case Data_Type::INT32_LE:
            return "int";
        /// Unsigned 32 bit ints do not fit in ints, so we use a long.
        case Data_Type::UINT32_LE:
        case Data_Type::INT64_LE:
        case Data_Type::UINT64_LE:
            return "long";
        case Data_Type::FLOAT32_LE:
            return "float";
        case Data_Type::FLOAT64_LE:
            return "double";
        case Data_Type::CHAR:
            return "char";
        default:
            throw std::runtime_error(
                    "Unexpected HDF5 data type in tablator::Table::to_ipac_string");
    }
}

/*******************************************************/


// This function determines the minimum possible width of each column,
// if necessary iterating through each column's values to determine
// their lengths in ascii format.

// This function produces acceptable results for columns of type UINT64_LE even if
// their active_datatype is CHAR.

std::vector<size_t> tablator::Ipac_Table_Writer::get_column_widths(const Table &table) {
    std::vector<size_t> widths;

    // Allow for '-' sign.
    static size_t MAX_INT8_STRLEN = 4;
    static size_t MAX_INT16_STRLEN = 6;

    const auto &columns = table.get_columns();

    // The first column is the null bitfield flags, which is not
    // written out in ipac_tables.
    widths.push_back(0);

    for (size_t col_idx = 1; col_idx < columns.size(); ++col_idx) {
        // Build lower bound for column width based on the 4 lines of
        // the header: name, unit, type, and null.
        const auto column = columns.at(col_idx);
        size_t col_width_from_headers(
                column.get_name().size() +
                (column.get_array_size() == 1
                         ? 0
                         : 1 + std::to_string(column.get_array_size() - 1).size()));

        const auto &field_prop_attributes =
                column.get_field_properties().get_attributes();
        const auto unit = field_prop_attributes.find("unit");
        if (unit != field_prop_attributes.end()) {
            col_width_from_headers =
                    std::max(col_width_from_headers, unit->second.size());
        }

        auto type = column.get_type();
        col_width_from_headers =
                std::max(col_width_from_headers, to_ipac_string(type).size());

        const auto &null_value = column.get_field_properties().get_values().null;
        const std::string &null_str =
                (null_value.empty()) ? tablator::Table::DEFAULT_NULL_VALUE : null_value;
        col_width_from_headers = std::max(col_width_from_headers, null_str.size());

        switch (type) {
            case Data_Type::INT8_LE:
            case Data_Type::UINT8_LE: {
                // std::cout << "INT8" << std::endl;
                widths.push_back(std::max(col_width_from_headers, MAX_INT8_STRLEN));
            } break;
            case Data_Type::INT16_LE: {
                // std::cout << "INT16" << std::endl;
                widths.push_back(std::max(col_width_from_headers, MAX_INT16_STRLEN));
            } break;
            case Data_Type::UINT16_LE: {
                // std::cout << "UINT16" << std::endl;
                widths.push_back(compute_max_column_width<uint16_t>(
                        table, col_idx, col_width_from_headers));
            } break;
            case Data_Type::INT32_LE: {
                // std::cout << "INT32" << std::endl;
                widths.push_back(compute_max_column_width<int32_t>(
                        table, col_idx, col_width_from_headers));
            } break;
            case Data_Type::UINT32_LE: {
                // std::cout << "UINT32" << std::endl;
                widths.push_back(compute_max_column_width<uint32_t>(
                        table, col_idx, col_width_from_headers));
            } break;
            case Data_Type::INT64_LE: {
                widths.push_back(compute_max_column_width<int64_t>(
                        table, col_idx, col_width_from_headers));
            } break;
            case Data_Type::UINT64_LE: {
                widths.push_back(compute_max_column_width<uint64_t>(
                        table, col_idx, col_width_from_headers));
            } break;
            case Data_Type::FLOAT32_LE: {
                widths.push_back(compute_max_column_width<float>(
                        table, col_idx, col_width_from_headers));
            } break;
            case Data_Type::FLOAT64_LE: {
                widths.push_back(compute_max_column_width_for_double(
                        table, col_idx, col_width_from_headers));
            } break;
            case Data_Type::CHAR: {
                widths.push_back(
                        std::max(col_width_from_headers, column.get_array_size()));
            } break;
        }
    }
    return widths;
}


/**********************************************************/
/* Wrappers for Data_Type_Adjuster-friendly analogues */
/**********************************************************/

void tablator::Ipac_Table_Writer::write(const tablator::Table &table,
                                        std::ostream &os) {
    write_subtable_by_row(table, os, 0, table.num_rows(), get_column_widths(table),
                          Data_Type_Adjuster(table).get_datatypes_for_writing(
                                  Format::Enums::IPAC_TABLE));
}

/**********************************************************/

void tablator::Ipac_Table_Writer::write_column_headers(const Table &table,
                                                       std::ostream &os) {
    tablator::Ipac_Table_Writer::write_column_headers(
            table, os, get_column_widths(table),
            Data_Type_Adjuster(table).get_datatypes_for_writing(
                    Format::Enums::IPAC_TABLE));
}


/**********************************************************/

void tablator::Ipac_Table_Writer::write_subtable_by_row(
        const Table &table, std::ostream &os,
        const std::vector<size_t> &requested_row_ids, bool skip_headers) {
    tablator::Ipac_Table_Writer::write_subtable_by_row(
            table, os, requested_row_ids, get_column_widths(table),
            Data_Type_Adjuster(table).get_datatypes_for_writing(
                    Format::Enums::IPAC_TABLE),
            skip_headers);
}

/**********************************************************/

void tablator::Ipac_Table_Writer::write_subtable_by_column_and_row(
        const Table &table, std::ostream &os, const std::vector<size_t> &column_ids,
        const std::vector<size_t> &requested_row_ids, bool skip_headers) {
    tablator::Ipac_Table_Writer::write_subtable_by_column_and_row(
            table, os, column_ids, requested_row_ids, get_column_widths(table),
            Data_Type_Adjuster(table).get_datatypes_for_writing(
                    Format::Enums::IPAC_TABLE),
            skip_headers);
}

/**********************************************************/

void tablator::Ipac_Table_Writer::write_subtable_by_row(const Table &table,
                                                        std::ostream &os,
                                                        size_t start_row,
                                                        size_t row_count,
                                                        bool skip_headers) {
    tablator::Ipac_Table_Writer::write_subtable_by_row(
            table, os, start_row, row_count, get_column_widths(table),
            Data_Type_Adjuster(table).get_datatypes_for_writing(
                    Format::Enums::IPAC_TABLE),
            skip_headers);
}

/**********************************************************/

void tablator::Ipac_Table_Writer::write_subtable_by_column_and_row(
        const Table &table, std::ostream &os, const std::vector<size_t> &column_ids,
        size_t start_row, size_t row_count, bool skip_headers) {
    tablator::Ipac_Table_Writer::write_subtable_by_column_and_row(
            table, os, column_ids, start_row, row_count, get_column_widths(table),
            Data_Type_Adjuster(table).get_datatypes_for_writing(
                    Format::Enums::IPAC_TABLE),
            skip_headers);
}

/**********************************************************/

void tablator::Ipac_Table_Writer::write_subtable_by_column_and_row(
        const Table &table, std::ostream &os, const std::vector<size_t> &column_ids,
        bool skip_headers) {
    write_subtable_by_column_and_row(table, os, column_ids, 0, table.num_rows(),
                                     skip_headers);
}

/**********************************************************/

void tablator::Ipac_Table_Writer::write_single_record(const Table &table,
                                                      std::ostream &os, size_t row_id) {
    size_t curr_row_offset = row_id * table.row_size();

    tablator::Ipac_Table_Writer::write_single_record_by_offset(
            table, os, curr_row_offset, get_column_widths(table),
            Data_Type_Adjuster(table).get_datatypes_for_writing(
                    Format::Enums::IPAC_TABLE));
}

/**********************************************************/

void tablator::Ipac_Table_Writer::write_single_record(
        const Table &table, std::ostream &os, const std::vector<size_t> &column_ids,
        size_t row_id) {
    size_t curr_row_offset = row_id * table.row_size();
    tablator::Ipac_Table_Writer::write_single_record_by_offset(
            table, os, column_ids, curr_row_offset, get_column_widths(table),
            Data_Type_Adjuster(table).get_datatypes_for_writing(
                    Format::Enums::IPAC_TABLE));
}

/**********************************************************/

void tablator::Ipac_Table_Writer::write_consecutive_records(const Table &table,
                                                            std::ostream &os,
                                                            size_t start_row,
                                                            size_t num_requested) {
    tablator::Ipac_Table_Writer::write_consecutive_records(
            table, os, start_row, num_requested, get_column_widths(table),
            Data_Type_Adjuster(table).get_datatypes_for_writing(
                    Format::Enums::IPAC_TABLE));
}

/**********************************************************/

void tablator::Ipac_Table_Writer::write_consecutive_records(
        const Table &table, std::ostream &os, const std::vector<size_t> &column_ids,
        size_t start_row, size_t num_requested) {
    tablator::Ipac_Table_Writer::write_consecutive_records(
            table, os, column_ids, start_row, num_requested, get_column_widths(table),
            Data_Type_Adjuster(table).get_datatypes_for_writing(
                    Format::Enums::IPAC_TABLE));
}

/**********************************************************/

void tablator::Ipac_Table_Writer::write_selected_records(
        const Table &table, std::ostream &os,
        const std::vector<size_t> &requested_row_ids) {
    tablator::Ipac_Table_Writer::write_selected_records(
            table, os, requested_row_ids, get_column_widths(table),
            Data_Type_Adjuster(table).get_datatypes_for_writing(
                    Format::Enums::IPAC_TABLE));
}

/**********************************************************/

void tablator::Ipac_Table_Writer::write_selected_records(
        const Table &table, std::ostream &os, const std::vector<size_t> &column_ids,
        const std::vector<size_t> &requested_row_ids) {
    tablator::Ipac_Table_Writer::write_selected_records(
            table, os, column_ids, requested_row_ids, get_column_widths(table),
            Data_Type_Adjuster(table).get_datatypes_for_writing(
                    Format::Enums::IPAC_TABLE));
}


/*******************************************************/
/* write_header() and its helpers */
/*******************************************************/

namespace {
static constexpr size_t KEYWORD_ALIGNMENT = 8;

void write_keyword_header_line(std::ostream &os, const std::string &name,
                               const std::string &value) {
    os << "\\" << std::setw(KEYWORD_ALIGNMENT);
    std::ostreambuf_iterator<char> out_iter(os);
    boost::replace_copy_if(name, out_iter, boost::is_any_of(tablator::NEWLINES), ' ');
    os << std::setw(0) << " = '";
    boost::replace_copy_if(value, out_iter, boost::is_any_of(tablator::NEWLINES), ' ');
    os << "'\n";
}

/*******************************************************/

void store_json_comments(const std::string &value,
                         const std::vector<std::string> &comments,
                         std::vector<std::string> &json_comments) {
    std::string val(value);
    boost::algorithm::trim(val);
    std::vector<std::string> elements;
    boost::split(elements, val, boost::is_any_of("\n"));
    for (const std::string &elt : elements) {
        if (find(comments.begin(), comments.end(), elt) == comments.end()) {
            json_comments.emplace_back(elt);
        }
    }
}

/*******************************************************/

void write_comment_line_with_newlines(std::ostream &os, const std::string &comment) {
    std::stringstream ss(comment);
    std::string line;
    while (std::getline(ss, line)) {
        os << "\\ " << line << "\n";
    }
}

/*******************************************************/

void write_comment_lines(std::ostream &os, const std::vector<std::string> &comments) {
    for (auto &c : comments) {
        write_comment_line_with_newlines(os, c);
    }
}

/*******************************************************/

void generate_and_write_default_comments(
        const tablator::Table &table, std::ostream &os,
        const std::vector<size_t> &included_column_ids,
        const std::vector<std::string> &json_comments) {
    const auto &columns = table.get_columns();
    const auto &comments = table.get_comments();
    const auto &results_resource_element = table.get_results_resource_element();

    const auto &table_element_description =
            results_resource_element.get_main_table_element().get_description();

    std::vector<std::string> description_vec;
    boost::split(description_vec, table_element_description, boost::is_any_of("\n"));

    for (size_t col_id : included_column_ids) {
        if (tablator::Ipac_Table_Writer::is_valid_col_id(col_id, columns.size())) {
            const auto &field_props = columns[col_id].get_field_properties();
            const auto field_prop_attributes = field_props.get_attributes();
            if (!field_prop_attributes.empty() ||
                !field_props.get_description().empty()) {
                std::string col_comment(columns[col_id].get_name());
                const auto unit = field_prop_attributes.find(tablator::UNIT);
                if (unit != field_prop_attributes.end() && !unit->second.empty()) {
                    col_comment.append(" (").append(unit->second).append(")");
                    boost::replace_if(col_comment, boost::is_any_of(tablator::NEWLINES),
                                      ' ');
                }

                if (find(comments.begin(), comments.end(), col_comment) !=
                            comments.end() ||
                    find(json_comments.begin(), json_comments.end(), col_comment) !=
                            json_comments.end() ||
                    find(description_vec.begin(), description_vec.end(), col_comment) !=
                            description_vec.end()) {
                    continue;
                }

                os << "\\ " << col_comment << "\n";
                const auto &desc = field_props.get_description();
                if (!desc.empty()) {
                    size_t start = desc.find_first_not_of(tablator::WHITESPACE);
                    if (start != std::string::npos) {
                        std::ostream_iterator<char> out_iter(os);
                        os << "\\ ___ ";
                        boost::replace_copy_if(desc.substr(start), out_iter,
                                               boost::is_any_of(tablator::NEWLINES),
                                               ' ');
                        os << "\n";
                    }
                }
                // FIXME: Write out description attributes
            }
        }
    }
}
}  // namespace


void tablator::Ipac_Table_Writer::write_header(const Table &table, std::ostream &os) {
    write_header(table, os, get_all_nonzero_col_ids(table.get_columns().size()),
                 table.num_rows());
}


void tablator::Ipac_Table_Writer::write_header(const Table &table, std::ostream &os,
                                               size_t num_requested_rows) {
    write_header(table, os, get_all_nonzero_col_ids(table.get_columns().size()),
                 num_requested_rows);
}


void tablator::Ipac_Table_Writer::write_header(
        const Table &table, std::ostream &os,
        const std::vector<size_t> &included_column_ids) {
    write_header(table, os, included_column_ids, table.num_rows());
}


void tablator::Ipac_Table_Writer::write_header(
        const Table &table, std::ostream &os,
        const std::vector<size_t> &included_column_ids, size_t num_requested_rows) {
    static constexpr char const *FIXLEN_STRING = "fixlen = T";
    static constexpr char const *JSON_DESC_LABEL = "RESOURCE.TABLE.DESCRIPTION";

    os << "\\" << FIXLEN_STRING << "\n";
    os << std::left;

    os << "\\" << tablator::Table::ROWS_RETRIEVED_KEYWORD << " = " << num_requested_rows
       << "\n";

    const auto &results_resource_element = table.get_results_resource_element();
    auto &resource_attributes = results_resource_element.get_attributes();
    for (auto &attr_pair : resource_attributes) {
        write_keyword_header_line(os, attr_pair.first, attr_pair.second);
    }

    // Iterate through resource-level properties.
    auto &labeled_resource_element_properties =
            results_resource_element.get_labeled_properties();
    for (auto &name_and_property : labeled_resource_element_properties) {
        const auto &label = name_and_property.first;
        auto &prop = name_and_property.second;
        const auto &prop_attributes = prop.get_attributes();

        const auto name_iter = prop_attributes.find(ATTR_NAME);
        const auto value_iter = prop_attributes.find(ATTR_VALUE);

        if (boost::equals(label, INFO) && prop_attributes.size() == 2 &&
            (name_iter != prop_attributes.end()) &&
            value_iter != prop_attributes.end()) {
            // e.g. if we converted from IPAC format to VOTable and are now converting
            // back
            write_keyword_header_line(os, name_iter->second, value_iter->second);
        } else if (!prop_attributes.empty()) {
            for (const auto &attr_pair : prop_attributes) {
                write_keyword_header_line(os, attr_pair.first, attr_pair.second);
            }
        }

        if (!label.empty() && !prop.get_value().empty()) {
            write_keyword_header_line(os, label, prop.get_value());
        } else if (!prop.get_value().empty()) {
            write_keyword_header_line(os, ATTR_VALUE, prop.get_value());
        }
    }

    const auto &comments = table.get_comments();
    std::vector<std::string> json_comments;

    // Iterate through top-level properties, distinguishing between keywords and
    // json5-ified comments. Write keywords here and save json_comments for a later
    // loop.
    // JTODO What about params?
    for (auto &name_and_property : table.get_labeled_properties()) {
        auto &prop = name_and_property.second;
        if (!prop.get_value().empty()) {
            if (boost::equals(name_and_property.first, JSON_DESC_LABEL)) {
                // This is a comment (or concatenated comments) from the Json5 format.
                // Err on the side of respecting newlines rather than replacing with
                // spaces.
                store_json_comments(prop.get_value(), comments, json_comments);
            } else {
                write_keyword_header_line(os, name_and_property.first,
                                          prop.get_value());
            }
        }
        auto &attrs = prop.get_attributes();
        auto name(attrs.find(ATTR_NAME)), value(attrs.find(ATTR_VALUE));
        if (attrs.size() == 2 && name != attrs.end() && value != attrs.end()) {
            // We wrote these at the top of this function.
            if (boost::equals(name->second, tablator::Table::FIXLEN_KEYWORD) ||
                boost::equals(name->second, tablator::Table::ROWS_RETRIEVED_KEYWORD)) {
                continue;
            }
            write_keyword_header_line(os, name->second, value->second);
        } else {
            for (auto &attr : attrs) {
                write_keyword_header_line(
                        os, name_and_property.first + "." + attr.first, attr.second);
            }
        }
    }

    // Stream json_comments followed by comments.
    if (!table.get_description().empty()) {
        write_comment_line_with_newlines(os, table.get_description());
    }
    write_comment_lines(os, comments);
    const auto table_element_description =
            results_resource_element.get_main_table_element().get_description();

    if (!table_element_description.empty()) {
        write_comment_line_with_newlines(os, table_element_description);
    }

    // Add default comments (column name with unit and description) if they aren't
    // present already.
    generate_and_write_default_comments(table, os, included_column_ids, json_comments);
}
