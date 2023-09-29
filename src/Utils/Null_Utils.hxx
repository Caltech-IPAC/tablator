#pragma once

#include <iostream>

#include <cassert>
#include <vector>

#include "../Data_Type.hxx"
#include "../data_size.hxx"
#include "Data_Array.hxx"

namespace tablator {

static const std::string null_bitfield_flags_name("null_bitfield_flags");
static const std::string null_bitfield_flags_description(
        "Packed bit array indicating whether an entry is null");


//========================================================

template <typename S, typename T>
void insert_null(std::vector<S> &data, size_t row_start_offset, size_t offset) {
    insert(data, row_start_offset, tablator::get_null<T>(), offset);
}

//========================================================

// Expect S to be either char or unit8_t.
template <typename S>
void set_null_internal(std::vector<S> &data, size_t offset_to_row_start,
                       const Data_Type &data_type, size_t offset) {
    switch (data_type) {
        case Data_Type::INT8_LE:
            insert_null<S, int8_t>(data, offset_to_row_start, offset);
            break;
        case Data_Type::UINT8_LE:
            insert_null<S, uint8_t>(data, offset_to_row_start, offset);
            break;
        case Data_Type::INT16_LE:
            insert_null<S, int16_t>(data, offset_to_row_start, offset);
            break;
        case Data_Type::UINT16_LE:
            insert_null<S, uint16_t>(data, offset_to_row_start, offset);
            break;
        case Data_Type::INT32_LE:
            insert_null<S, int32_t>(data, offset_to_row_start, offset);
            break;
        case Data_Type::UINT32_LE:
            insert_null<S, uint32_t>(data, offset_to_row_start, offset);
            break;
        case Data_Type::INT64_LE:
            insert_null<S, int64_t>(data, offset_to_row_start, offset);
            break;
        case Data_Type::UINT64_LE:
            insert_null<S, uint64_t>(data, offset_to_row_start, offset);
            break;
        case Data_Type::FLOAT32_LE:
            insert_null<S, float>(data, offset_to_row_start, offset);
            break;
        case Data_Type::FLOAT64_LE:
            insert_null<S, double>(data, offset_to_row_start, offset);
            break;
        case Data_Type::CHAR:
            insert(data, offset_to_row_start, '\0', offset);
            break;
        default:
            throw std::runtime_error(
                    "Unexpected data type in tablator::Row::set_null()");
    }
}

//========================================================

// Expect S to be either char or unit8_t.
template <typename S>
void set_null(std::vector<S> &data, size_t offset_to_row_start, Data_Type data_type,
              size_t array_size, size_t col_idx, size_t start_offset_within_row,
              size_t end_offset_within_row) {
    // Each of table's column is represented in null-column land by a single bit,
    // so 8 columns are represented in each byte.
    // Divide by 8 to determine which byte our column's bit lives in...
    const int byte = (col_idx - 1) / 8;

    // ...use remainder to identify the relevant bit of that byte...
    const char mask = (128 >> ((col_idx - 1) % 8));

    // ...then set that bit in that byte of the appropriate row.
    data[offset_to_row_start + byte] = data[offset_to_row_start + byte] | mask;

    // Finally, update the bytes of data corresponding to the column itself.
    size_t curr_offset = start_offset_within_row;
    size_t data_type_size = data_size(data_type);

    for (size_t i = 0; i < array_size; ++i) {
        set_null_internal(data, offset_to_row_start, data_type, curr_offset);
        curr_offset += data_type_size;
        if (curr_offset >= end_offset_within_row) {
            // Shouldn't happen.
            break;
        }
    }
}

//========================================================

}  // namespace tablator
