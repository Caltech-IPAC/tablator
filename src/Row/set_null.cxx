#include <stdexcept>

#include "../Common.hxx"
#include "../Row.hxx"
#include "../data_size.hxx"

// https://www.ivoa.net/documents/VOTable/20250116/REC-VOTable-1.5.html#tth_sEc5.4

// It is recommended, but not required, that a cell value flagged as
// null is filled with the NaN value for floating point or complex
// datatypes, and zero-valued bytes for other datatypes. It is
// particularly recommended that a variable length array cell value
// flagged as null is represented as 4 zero-valued bytes, indicating a
// zero-length value.

size_t tablator::Row::set_null(const Data_Type &data_type, const size_t &array_size,
                               const size_t &col_idx, const size_t &offset,
                               const size_t &offset_end, bool dynamic_array_flag) {
    const int byte = (col_idx - 1) / 8;
    const char mask = (128 >> ((col_idx - 1) % 8));

    // Update the null_bitfield_flag's bit for this column.
    data_[byte] = data_[byte] | mask;

    size_t curr_offset = offset;
    size_t data_type_size = data_size(data_type);

    if (dynamic_array_flag) {
        char *curr_ptr = data_.data() + curr_offset;
        *(reinterpret_cast<uint32_t *>(curr_ptr)) = 0;

        curr_offset += tablator::DYNAMIC_ARRAY_OFFSET;
    } else {
        // Mark the indicated array elements as null.
        for (size_t i = 0; i < array_size; ++i) {
            set_null_internal(data_type, curr_offset);
            curr_offset += data_type_size;
            if (curr_offset >= offset_end) {
                // Shouldn't happen.
                break;
            }
        }
    }
    return (curr_offset - offset);
}

void tablator::Row::set_null_internal(const Data_Type &data_type,
                                      const size_t &offset) {
    switch (data_type) {
        case Data_Type::INT8_LE:
            insert_null<int8_t>(offset);
            break;
        case Data_Type::UINT8_LE:
            insert_null<uint8_t>(offset);
            break;
        case Data_Type::INT16_LE:
            insert_null<int16_t>(offset);
            break;
        case Data_Type::UINT16_LE:
            insert_null<uint16_t>(offset);
            break;
        case Data_Type::INT32_LE:
            insert_null<int32_t>(offset);
            break;
        case Data_Type::UINT32_LE:
            insert_null<uint32_t>(offset);
            break;
        case Data_Type::INT64_LE:
            insert_null<int64_t>(offset);
            break;
        case Data_Type::UINT64_LE:
            insert_null<uint64_t>(offset);
            break;
        case Data_Type::FLOAT32_LE:
            insert_null<float>(offset);
            break;
        case Data_Type::FLOAT64_LE:
            insert_null<double>(offset);
            break;
        case Data_Type::CHAR:
            insert('\0', offset);
            break;
        default:
            throw std::runtime_error(
                    "Unexpected data type in tablator::Row::set_null()");
    }
}
