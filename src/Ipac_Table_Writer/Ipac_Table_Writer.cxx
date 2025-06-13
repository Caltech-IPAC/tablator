#include "../Ipac_Table_Writer.hxx"

#include <set>

#include <boost/lexical_cast.hpp>
#include <boost/range/algorithm/replace_copy_if.hpp>
#include <boost/range/algorithm/replace_if.hpp>

#include "../Ascii_Writer.hxx"
#include "../Data_Type_Adjuster.hxx"
#include "../Decimal_String_Trimmer.hxx"
#include "../Table.hxx"


// This file contains high-level implementations of public functions of the
// Ipac_Table_Writer class.

namespace {

const std::vector<size_t> get_all_nonzero_col_ids(const tablator::Table &table) {
    std::vector<size_t> all_col_ids(table.get_columns().size() - 1);
    std::iota(all_col_ids.begin(), all_col_ids.end(), 1);
    return all_col_ids;
}

const std::vector<size_t> get_all_row_ids(const tablator::Table &table) {
    std::vector<size_t> all_row_ids(table.get_num_rows());
    std::iota(all_row_ids.begin(), all_row_ids.end(), 0);
    return all_row_ids;
}

//====================================================

// This templatized function is called for types other than CHAR and
// FLOAT64_LE, each of which has its own dedicated function (below).

template <typename T>
size_t compute_max_column_width_for_type(const tablator::Table &table,
                                         const std::vector<size_t> &requested_row_ids,
                                         size_t col_idx,
                                         uint8_t const *col_data_start_ptr,
                                         size_t col_width_from_headers,
                                         uint max_width_for_type, uint array_size) {
    if (col_width_from_headers >= max_width_for_type) {
        return col_width_from_headers;
    }
    size_t max_width_sofar = col_width_from_headers;

    for (size_t requested_row_id : requested_row_ids) {
        if (table.is_null_value(requested_row_id, col_idx)) {
            // We've already accounted for the width of the null value.
            continue;
        }

        size_t curr_row_start_offset = requested_row_id * table.get_row_size();
        uint8_t const *curr_col_data_ptr = col_data_start_ptr + curr_row_start_offset;

        const T *curr_array_elt_data_ptr =
                reinterpret_cast<const T *>(curr_col_data_ptr);

        for (uint j = 0; j < array_size && max_width_sofar < max_width_for_type; ++j) {
            T curr_array_elt = *curr_array_elt_data_ptr;
            if (curr_array_elt == tablator::get_null<T>()) {
                continue;
            }
            std::string string_val = boost::lexical_cast<std::string>(curr_array_elt);
            max_width_sofar = std::max(max_width_sofar, string_val.size());

            if (max_width_sofar >= max_width_for_type) {
                break;
            }
            ++curr_array_elt_data_ptr;
        }
    }
    return max_width_sofar;
}

//====================================================

size_t compute_max_column_width_for_double(const tablator::Table &table,
                                           const std::vector<size_t> &requested_row_ids,
                                           size_t col_idx,
                                           uint8_t const *col_data_start_ptr,
                                           size_t col_width_from_headers,
                                           uint min_run_length_for_trimming,
                                           uint max_width_for_double, uint array_size) {
    if (col_width_from_headers >= max_width_for_double) {
        return col_width_from_headers;
    }
    size_t max_width_sofar = col_width_from_headers;

    for (size_t requested_row_id : requested_row_ids) {
        if (table.is_null_value(requested_row_id, col_idx)) {
            // We've already accounted for the width of the null value.
            continue;
        }

        size_t curr_row_start_offset = requested_row_id * table.get_row_size();
        uint8_t const *curr_col_data_ptr = col_data_start_ptr + curr_row_start_offset;

        const double *curr_array_elt_data_ptr =
                reinterpret_cast<const double *>(curr_col_data_ptr);

        for (uint j = 0; j < array_size && max_width_sofar < max_width_for_double;
             ++j) {
            double curr_array_elt = *curr_array_elt_data_ptr;
            if (curr_array_elt == tablator::get_null<double>()) {
                continue;
            }
            size_t curr_width =
                    tablator::Decimal_String_Trimmer::get_decimal_string_length(
                            curr_array_elt, min_run_length_for_trimming);
            max_width_sofar = std::max(max_width_sofar, curr_width);

            if (max_width_sofar >= max_width_for_double) {
                return max_width_sofar;
            }
            ++curr_array_elt_data_ptr;
        }
    }
    return max_width_sofar;
}

//====================================================

size_t compute_max_column_width_for_char(const tablator::Table &table,
                                         const std::vector<size_t> &requested_row_ids,
                                         size_t col_idx,
                                         uint8_t const *col_data_start_ptr,
                                         size_t col_width_from_headers,
                                         size_t array_size, bool dynamic_array_flag) {
    size_t max_width_sofar = col_width_from_headers;

    for (size_t requested_row_id : requested_row_ids) {
        if (table.is_null_value(requested_row_id, col_idx)) {
            continue;
        }
        size_t curr_array_size = array_size;
        size_t curr_row_start_offset = requested_row_id * table.get_row_size();
        uint8_t const *curr_col_data_ptr = col_data_start_ptr + curr_row_start_offset;
        if (dynamic_array_flag) {
            curr_array_size = *(reinterpret_cast<const uint32_t *>(curr_col_data_ptr));
            curr_col_data_ptr += sizeof(uint32_t);
            if (curr_array_size > array_size) {
                // JTODO check this when reading.
                throw std::runtime_error(
                        "Dynamic array size must not be larger than "
                        "column.array_size.");
            }
        }
        // JTODO skip the strlen?  Not meaningful?
        size_t curr_width =
                std::min(strlen(reinterpret_cast<const char *>(curr_col_data_ptr)),
                         curr_array_size);
        max_width_sofar = std::max(max_width_sofar, curr_width);
    }
    return max_width_sofar;
}

//====================================================

}  // namespace


/**********************************************************/
/* Auxiliary functions exposed through Table */
/**********************************************************/

std::string tablator::Ipac_Table_Writer::to_ipac_string(const Data_Type &type) {
    // Write out unsigned integers as integers for backwards compatibility
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

size_t tablator::Ipac_Table_Writer::get_single_column_width(
        const Table &table, const std::vector<size_t> &requested_row_ids,
        size_t col_idx, const Command_Line_Options &options) {
    // The first column is the null bitfield flags, which is not
    // written out in ipac_tables.
    if (col_idx == 0) {
        return 0;
    }

    // Allow for '-' sign.
    static size_t MAX_INT8_STRLEN = ceil(log10(INT8_MAX)) + 1;
    static size_t MAX_UINT8_STRLEN = ceil(log10(UINT8_MAX));
    static size_t MAX_INT16_STRLEN = ceil(log10(INT16_MAX)) + 1;
    static size_t MAX_UINT16_STRLEN = ceil(log10(UINT16_MAX));

    static size_t MAX_INT32_STRLEN = ceil(log10(INT32_MAX)) + 1;
    static size_t MAX_UINT32_STRLEN = ceil(log10(UINT32_MAX));
    static size_t MAX_INT64_STRLEN = ceil(log10(INT64_MAX)) + 1;
    static size_t MAX_UINT64_STRLEN = ceil(log10(UINT64_MAX));

    // https://stackoverflow.com/questions/2151302/counting-digits-in-a-float
    static size_t MAX_FLOAT32_STRLEN = 15;
    static size_t MAX_FLOAT64_STRLEN = 24;

    auto column = table.get_columns().at(col_idx);

    // Build lower bound for column width based on the 4 lines of
    // the header: name, unit, type, and null.

    auto type = column.get_type();
    uint array_size = column.get_array_size();
    bool got_array = (array_size > 1);
    size_t max_width_sofar = to_ipac_string(type).size();

    size_t width_from_name = column.get_name().size();
    if (got_array && type != Data_Type::CHAR) {
        // Such columns are represented in IPAC Table format as a
        // sequence of columns of array_size 1 whose names are
        // column_name_0, column_name_1, ... .  We'll use the same
        // width for all of these columns, the smallest width that
        // works for all of them.
        width_from_name += (1 + std::to_string(array_size - 1).size());
    }

    max_width_sofar = std::max(max_width_sofar, width_from_name);
    const auto &field_prop_attributes = column.get_field_properties().get_attributes();
    const auto unit = field_prop_attributes.find("unit");
    if (unit != field_prop_attributes.end()) {
        max_width_sofar = std::max(max_width_sofar, unit->second.size());
    }

    const auto &null_value = column.get_field_properties().get_values().null;
    const std::string &null_str =
            (null_value.empty()) ? tablator::Table::DEFAULT_NULL_VALUE : null_value;
    max_width_sofar = std::max(max_width_sofar, null_str.size());

    const std::vector<uint8_t> &table_data = table.get_data();
    size_t col_offset = table.get_offsets().at(col_idx);
    uint8_t const *data_start_ptr = table_data.data();
    uint8_t const *col_data_start_ptr = data_start_ptr + col_offset;

    if ((type != Data_Type::CHAR) && column.get_dynamic_array_flag()) {
        // JTODO
        // The flag might be set, but variable-length arrays are supported only for CHAR
        // columns in ipac_table format. If there are in fact arrays of different
        // lengths in a column of non-CHAR type), an error will be thrown elsewhere.
        // JTODO. For now, just skip the dynamic_array_size value.
        col_data_start_ptr += sizeof(uint32_t);
    }

    bool trim_decimal_runs_f = options.is_trim_decimal_runs();
    short min_run_length_for_trim = options.min_run_length_for_trim_;

    switch (type) {
        case Data_Type::INT8_LE: {
            // std::cout << "INT8" << std::endl;
            max_width_sofar = compute_max_column_width_for_type<int8_t>(
                    table, requested_row_ids, col_idx, col_data_start_ptr,
                    max_width_sofar, MAX_INT8_STRLEN, array_size);
        } break;
        case Data_Type::UINT8_LE: {
            // std::cout << "UINT8" << std::endl;
            max_width_sofar = compute_max_column_width_for_type<uint8_t>(
                    table, requested_row_ids, col_idx, col_data_start_ptr,
                    max_width_sofar, MAX_UINT8_STRLEN, array_size);
        } break;
        case Data_Type::INT16_LE: {
            // std::cout << "INT16" << std::endl;
            max_width_sofar = compute_max_column_width_for_type<int16_t>(
                    table, requested_row_ids, col_idx, col_data_start_ptr,
                    max_width_sofar, MAX_INT16_STRLEN, array_size);
        } break;
        case Data_Type::UINT16_LE: {
            // std::cout << "UINT16" << std::endl;
            max_width_sofar = compute_max_column_width_for_type<uint16_t>(
                    table, requested_row_ids, col_idx, col_data_start_ptr,
                    max_width_sofar, MAX_UINT16_STRLEN, array_size);
        } break;
        case Data_Type::INT32_LE: {
            // std::cout << "INT32" << std::endl;
            max_width_sofar = compute_max_column_width_for_type<int32_t>(
                    table, requested_row_ids, col_idx, col_data_start_ptr,
                    max_width_sofar, MAX_INT32_STRLEN, array_size);
        } break;
        case Data_Type::UINT32_LE: {
            // std::cout << "UINT32" << std::endl;
            max_width_sofar = compute_max_column_width_for_type<uint32_t>(
                    table, requested_row_ids, col_idx, col_data_start_ptr,
                    max_width_sofar, MAX_UINT32_STRLEN, array_size);
        } break;
        case Data_Type::INT64_LE: {
            max_width_sofar = compute_max_column_width_for_type<int64_t>(
                    table, requested_row_ids, col_idx, col_data_start_ptr,
                    max_width_sofar, MAX_INT64_STRLEN, array_size);
        } break;
        case Data_Type::UINT64_LE: {
            max_width_sofar = compute_max_column_width_for_type<uint64_t>(
                    table, requested_row_ids, col_idx, col_data_start_ptr,
                    max_width_sofar, MAX_UINT64_STRLEN, array_size);
        } break;
        case Data_Type::FLOAT32_LE: {
            // std::cout << "FLOAT32" << std::endl;
            max_width_sofar = compute_max_column_width_for_type<float>(
                    table, requested_row_ids, col_idx, col_data_start_ptr,
                    max_width_sofar, MAX_FLOAT32_STRLEN, array_size);
        } break;
        case Data_Type::FLOAT64_LE: {
            // std::cout << "FLOAT64" << std::endl;
            if (trim_decimal_runs_f) {
                max_width_sofar = compute_max_column_width_for_double(
                        table, requested_row_ids, col_idx, col_data_start_ptr,
                        max_width_sofar, min_run_length_for_trim, MAX_FLOAT64_STRLEN,
                        array_size);
            } else {
                max_width_sofar = compute_max_column_width_for_type<double>(
                        table, requested_row_ids, col_idx, col_data_start_ptr,
                        max_width_sofar, MAX_FLOAT64_STRLEN, array_size);
            }
        } break;
        case Data_Type::CHAR: {
            max_width_sofar = compute_max_column_width_for_char(
                    table, requested_row_ids, col_idx, col_data_start_ptr,
                    max_width_sofar, array_size, column.get_dynamic_array_flag());
        } break;
    }
    return max_width_sofar;
}

/*******************************************************/

// This function determines, for each column of the specified table,
// the number of chars to allocate to that column in the table
// returned to the caller, if necessary iterating through the column's
// values to determine their lengths in ascii format.

// Each column of the specified table has a corresponding entry in
// this function's return value (the vector ipac_column_widths),
// whether or not that column's index appears in requested_col_ids.
// Columns whose indices do not appear in requested_col_ids are
// assigned a width of 0.  This keeps ipac_column_widths in sync with
// other vectors indexed by column_idx.

// This function produces acceptable results for columns of type UINT64_LE even if
// their active_datatype is CHAR.

std::vector<size_t> tablator::Ipac_Table_Writer::get_column_widths(
        const Table &table, const std::vector<size_t> &requested_row_ids,
        const std::vector<size_t> &requested_col_ids,
        const Command_Line_Options &options) {
    std::vector<size_t> widths;

    std::vector<size_t> sorted_col_ids(requested_col_ids);
    std::sort(sorted_col_ids.begin(), sorted_col_ids.end());

    size_t prev_col_idx = 0;
    for (auto col_idx : sorted_col_ids) {
        if (col_idx == 0) {
            // The 0th column is the null bitfield flags, which is not
            //  displayed in ipac_tables.
            continue;
        }
        if (prev_col_idx > col_idx) {
            // We get here if requested_col_ids contained duplicate elements.
            continue;
        }
        while (prev_col_idx < col_idx) {
            widths.push_back(0);
            ++prev_col_idx;
        }

        if (is_valid_col_idx(table, col_idx)) {
            widths.push_back(get_single_column_width(table, requested_row_ids, col_idx,
                                                     options));
            ++prev_col_idx;
        } else {
            // col_idx is too big to correspond to a column of our table.
            break;
        }
    }
    return widths;
}


std::vector<size_t> tablator::Ipac_Table_Writer::get_column_widths(
        const Table &table, const Command_Line_Options &options) {
    return get_column_widths(table, get_all_row_ids(table),
                             get_all_nonzero_col_ids(table), options);
}


/**********************************************************/
/* Wrappers for Data_Type_Adjuster-friendly analogues */
/**********************************************************/

void tablator::Ipac_Table_Writer::write(const tablator::Table &table, std::ostream &os,
                                        const Command_Line_Options &options) {
    std::vector<size_t> included_col_ids = get_all_nonzero_col_ids(table);

    write_subtable_by_column_and_row(
            table, os, included_col_ids, 0, table.get_num_rows(),
            get_column_widths(table, options),
            Data_Type_Adjuster(table).get_datatypes_for_writing(
                    Format::Enums::IPAC_TABLE),
            options);
}

/**********************************************************/

void tablator::Ipac_Table_Writer::write_column_headers(
        const Table &table, std::ostream &os, const Command_Line_Options &options) {
    tablator::Ipac_Table_Writer::write_column_headers(
            table, os, get_column_widths(table, options),
            Data_Type_Adjuster(table).get_datatypes_for_writing(
                    Format::Enums::IPAC_TABLE));
}


/**********************************************************/

void tablator::Ipac_Table_Writer::write_subtable_by_row(
        const Table &table, std::ostream &os,
        const std::vector<size_t> &requested_row_ids,
        const Command_Line_Options &options) {
    std::vector<size_t> included_col_ids = get_all_nonzero_col_ids(table);

    tablator::Ipac_Table_Writer::write_subtable_by_column_and_row(
            table, os, included_col_ids, requested_row_ids,
            get_column_widths(table, requested_row_ids, included_col_ids, options),
            Data_Type_Adjuster(table).get_datatypes_for_writing(
                    Format::Enums::IPAC_TABLE),
            options);
}

/**********************************************************/

void tablator::Ipac_Table_Writer::write_subtable_by_column_and_row(
        const Table &table, std::ostream &os, const std::vector<size_t> &column_ids,
        const std::vector<size_t> &requested_row_ids,
        const Command_Line_Options &options) {
    tablator::Ipac_Table_Writer::write_subtable_by_column_and_row(
            table, os, column_ids, requested_row_ids,
            get_column_widths(table, requested_row_ids, column_ids, options),
            Data_Type_Adjuster(table).get_datatypes_for_writing(
                    Format::Enums::IPAC_TABLE),
            options);
}

/**********************************************************/

void tablator::Ipac_Table_Writer::write_subtable_by_row(
        const Table &table, std::ostream &os, size_t start_row,
        size_t requested_consecutive_row_count, const Command_Line_Options &options) {
    size_t true_row_count =
            get_true_row_count(table, start_row, requested_consecutive_row_count);

    std::vector<size_t> requested_row_ids(true_row_count);
    std::iota(requested_row_ids.begin(), requested_row_ids.end(), start_row);

    std::vector<size_t> included_col_ids = get_all_nonzero_col_ids(table);

    tablator::Ipac_Table_Writer::write_subtable_by_column_and_row(
            table, os, included_col_ids, start_row, true_row_count,
            get_column_widths(table, requested_row_ids, included_col_ids, options),
            Data_Type_Adjuster(table).get_datatypes_for_writing(
                    Format::Enums::IPAC_TABLE),
            options);
}

/**********************************************************/

void tablator::Ipac_Table_Writer::write_subtable_by_column_and_row(
        const Table &table, std::ostream &os, const std::vector<size_t> &column_ids,
        size_t start_row, size_t requested_consecutive_row_count,
        const Command_Line_Options &options) {
    size_t true_row_count =
            get_true_row_count(table, start_row, requested_consecutive_row_count);

    std::vector<size_t> requested_row_ids(true_row_count);
    std::iota(requested_row_ids.begin(), requested_row_ids.end(), start_row);

    tablator::Ipac_Table_Writer::write_subtable_by_column_and_row(
            table, os, column_ids, start_row, true_row_count,
            get_column_widths(table, requested_row_ids, column_ids, options),
            Data_Type_Adjuster(table).get_datatypes_for_writing(
                    Format::Enums::IPAC_TABLE),
            options);
}

/**********************************************************/

void tablator::Ipac_Table_Writer::write_subtable_by_column_and_row(
        const Table &table, std::ostream &os, const std::vector<size_t> &column_ids,
        const Command_Line_Options &options) {
    write_subtable_by_column_and_row(table, os, column_ids, 0, table.get_num_rows(),
                                     options);
}

/**********************************************************/

void tablator::Ipac_Table_Writer::write_single_record(
        const Table &table, std::ostream &os, size_t row_idx,
        const Command_Line_Options &options) {
    std::vector<size_t> requested_row_ids(1, row_idx);
    std::vector<size_t> included_col_ids = get_all_nonzero_col_ids(table);

    tablator::Ipac_Table_Writer::write_single_record_internal(
            table, os, included_col_ids, row_idx,
            get_column_widths(table, requested_row_ids, included_col_ids, options),
            Data_Type_Adjuster(table).get_datatypes_for_writing(
                    Format::Enums::IPAC_TABLE),
            options);
}

/**********************************************************/

void tablator::Ipac_Table_Writer::write_single_record(
        const Table &table, std::ostream &os, const std::vector<size_t> &column_ids,

        size_t row_idx, const Command_Line_Options &options) {
    std::vector<size_t> requested_row_ids(1, row_idx);
    tablator::Ipac_Table_Writer::write_single_record_internal(
            table, os, column_ids, row_idx,
            get_column_widths(table, requested_row_ids, column_ids, options),
            Data_Type_Adjuster(table).get_datatypes_for_writing(
                    Format::Enums::IPAC_TABLE),
            options);
}

/**********************************************************/

void tablator::Ipac_Table_Writer::write_consecutive_records(
        const Table &table, std::ostream &os, size_t start_row,
        size_t requested_consecutive_row_count, const Command_Line_Options &options) {
    size_t true_row_count =
            get_true_row_count(table, start_row, requested_consecutive_row_count);

    std::vector<size_t> requested_row_ids(true_row_count);
    std::iota(requested_row_ids.begin(), requested_row_ids.end(), start_row);

    std::vector<size_t> included_col_ids = get_all_nonzero_col_ids(table);

    tablator::Ipac_Table_Writer::write_consecutive_records(
            table, os, included_col_ids, start_row, true_row_count,
            get_column_widths(table, requested_row_ids, included_col_ids, options),
            Data_Type_Adjuster(table).get_datatypes_for_writing(
                    Format::Enums::IPAC_TABLE),
            options);
}

/**********************************************************/

void tablator::Ipac_Table_Writer::write_consecutive_records(
        const Table &table, std::ostream &os, const std::vector<size_t> &column_ids,
        size_t start_row, size_t requested_consecutive_row_count,
        const Command_Line_Options &options) {
    size_t true_row_count =
            get_true_row_count(table, start_row, requested_consecutive_row_count);

    std::vector<size_t> requested_row_ids(true_row_count);
    std::iota(requested_row_ids.begin(), requested_row_ids.end(), start_row);

    tablator::Ipac_Table_Writer::write_consecutive_records(
            table, os, column_ids, start_row, true_row_count,
            get_column_widths(table, requested_row_ids, column_ids, options),
            Data_Type_Adjuster(table).get_datatypes_for_writing(
                    Format::Enums::IPAC_TABLE),
            options);
}

/**********************************************************/

void tablator::Ipac_Table_Writer::write_selected_records(
        const Table &table, std::ostream &os,
        const std::vector<size_t> &requested_row_ids,
        const Command_Line_Options &options) {
    std::vector<size_t> included_col_ids = get_all_nonzero_col_ids(table);

    tablator::Ipac_Table_Writer::write_selected_records(
            table, os, included_col_ids, requested_row_ids,
            get_column_widths(table, requested_row_ids, included_col_ids, options),
            Data_Type_Adjuster(table).get_datatypes_for_writing(
                    Format::Enums::IPAC_TABLE),
            options);
}

/**********************************************************/

void tablator::Ipac_Table_Writer::write_selected_records(
        const Table &table, std::ostream &os, const std::vector<size_t> &column_ids,
        const std::vector<size_t> &requested_row_ids,
        const Command_Line_Options &options) {
    tablator::Ipac_Table_Writer::write_selected_records(
            table, os, column_ids, requested_row_ids,
            get_column_widths(table, requested_row_ids, column_ids, options),
            Data_Type_Adjuster(table).get_datatypes_for_writing(
                    Format::Enums::IPAC_TABLE),
            options);
}


/*******************************************************/
/* write_keywords_and_comments() and its helpers */
/*******************************************************/

namespace {
static constexpr size_t KEYWORD_ALIGNMENT = 8;

void write_keyword_header_line(std::ostream &os, const std::string &name,
                               const std::string &value) {
    if (name.empty()) {
        throw std::runtime_error("Invalid keyword pair: name must not be empty.");
    }
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

void write_comment_lines(const tablator::Table &table, std::ostream &os,
                         const std::vector<size_t> &included_column_ids,
                         const std::vector<std::string> &comments) {
    const auto &columns = table.get_columns();
    const auto excluded_column_ids = table.find_omitted_column_ids(included_column_ids);

    if (excluded_column_ids.empty()) {
        for (auto &cmt : comments) {
            write_comment_line_with_newlines(os, cmt);
        }
        return;
    }

    // Prepare to skip comments that look like names of excluded columns.
    std::set<std::string> excluded_column_names;
    for (size_t oci : excluded_column_ids) {
        excluded_column_names.insert(columns[oci].get_name());
    }
    for (auto &cmt : comments) {
        size_t cmt_len = cmt.size();
        auto iter = std::find_if(
                excluded_column_names.begin(), excluded_column_names.end(),
                [cmt, cmt_len](const std::string &name) {
                    return (boost::starts_with(cmt, name) &&
                            ((cmt_len == name.size()) || cmt.at(name.size()) == ' '));
                });
        if (iter == excluded_column_names.end()) {
            write_comment_line_with_newlines(os, cmt);
        }
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

    for (size_t col_idx : included_column_ids) {
        if (tablator::Ipac_Table_Writer::is_valid_col_idx(table, col_idx)) {
            const auto &field_props = columns[col_idx].get_field_properties();
            const auto field_prop_attributes = field_props.get_attributes();
            if (!field_prop_attributes.empty() ||
                !field_props.get_description().empty()) {
                std::string col_comment(columns[col_idx].get_name());
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


size_t tablator::Ipac_Table_Writer::get_true_row_count(
        const tablator::Table &table, size_t start_row,
        size_t requested_consecutive_row_count) {
    size_t true_row_count = 0;
    if (start_row < table.get_num_rows()) {
        true_row_count = std::min(table.get_num_rows() - start_row,
                                  requested_consecutive_row_count);
    }
    return true_row_count;
}


bool tablator::Ipac_Table_Writer::is_valid_col_idx(const Table &table, size_t col_idx) {
    return ((col_idx > 0) && (col_idx < table.get_columns().size()));
}

//====================================================

void tablator::Ipac_Table_Writer::write_keywords_and_comments(const Table &table,
                                                              std::ostream &os) {
    write_keywords_and_comments(table, os, get_all_nonzero_col_ids(table),
                                table.get_num_rows());
}


void tablator::Ipac_Table_Writer::write_keywords_and_comments(const Table &table,
                                                              std::ostream &os,
                                                              size_t true_row_count) {
    write_keywords_and_comments(table, os, get_all_nonzero_col_ids(table),
                                true_row_count);
}


void tablator::Ipac_Table_Writer::write_keywords_and_comments(
        const Table &table, std::ostream &os,
        const std::vector<size_t> &included_column_ids) {
    write_keywords_and_comments(table, os, included_column_ids, table.get_num_rows());
}


void tablator::Ipac_Table_Writer::write_keywords_and_comments(
        const Table &table, std::ostream &os,
        const std::vector<size_t> &included_column_ids, size_t true_row_count) {
    static constexpr char const *FIXLEN_STRING = "fixlen = T";
    static constexpr char const *JSON_DESC_LABEL = "RESOURCE.TABLE.DESCRIPTION";

    os << "\\" << FIXLEN_STRING << "\n";
    os << std::left;

    os << "\\" << tablator::Table::ROWS_RETRIEVED_KEYWORD << " = " << true_row_count
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
        if (label.empty()) {
            throw std::runtime_error(
                    "Invalid labeled_property: label must not be empty.");
        }

        auto &prop = name_and_property.second;
        const auto &prop_attributes = prop.get_attributes();
        const auto name_iter = prop_attributes.find(ATTR_NAME);
        const auto value_iter = prop_attributes.find(ATTR_VALUE);
        const auto comment_iter = prop_attributes.find(ATTR_COMMENT);

        if (boost::equals(label, INFO) && (name_iter != prop_attributes.end()) &&
            value_iter != prop_attributes.end() &&
            (prop_attributes.size() == 2 ||
             (comment_iter != prop_attributes.end() && prop_attributes.size() == 3))) {
            // e.g. if we converted from IPAC format to VOTable and are now converting
            // back
            write_keyword_header_line(os, name_iter->second, value_iter->second);
            if (comment_iter != prop_attributes.end()) {
                write_keyword_header_line(os, name_iter->second + "_COMMENT",
                                          comment_iter->second);
            }
        } else if (value_iter != prop_attributes.end()) {
            // e.g. if we're converting from FITS or IPAC table format.
            write_keyword_header_line(os, label, value_iter->second);
            if (comment_iter != prop_attributes.end()) {
                write_keyword_header_line(os, label + "_COMMENT", comment_iter->second);
            }
        } else if (!prop_attributes.empty()) {
            for (const auto &attr_pair : prop_attributes) {
                // e.g. "\type = 'results'
                write_keyword_header_line(os, attr_pair.first, attr_pair.second);
            }
        }

        if (!prop.get_value().empty()) {
            write_keyword_header_line(os, label, prop.get_value());
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
    write_comment_lines(table, os, included_column_ids, comments);
    const auto table_element_description =
            results_resource_element.get_main_table_element().get_description();

    if (!table_element_description.empty()) {
        write_comment_line_with_newlines(os, table_element_description);
    }

    // Add default comments (column name with unit and description) if they aren't
    // present already.
    generate_and_write_default_comments(table, os, included_column_ids, json_comments);
}
