#include "../Table.hxx"

#include <sstream>

#include "../Ascii_Writer.hxx"


//==================================================================
//                 Extractors
//==================================================================

const char *tablator::Table::extract_value_ptr(size_t col_idx,
                                                  size_t row_idx) const {
    const auto &columns = get_columns();
    if (col_idx >= columns.size()) {
        throw std::runtime_error("Invalid column index: " + std::to_string(col_idx));
    }
    if (row_idx >= get_num_rows()) {
        throw std::runtime_error("Invalid row index: " + std::to_string(row_idx));
    }

    return get_data().at(row_idx).data() + get_offsets().at(col_idx);
}

std::string tablator::Table::extract_value_as_string(
        const std::string &col_name, size_t row_idx,
        const Command_Line_Options &options) const {
    size_t col_idx = get_column_index(col_name);  // throws if col_name is invalid
    return extract_value_as_string(col_idx, row_idx, options);
}


std::string tablator::Table::extract_value_as_string(
        size_t col_idx, size_t row_idx, const Command_Line_Options &options) const {
    const auto &columns = get_columns();
    const auto &offsets = get_offsets();
    const auto &data = get_data();
    if ((col_idx == 0) || (col_idx >= columns.size())) {
        throw std::runtime_error("Invalid column index: " + std::to_string(col_idx));
    }
    if (row_idx >= get_num_rows()) {
        throw std::runtime_error("Invalid row index: " + std::to_string(row_idx));
    }

#if 0
    size_t curr_row_offset = row_idx * get_row_size();
#endif
    auto &column = columns[col_idx];
    if (is_null_value(row_idx, col_idx)) {
        auto &null_value = column.get_field_properties().get_values().null;
        return (null_value.empty() ? tablator::Table::DEFAULT_NULL_VALUE : null_value);
    }
    std::stringstream ss;

    // JTODO Write UINT8_LE values the way Ipac_Table_Writer does?
    tablator::Ascii_Writer::write_type_as_ascii(
            ss, column.get_type(), column.get_array_size(),
#if 0
            data.data() + curr_row_offset + offsets[col_idx],
#else
			data.at(row_idx).data() + offsets[col_idx],
#endif
            tablator::Ascii_Writer::DEFAULT_SEPARATOR, options);
    return ss.str();
}

//==================================================================

std::vector<std::string> tablator::Table::extract_column_values_as_strings(
        const std::string &col_name, const Command_Line_Options &options) const {
    size_t col_idx = get_column_index(col_name);  // throws if col_name is invalid

    std::vector<std::string> col_vals;

    for (size_t curr_row_idx = 0; curr_row_idx < get_num_rows(); ++curr_row_idx) {
        col_vals.emplace_back(extract_value_as_string(col_idx, curr_row_idx, options));
    }
    return col_vals;
}


//==================================================================
//                 Inserters
//==================================================================

namespace {

void validate_parameters(const tablator::Row &row, const tablator::Table &table,
                         size_t col_idx, size_t start_elt_idx,
                         uint32_t num_elts_to_insert) {
    const auto &table_columns = table.get_columns();

    if (col_idx == 0 || col_idx >= table_columns.size()) {
        std::stringstream msg_ss;
        msg_ss << "Invalid col_idx: " << std::to_string(col_idx);
        throw std::runtime_error(msg_ss.str());
    }
    if (row.get_size() != table.get_row_size()) {
        std::stringstream msg_ss;
        msg_ss << "Invalid col_idx: " << std::to_string(col_idx)
               << ", num_columns: " << table_columns.size();
        ;
        throw std::runtime_error(msg_ss.str());
    }

    const auto &column = table_columns.at(col_idx);
    if (start_elt_idx + num_elts_to_insert > column.get_array_size()) {
        std::stringstream msg_ss;
        msg_ss << "Invalid combination, start_elt_idx: " << start_elt_idx
               << ", num_to_insert: " << num_elts_to_insert
               << ", array_size: " << std::to_string(column.get_array_size());

        throw std::runtime_error(msg_ss.str());
    }
}

//======================================================

void insert_column_element_to_row_internal(tablator::Row &row,
                                           const tablator::Table &table, size_t col_idx,
                                           size_t elt_idx, const char *data_ptr,
                                           uint32_t num_elements_to_insert) {
    // std::cout << "insert_to_row_internal(), col_idx: " << col_idx << std::endl;
    const auto &column = table.get_columns().at(col_idx);
    auto data_size = tablator::data_size(column.get_type());
    auto insert_offset = table.get_offsets().at(col_idx) + elt_idx * data_size;
    row.insert(data_ptr, data_ptr + num_elements_to_insert * data_size, insert_offset,
               table.get_idx_in_dynamic_cols_list(col_idx));
}
}  // namespace


//======================================================

void tablator::Table::insert_null_into_row(tablator::Row &row, size_t col_idx,
                                           uint32_t array_size) const {
    validate_parameters(row, *this, col_idx, 0 /* elt_idx */, array_size);
    const auto &column = get_columns().at(col_idx);
    row.insert_null(column.get_type(), sizeof(uint32_t), col_idx,
                    get_offsets().at(col_idx), get_offsets().at(col_idx + 1),
                    get_idx_in_dynamic_cols_list(col_idx));
}


//===============================================================

void tablator::Table::insert_array_element_into_row(tablator::Row &row, size_t col_idx,
                                                    size_t elt_idx,
                                                    const char *data_ptr) const {
    validate_parameters(row, *this, col_idx, elt_idx, 1 /* num_elements_to_insert */);
    insert_column_element_to_row_internal(row, *this, col_idx, elt_idx, data_ptr, 1);
}

//===============================================================

void tablator::Table::insert_ptr_value_into_row(tablator::Row &row, size_t col_idx,
                                                const char *data_ptr,
                                                uint32_t array_size) const {
    validate_parameters(row, *this, col_idx, 0 /* start_elt_idx */, array_size);
    insert_column_element_to_row_internal(row, *this, col_idx, 0, data_ptr, array_size);
}

//===============================================================

void tablator::Table::insert_string_column_value_into_row(
        Row &row, size_t col_idx, const char *data_ptr,
        uint32_t curr_array_size) const {
    assert(col_idx < get_offsets().size() - 1);
    assert(curr_array_size <= get_columns().at(col_idx).get_array_size());
    // std::cout << "insert_string_column_value(), col_idx: " << col_idx << std::endl;
#if 1
    // JTODO is this copy necessary?  Row is pre-filled with '\0's.
    std::string val_copy(reinterpret_cast<const char *>(data_ptr));
    size_t offset_begin = get_offsets().at(col_idx);
    size_t offset_end = get_offsets().at(col_idx + 1);
    val_copy.resize(offset_end - offset_begin, '\0');
    insert_ptr_value_into_row(row, col_idx,
                              reinterpret_cast<const char *>(val_copy.data()),
                              curr_array_size);
#else
    insert_ptr_value_into_row(row, col_idx, reinterpret_cast<const char *>(data_ptr),
                              curr_array_size);
#endif
}
