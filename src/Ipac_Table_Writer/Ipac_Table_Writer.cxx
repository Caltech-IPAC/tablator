#include "../Ipac_Table_Writer.hxx"

#include "../Data_Type_Adjuster.hxx"
#include "../Table.hxx"

// This file contains (high-level) implementations of public functions of the
// Ipac_Table_Writer class.


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


// This function produces acceptable results for columns of type UINT64_LE even if
// their active_datatype is CHAR.

std::vector<size_t> tablator::Ipac_Table_Writer::get_column_widths(const Table &table) {
    std::vector<size_t> widths;
    auto &columns = table.columns;
    auto column(std::next(columns.begin()));
    // First column is the null bitfield flags, which are not written
    // out in ipac_tables.
    widths.push_back(0);
    for (; column != columns.end(); ++column) {
        size_t header_size(
                column->name.size() +
                (column->array_size == 1
                         ? 0
                         : 1 + std::to_string(column->array_size - 1).size()));
        auto unit = column->field_properties.attributes.find("unit");
        if (unit != column->field_properties.attributes.end()) {
            header_size = std::max(header_size, unit->second.size());
        }
        if (column->type == Data_Type::CHAR) {
            // The minimum of 4 is to accomodate the length of the
            // literals 'char' and 'null'.
            widths.push_back(
                    std::max((size_t)4, std::max(header_size, column->array_size)));
        } else {
            // buffer_size = 1 (sign) + 1 (leading digit) + 1
            // (decimal) + 1 (exponent sign) + 3 (exponent) (value
            // could be e.g. uint64 as well as double, but double's
            // buffer_size is generous enough).
            const size_t buffer_size(7);
            widths.push_back(
                    std::max(header_size,
                             std::numeric_limits<double>::max_digits10 + buffer_size));
        }
    }
    return widths;
}


/**********************************************************/
/* Wrappers for Data_Type_Adjuster-friendly analogues */
/**********************************************************/

void tablator::Ipac_Table_Writer::write_ipac_table(const tablator::Table &table,
                                                   std::ostream &os) {
    write_ipac_subtable_by_row(table, os, 0, table.num_rows(),
                               Data_Type_Adjuster(table).get_datatypes_for_writing(
                                       Format::Enums::IPAC_TABLE));
}

/**********************************************************/

void tablator::Ipac_Table_Writer::write_ipac_column_headers(const Table &table,
                                                            std::ostream &os) {
    Data_Type_Adjuster adjuster(table);
    const std::vector<Data_Type> datatypes_for_writing =
            adjuster.get_datatypes_for_writing(Format::Enums::IPAC_TABLE);
    tablator::Ipac_Table_Writer::write_ipac_column_headers(table, os,
                                                           datatypes_for_writing);
}

/**********************************************************/

void tablator::Ipac_Table_Writer::write_ipac_subtable_by_row(
        const Table &table, std::ostream &os, std::vector<size_t> requested_row_ids) {
    tablator::Ipac_Table_Writer::write_ipac_subtable_by_row(
            table, os, requested_row_ids,
            Data_Type_Adjuster(table).get_datatypes_for_writing(
                    Format::Enums::IPAC_TABLE));
}

/**********************************************************/

void tablator::Ipac_Table_Writer::write_ipac_subtable_by_row(const Table &table,
                                                             std::ostream &os,
                                                             size_t start_row,
                                                             size_t row_count) {
    tablator::Ipac_Table_Writer::write_ipac_subtable_by_row(
            table, os, start_row, row_count,
            Data_Type_Adjuster(table).get_datatypes_for_writing(
                    Format::Enums::IPAC_TABLE));
}

/**********************************************************/

void tablator::Ipac_Table_Writer::write_consecutive_ipac_records(const Table &table,
                                                                 std::ostream &os,
                                                                 size_t start_row,
                                                                 size_t num_requested) {
    Data_Type_Adjuster adjuster(table);
    const std::vector<Data_Type> datatypes_for_writing =
            adjuster.get_datatypes_for_writing(Format::Enums::IPAC_TABLE);

    tablator::Ipac_Table_Writer::write_consecutive_ipac_records(
            table, os, start_row, num_requested, datatypes_for_writing);
}

/**********************************************************/

void tablator::Ipac_Table_Writer::write_selected_ipac_records(
        const Table &table, std::ostream &os,
        std::vector<size_t> const &requested_row_ids) {
    Data_Type_Adjuster adjuster(table);
    const std::vector<Data_Type> datatypes_for_writing =
            adjuster.get_datatypes_for_writing(Format::Enums::IPAC_TABLE);

    tablator::Ipac_Table_Writer::write_selected_ipac_records(
            table, os, requested_row_ids, datatypes_for_writing);
}

/**********************************************************/

void tablator::Ipac_Table_Writer::write_single_ipac_record(const Table &table,
                                                           std::ostream &os,
                                                           size_t row_id) {
    Data_Type_Adjuster adjuster(table);
    const std::vector<Data_Type> datatypes_for_writing =
            adjuster.get_datatypes_for_writing(Format::Enums::IPAC_TABLE);

    size_t curr_row_offset = row_id * table.row_size();
    tablator::Ipac_Table_Writer::write_single_ipac_record_by_offset(
            table, os, curr_row_offset, datatypes_for_writing);
}


/*******************************************************/
/* write_ipac_table_header() and its helpers */
/*******************************************************/

namespace {
static constexpr size_t KEYWORD_ALIGNMENT = 8;

void write_keyword_header_line(std::ostream &os, const std::string &name,
                               const std::string &value) {
    os << "\\" << std::setw(KEYWORD_ALIGNMENT)
       << tablator::Ipac_Table_Writer::convert_newlines(name) << " = "
       << "'" << tablator::Ipac_Table_Writer::convert_newlines(value) << "'\n";
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

void write_comment_lines(std::ostream &os, const std::vector<std::string> &comments) {
    for (auto &c : comments) {
        std::stringstream ss(c);
        std::string line;
        while (std::getline(ss, line)) {
            os << "\\ " << line << "\n";
        }
    }
}

/*******************************************************/

void generate_and_write_default_comments(
        const tablator::Table &table, std::ostream &os,
        const std::vector<std::string> &json_comments) {
    auto columns = table.columns;
    auto comments = table.comments;
    for (size_t i = 1; i < columns.size(); ++i) {
        auto props = columns[i].field_properties;
        if (!props.attributes.empty() || !props.description.empty()) {
            std::string col_comment(columns[i].name);
            auto unit = props.attributes.find("unit");
            if (unit != props.attributes.end() && !unit->second.empty()) {
                col_comment.append(" (").append(unit->second).append(")");
            }

            if (find(comments.begin(), comments.end(), col_comment) != comments.end() ||
                find(json_comments.begin(), json_comments.end(), col_comment) !=
                        json_comments.end()) {
                continue;
            }

            os << "\\ " << col_comment << "\n";
            if (!props.description.empty())
                os << "\\ ___ " << props.description << "\n";
            // FIXME: Write out description attributes
        }
    }
}
}  // namespace


void tablator::Ipac_Table_Writer::write_ipac_table_header(const Table &table,
                                                          std::ostream &os,
                                                          int num_requested_rows) {
    static constexpr char const *FIXLEN_STRING = "fixlen = T";
    static constexpr char const *JSON_DESC_LABEL = "RESOURCE.TABLE.DESCRIPTION";


    os << "\\" << FIXLEN_STRING << "\n";
    os << std::left;

    size_t num_rows_to_report = (num_requested_rows == WRITE_ALL_ROWS)
                                        ? table.num_rows()
                                        : num_requested_rows;

    os << "\\" << tablator::Table::ROWS_RETRIEVED_KEYWORD << " = " << num_rows_to_report
       << "\n";

    auto &comments = table.comments;
    std::vector<std::string> json_comments;

    // Iterate through properties, distinguishing between keywords and json5-ified
    // comments. Write keywords here and save json_comments for a later loop. (Standard
    // refers to keywords rather than properties.)
    for (auto &name_and_property : table.properties) {
        auto &p = name_and_property.second;
        if (!p.value.empty()) {
            if (boost::equals(name_and_property.first, JSON_DESC_LABEL)) {
                // This is a comment (or concatenated comments) from the Json5 format.
                // Err on the side of respecting newlines rather than replacing with
                // spaces.
                store_json_comments(p.value, comments, json_comments);
            } else {
                write_keyword_header_line(os, name_and_property.first, p.value);
            }
        }
        auto &a = p.attributes;
        auto name(a.find("name")), value(a.find("value"));
        if (a.size() == 2 && name != a.end() && value != a.end()) {
            // We wrote these at the top of this function.
            if (boost::equals(name->second, tablator::Table::FIXLEN_KEYWORD) ||
                boost::equals(name->second, tablator::Table::ROWS_RETRIEVED_KEYWORD)) {
                continue;
            }
            write_keyword_header_line(os, name->second, value->second);
        } else {
            for (auto &attr : a) {
                write_keyword_header_line(
                        os, name_and_property.first + "." + attr.first, attr.second);
            }
        }
    }

    // Stream json_comments followed by comments.
    write_comment_lines(os, json_comments);
    write_comment_lines(os, comments);

    // Add default comments (column name with unit and description) if they aren't
    // present already.
    generate_and_write_default_comments(table, os, json_comments);
}
