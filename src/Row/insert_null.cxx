#include <stdexcept>

#include "../Row.hxx"
#include "../data_size.hxx"

void tablator::Row::insert_null(Data_Type data_type, const size_t &array_size,
                                const size_t &offset, const size_t &offset_end,
                                const size_t &col_idx, bool dynamic_array_flag) {
    const int byte = (col_idx - 1) / 8;
    const char mask = (128 >> ((col_idx - 1) % 8));

    // Update the null_bitfield_flag's bit for this column.
    data_[byte] = data_[byte] | mask;

    if (dynamic_array_flag) {
        set_dynamic_array_size(col_idx, 0);
    }

    size_t curr_offset = offset;
    size_t data_type_size = get_data_size(data_type);

    // Mark the indicated array elements as null.
    for (size_t i = 0; i < array_size; ++i) {
        insert_null_by_type(data_type, curr_offset);
        curr_offset += data_type_size;
        if (curr_offset >= offset_end) {
            // Shouldn't happen.
            break;
        }
    }
}

//=========================================================

void tablator::Row::insert_null_by_type(Data_Type data_type, const size_t &offset) {
    switch (data_type) {
        case Data_Type::INT8_LE:
            insert_null_internal<int8_t>(offset);
            break;
        case Data_Type::UINT8_LE:
            insert_null_internal<uint8_t>(offset);
            break;
        case Data_Type::INT16_LE:
            insert_null_internal<int16_t>(offset);
            break;
        case Data_Type::UINT16_LE:
            insert_null_internal<uint16_t>(offset);
            break;
        case Data_Type::INT32_LE:
            insert_null_internal<int32_t>(offset);
            break;
        case Data_Type::UINT32_LE:
            insert_null_internal<uint32_t>(offset);
            break;
        case Data_Type::INT64_LE:
            insert_null_internal<int64_t>(offset);
            break;
        case Data_Type::UINT64_LE:
            insert_null_internal<uint64_t>(offset);
            break;
        case Data_Type::FLOAT32_LE:
            insert_null_internal<float>(offset);
            break;
        case Data_Type::FLOAT64_LE:
            insert_null_internal<double>(offset);
            break;
        case Data_Type::CHAR:
            insert('\0', offset);
            break;
        default:
            throw std::runtime_error("Unexpected data type in insert_null_by_type()");
    }
}
