#include "../Table.hxx"

#include <sstream>

#include "../Ascii_Writer.hxx"

//==================================================================
//                 Extractors
//==================================================================

const uint8_t *tablator::Table::extract_value_ptr(size_t col_idx,
                                                  size_t row_idx) const {
    const auto &columns = get_columns();
    if (col_idx >= columns.size()) {
        throw std::runtime_error("Invalid column index: " + std::to_string(col_idx));
    }
    if (row_idx >= get_num_rows()) {
        throw std::runtime_error("Invalid row index: " + std::to_string(row_idx));
    }

    return get_data().data() + row_idx * get_row_size() + get_offsets().at(col_idx);
}

std::string tablator::Table::extract_value_as_string(
        const std::string &col_name, size_t row_idx,
        const Command_Line_Options &options) const {
    size_t col_idx = column_index(col_name);  // throws if col_name is invalid
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

    size_t curr_row_offset = row_idx * get_row_size();
    auto &column = columns[col_idx];
    if (is_null_value(row_idx, col_idx)) {
        auto &null_value = column.get_field_properties().get_values().null;
        return (null_value.empty() ? tablator::Table::DEFAULT_NULL_VALUE : null_value);
    }
    std::stringstream ss;

    // JTODO Write UINT8_LE values the way Ipac_Table_Writer does?
    tablator::Ascii_Writer::write_type_as_ascii(
            ss, column.get_type(), column.get_array_size(),
            data.data() + curr_row_offset + offsets[col_idx],
            tablator::Ascii_Writer::DEFAULT_SEPARATOR, options);
    return ss.str();
}

//==================================================================

std::vector<std::string> tablator::Table::extract_column_values_as_strings(
        const std::string &col_name, const Command_Line_Options &options) const {
    size_t col_idx = column_index(col_name);  // throws if col_name is invalid

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

void insert_blob_to_row_internal(tablator::Row &row, const tablator::Table &table,
                                 size_t col_idx, size_t elt_idx,
                                 const uint8_t *data_ptr,
                                 uint32_t num_elements_to_insert) {
    const auto &column = table.get_columns().at(col_idx);
    auto data_size = tablator::data_size(column.get_type());
    auto insert_offset = table.get_offsets().at(col_idx) + elt_idx * data_size;
    row.insert(data_ptr, data_ptr + num_elements_to_insert * data_size, insert_offset);
}
}  // namespace


//======================================================

void tablator::Table::insert_null_into_row(tablator::Row &row, size_t col_idx,
                                           uint32_t array_size) const {
    validate_parameters(row, *this, col_idx, 0 /* elt_idx */, array_size);
    const auto &column = get_columns().at(col_idx);
    row.set_null(column.get_type(), sizeof(uint32_t), col_idx,
                 get_offsets().at(col_idx), get_offsets().at(col_idx + 1));
}


//===============================================================

void tablator::Table::insert_array_element_into_row(tablator::Row &row, size_t col_idx,
                                                    size_t elt_idx,
                                                    const uint8_t *data_ptr) const {
    validate_parameters(row, *this, col_idx, elt_idx, 1 /* num_elements_to_insert */);
    insert_blob_to_row_internal(row, *this, col_idx, elt_idx, data_ptr, 1);
}
//===============================================================

void tablator::Table::insert_ptr_value_into_row(tablator::Row &row, size_t col_idx,
                                                const uint8_t *data_ptr,
                                                uint32_t array_size) const {
    validate_parameters(row, *this, col_idx, 0 /* start_elt_idx */, array_size);
    insert_blob_to_row_internal(row, *this, col_idx, 0, data_ptr, array_size);
}

void tablator::Table::insert_string_column_value_into_row(
        Row &row, size_t col_idx, const uint8_t *data_ptr,
        uint32_t curr_array_size) const {
    insert_ptr_value_into_row(row, col_idx, reinterpret_cast<const uint8_t *>(data_ptr),
                              curr_array_size);
}
