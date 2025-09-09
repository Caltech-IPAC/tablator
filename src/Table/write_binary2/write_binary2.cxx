#include "../../Table.hxx"

#include <iostream>
#include <map>
#include <sstream>
#include <string>

#include "../../Data_Type_Adjuster.hxx"
#include "../../Utils/Endian_Utils.hxx"
#include "../../to_string.hxx"
#include "Bigendian_Null_Lookup.hxx"

namespace {

//============================================================
//          Support for writing NULL values
//============================================================

// Per https://www.ivoa.net/documents/VOTable/20250116/REC-VOTable-1.5.html#tth_sEc5.4:

// "It is recommended, but not required, that a cell value flagged as null is filled
// with the NaN value for floating point or complex datatypes, and zero-valued bytes
// for other datatypes. It is particularly recommended that a variable length array
// cell value flagged as null is represented as 4 zero-valued bytes, indicating a
// zero-length value."


void write_bigendian_null_array(uint8_t *&write_ptr,
                                tablator::Data_Type datatype_for_writing,
                                uint32_t array_size) {
    const uint8_t *bigendian_null_ptr =
            tablator::Bigendian_Null_Lookup::get_bigendian_null_ptr(
                    datatype_for_writing);

    size_t datatype_size = get_data_size(datatype_for_writing);
    for (uint32_t i = 0; i < array_size; ++i) {
        memcpy(write_ptr, bigendian_null_ptr, datatype_size);
        write_ptr += datatype_size;
    }
}

//============================================================
//           Support for writing non-NULL values
//============================================================

template <typename data_type>
void write_element(uint8_t *&write_ptr, const uint8_t *data_ptr, uint32_t array_size) {
    size_t data_size = sizeof(data_type);
    if (data_size == 1) {
        memcpy(write_ptr, data_ptr, array_size);
        write_ptr += array_size;
        data_ptr += array_size;
    } else {
        for (uint32_t i = 0; i < array_size; ++i) {
            tablator::copy_swapped_bytes(write_ptr, data_ptr, data_size);
            write_ptr += data_size;
            data_ptr += data_size;
        }
    }
}

}  // namespace

//============================================================
//============================================================

namespace tablator {

const std::string encode_base64_stream(const uint8_t *bin2_ptr, size_t bin2_len);

//============================================================

void Table::splice_binary2_and_write(std::ostream &os, std::stringstream &ss,
                                     uint num_spaces_left,
                                     uint num_spaces_right) const {
    std::string s(ss.str());
    size_t binary2_offset(s.find(BINARY2_PLACEHOLDER));
    os << s.substr(0, binary2_offset - num_spaces_left);
    write_binary2(os);
    os << s.substr(binary2_offset + BINARY2_PLACEHOLDER.size() + num_spaces_right);
}

//============================================================

void Table::write_binary2(std::ostream &os) const {
    const auto &columns = get_columns();
    const auto &offsets = get_offsets();
    const auto &data = get_data();
    const std::vector<Data_Type> datatypes_for_writing =
            Data_Type_Adjuster(*this).get_datatypes_for_writing(
                    Format::Enums::VOTABLE_BINARY2);

    // Gather table-level info.
    const uint8_t *data_start_ptr = get_data().data();
    const size_t num_rows = get_num_rows();
    size_t num_dynamic_columns = get_data_details().get_num_dynamic_columns();

    // Create buffer for writing.

    // Per IVOA spec quoted above, variable-length arrays of
    // primitives are preceded by a 4-byte integer
    // whose value is the number of items of the array.

    std::vector<uint8_t> write_vec;
    write_vec.resize(4 * num_dynamic_columns * num_rows + data.size());
    std::fill(write_vec.begin(), write_vec.end(), 0);

    const uint8_t *write_buff = write_vec.data();
    uint8_t *write_ptr_orig = const_cast<uint8_t *>(write_buff);
    uint8_t *write_ptr = write_ptr_orig;
    const uint8_t *row_start_ptr = data_start_ptr;

    // Write to buffer.
    for (size_t row_idx = 0; row_idx < num_rows;
         ++row_idx, row_start_ptr += get_row_size()) {
        for (size_t col_idx = 0; col_idx < columns.size(); ++col_idx) {
            auto &column = columns[col_idx];
            uint32_t array_size = column.get_array_size();
            bool dynamic_array_flag = column.get_dynamic_array_flag();

            const uint8_t *curr_data_ptr = row_start_ptr + offsets[col_idx];
            Data_Type datatype_for_writing = datatypes_for_writing[col_idx];

            // Check for null values. null_bitfield_flags are never null.
            if (col_idx > 0) {
                bool null_flag_is_set = is_null_value(row_idx, col_idx);
                // Follow recommendation of IVOA spec quoted above.
                if (null_flag_is_set) {
                    if (dynamic_array_flag) {
                        // No need to swap here; big-endian and
                        // little-endian representations of 0 are the
                        // same.
                        *(reinterpret_cast<uint32_t *>(write_ptr)) = 0;
                        write_ptr += sizeof(uint32_t);
                    } else {
                        write_bigendian_null_array(write_ptr, datatype_for_writing,
                                                   array_size);
                    }
                    continue;
                }
            }  // end col_idx > 0

            uint32_t curr_array_size = array_size;
            if (dynamic_array_flag) {
                curr_array_size =
                        get_data_details().get_dynamic_array_size(row_idx, col_idx);
                copy_swapped_bytes(write_ptr,
                                   reinterpret_cast<const uint8_t *>(&curr_array_size),
                                   sizeof(uint32_t));

                write_ptr += sizeof(uint32_t);
            }

            switch (datatype_for_writing) {
                case Data_Type::INT8_LE: {
                    // std::cout << "INT8_LE" << std::endl;
                    for (uint32_t i = 0; i < array_size; ++i) {
                        uint8_t element =
                                *(reinterpret_cast<const uint8_t *>(curr_data_ptr));
                        bool result =
                                (element == 't' || element == 'T' || element == '1' ||
                                 element == true || element == static_cast<uint8_t>(1));
                        // std::cout << (result ? "true" : "false") << std::endl;
                        if (!result &&
                            !(element == 'f' || element == 'F' || element == '0' ||
                              element == false || element == static_cast<uint8_t>(0))) {
                            throw std::exception();
                        }
                        uint8_t final = result ? static_cast<uint8_t>('1')
                                               : static_cast<uint8_t>('0');
                        memcpy(write_ptr, &final, 1);
                        ++write_ptr;
                        ++curr_data_ptr;
                    }

                } break;
                case Data_Type::UINT8_LE: {
                    // std::cout << "UINT8_LE" << std::endl;
                    write_element<uint8_t>(write_ptr, curr_data_ptr, array_size);
                } break;
                case Data_Type::INT16_LE: {
                    // std::cout << "INT16_LE" << std::endl;
                    write_element<int16_t>(write_ptr, curr_data_ptr, array_size);
                } break;
                case Data_Type::UINT16_LE: {
                    // std::cout << "UINT16_LE" << std::endl;
                    write_element<uint16_t>(write_ptr, curr_data_ptr, array_size);
                } break;
                case Data_Type::INT32_LE: {
                    // std::cout << "INT32_LE" << std::endl;
                    write_element<int32_t>(write_ptr, curr_data_ptr, array_size);
                } break;
                case Data_Type::UINT32_LE: {
                    // std::cout << "UINT32_LE" << std::endl;
                    write_element<uint32_t>(write_ptr, curr_data_ptr, array_size);
                } break;
                case Data_Type::INT64_LE: {
                    // std::cout << "INT64_LE" << std::endl;
                    write_element<int64_t>(write_ptr, curr_data_ptr, array_size);
                } break;
                case Data_Type::UINT64_LE: {
                    // std::cout << "UINT64_LE" << std::endl;
                    write_element<uint64_t>(write_ptr, curr_data_ptr, array_size);
                } break;
                case Data_Type::FLOAT32_LE: {
                    // std::cout << "FLOAT32_LE" << std::endl;
                    write_element<float>(write_ptr, curr_data_ptr, array_size);
                } break;
                case Data_Type::FLOAT64_LE: {
                    // std::cout << "FLOAT64_LE" << std::endl;
                    write_element<double>(write_ptr, curr_data_ptr, array_size);
                } break;
                case Data_Type::CHAR: {
                    // std::cout << "CHAR" << std::endl;
                    write_element<char>(write_ptr, curr_data_ptr, curr_array_size);
                } break;
                default:
                    throw std::runtime_error(
                            "Unknown data type when writing data in binary2 "
                            "format: " +
                            to_string(column.get_type()));
                    break;
            }
        }
    }

    const std::string base64_str = encode_base64_stream(
            write_ptr_orig, std::distance(write_ptr_orig, write_ptr));
    os << '\n' << base64_str << '\n';
}

}  // namespace tablator
