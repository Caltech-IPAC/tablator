#include <cmath>
#include <limits>
#include <string>

#include "../../Ascii_Writer.hxx"
#include "../../Data_Type_Adjuster.hxx"
#include "../../Table.hxx"
#include "../../to_string.hxx"

#include <fstream>
#include <iostream>
#include <sstream>



namespace {

/**********************************************************/
/*  Convert from  little-endian to big-endian             */
/**********************************************************/

  // Copy data_size bytes in reverse order from sec to dest.
void swap_copy(uint8_t *dest, const uint8_t *src, size_t data_size) {
    for (size_t i = 0; i < data_size; i++) {
        dest[data_size - 1 - i] = src[i];
    }
}

//**********************************************************/
/*  Support for writing NULL values                        */
//**********************************************************/

// Per https://www.ivoa.net/documents/VOTable/20250116/REC-VOTable-1.5.html#tth_sEc5.4:
    // "It is recommended, but not required, that a cell value flagged as null is filled
    // with the NaN value for floating point or complex datatypes, and zero-valued bytes
    // for other datatypes. It is particularly recommended that a variable length array
    // cell value flagged as null is represented as 4 zero-valued bytes, indicating a
    // zero-length value."



template <typename T>
inline typename std::enable_if<std::is_integral<T>::value, T>::type
get_binary2_null() {
    if (std::is_same<T, char>::value) {
        return '\0';
    }
    return T(0);
}

template <typename T>
inline typename std::enable_if<std::is_floating_point<T>::value, T>::type
get_binary2_null() {
    return std::numeric_limits<T>::quiet_NaN();
}

/**********************************************************/

template <typename T>
void write_binary2_null(uint8_t *&write_ptr, size_t array_size) {

    T little_endian_null = get_binary2_null<T>();
    uint8_t *little_endian_null_ptr = reinterpret_cast<uint8_t *>(&little_endian_null);

    T big_endian_null = 0;
    uint8_t *big_endian_null_ptr = reinterpret_cast<uint8_t *>(&big_endian_null);

    swap_copy(big_endian_null_ptr, little_endian_null_ptr, sizeof(T));

    for (size_t i = 0; i < array_size; ++i) {
       //  *(reinterpret_cast<T *>(write_ptr)) = get_binary2_null<T>();
        memcpy(write_ptr, big_endian_null_ptr, sizeof(T));
        write_ptr += sizeof(T);
    }
}

/**********************************************************/

void write_binary2_null_for_datatype(uint8_t *&write_ptr,
                             tablator::Data_Type datatype_for_writing,
                             size_t array_size) {
    // It is recommended, but not required, that a cell value flagged as null is filled
    // with the NaN value for floating point or complex datatypes, and zero-valued bytes
    // for other datatypes. It is particularly recommended that a variable length array
    // cell value flagged as null is represented as 4 zero-valued bytes, indicating a
    // zero-length value.

    switch (datatype_for_writing) {
        case tablator::Data_Type::INT8_LE: {
            write_binary2_null<bool>(write_ptr, array_size);
        } break;
        case tablator::Data_Type::UINT8_LE: {
            write_binary2_null<uint8_t>(write_ptr, array_size);
        } break;
        case tablator::Data_Type::INT16_LE: {
            write_binary2_null<int16_t>(write_ptr, array_size);
        } break;
        case tablator::Data_Type::UINT16_LE: {
            write_binary2_null<uint16_t>(write_ptr, array_size);
        } break;
        case tablator::Data_Type::INT32_LE: {
            write_binary2_null<int32_t>(write_ptr, array_size);
        } break;
        case tablator::Data_Type::UINT32_LE: {
            write_binary2_null<uint32_t>(write_ptr, array_size);
        } break;
        case tablator::Data_Type::INT64_LE: {
            write_binary2_null<int64_t>(write_ptr, array_size);
        } break;
        case tablator::Data_Type::UINT64_LE: {
            write_binary2_null<uint64_t>(write_ptr, array_size);
        } break;
        case tablator::Data_Type::FLOAT32_LE: {
            write_binary2_null<float>(write_ptr, array_size);
        } break;
        case tablator::Data_Type::FLOAT64_LE: {
            write_binary2_null<double>(write_ptr, array_size);
            ;
        } break;
        case tablator::Data_Type::CHAR: {
		  // JTODO
            write_binary2_null<char>(write_ptr, array_size);
        } break;
        default:
            throw std::runtime_error(
                    "Unknown data type when writing data in binary2 "
                    "format: ");
            break;
    }
}

/**********************************************************/
/*  Support for writing non-NULL values                   */
/**********************************************************/


template <typename data_type>
void write_element(uint8_t *&write_ptr, const uint8_t *data_ptr, size_t array_size) {
    // Do we allow individual array elts to be null?

    size_t data_size = sizeof(data_type);
    if (data_size == 1) {
        memcpy(write_ptr, data_ptr, array_size);
        write_ptr += array_size;
        data_ptr += array_size;
    } else {
        for (size_t i = 0; i < array_size; ++i) {
            swap_copy(write_ptr, data_ptr, data_size);
            write_ptr += data_size;
            data_ptr += data_size;
        }
    }
}

}  // namespace

/**********************************************************/
/**********************************************************/

namespace tablator {
  const std::string encode_base64_stream(const char *bin2_ptr, size_t bin2_len);

void Table::splice_binary2_and_write(std::ostream &os, std::stringstream &ss,
                                     Format::Enums enum_format, uint num_spaces_left,
                                     uint num_spaces_right) const {
    std::string s(ss.str());
    size_t binary2_offset(s.find(BINARY2_PLACEHOLDER));
    os << s.substr(0, binary2_offset - num_spaces_left);
    write_binary2(os, enum_format);
    os << s.substr(binary2_offset + BINARY2_PLACEHOLDER.size() + num_spaces_right);
}

  /**********************************************************/

  void Table::write_binary2(std::ostream &os, const Format::Enums &output_format) const {
    std::string tr_prefix, tr_suffix, td_suffix, tr_indent;
    std::string binary2_indent = "                    ";
    const bool is_json(output_format == Format::Enums::JSON ||
                       output_format == Format::Enums::JSON5);

    // JTODO clean up

    if (is_json) {
        binary2_indent = "                    ";
        tr_indent = binary2_indent + "    ";
        tr_prefix = tr_indent + "[\n";
        tr_suffix = tr_indent + "]";

        os << '\n' << binary2_indent << "[\n";
    } else {
        binary2_indent = "        ";
        tr_indent = binary2_indent + "  ";
        tr_prefix = tr_indent + "<TR>\n";
        tr_suffix = tr_indent + "</TR>";

        os << '\n';
    }

    const auto &columns = get_columns();
    const auto &offsets = get_offsets();
    const auto &data = get_data();

    const std::vector<Data_Type> datatypes_for_writing =
            Data_Type_Adjuster(*this).get_datatypes_for_writing(
                    Format::Enums::VOTABLE_BINARY2);

    // Retrieve table's data pointer.
    const uint8_t *data_start_ptr = get_data().data();

    // Create buffer for writing.

    std::vector<uint8_t> write_vec;
	// JTODO simplify now that we store dynamic_array_size internally
    write_vec.resize(
					 4 * columns.size() * get_num_rows() +
            get_data().size());  // reserve?  JTODO allow for array_size entries
    std::fill(write_vec.begin(), write_vec.end(), 0);
    const uint8_t *write_buff = write_vec.data();
    uint8_t *write_ptr_orig = const_cast<uint8_t *>(write_buff);
    uint8_t *write_ptr = write_ptr_orig;

    const size_t number_of_rows(get_num_rows());
    const uint8_t *row_start_ptr = data_start_ptr;

    for (size_t row_idx = 0; row_idx < number_of_rows;
         ++row_idx, row_start_ptr += get_row_size()) {


        for (size_t col_idx = 0; col_idx < columns.size(); ++col_idx) {

            auto &column = columns[col_idx];

            const uint8_t *curr_data_ptr = row_start_ptr + offsets[col_idx];
            size_t curr_row_start_offset = row_idx * get_row_size();
            Data_Type datatype_for_writing = datatypes_for_writing[col_idx];

            size_t array_size = column.get_array_size();  // JTODO max_?
            bool dynamic_array_flag = column.get_dynamic_array_flag();

            if (col_idx > 0) {
                bool null_flag_is_set = is_null(curr_row_start_offset, col_idx);

				// JTODO rename
                bool all_or_nothing_null = !dynamic_array_flag;
                // ((array_size == 1) || !dynamic_array_flag;

                if (null_flag_is_set) {
                    if (all_or_nothing_null) {
                        // No need to inspect individual array elements.  // JTODO

                        // JTODO NULL_NOT_DYNAMIC
                        write_binary2_null_for_datatype(write_ptr, datatype_for_writing,
                                                array_size);
                    }

                    else {  // dynamic_array_flag
					  // Follow recommendation of  IVOA spec quoted above.

					  // No need to swap here; big-endian and
					  // little-endian representations of 0 are the
					  // same.

                        *(reinterpret_cast<uint32_t *>(write_ptr)) = 0;
                        write_ptr += sizeof(uint32_t);
                    }
                    continue;
                } // end null_flag_is_set
            } // end col_idx > 0

            uint32_t curr_array_size = 0;
            if (dynamic_array_flag) {
					  // Follow recommendation of  IVOA spec quoted above.

                // Variable-length arrays of primitives are preceded by a 4-byte integer
                // containing the number of items of the array. JTODO how do we get
                // per-row array_size?  Do we not support variable-length arrays except
                // for char?

			  // JTODO  read it from curr_data_ptr
#if 0
                curr_array_size = std::min(column.get_array_size(),
                                           strlen(reinterpret_cast<const char *>(
                                                   curr_data_ptr)));  // JTODO
#else
				curr_array_size = *(reinterpret_cast<const uint32_t *>(curr_data_ptr));
				curr_data_ptr += sizeof(uint32_t);
#endif
                swap_copy(write_ptr,
                          reinterpret_cast<const uint8_t *>(&curr_array_size),
                          sizeof(uint32_t));

                write_ptr += sizeof(uint32_t);
            }

            switch (datatype_for_writing) {
                case Data_Type::INT8_LE: {
                    // JTODO is this one tricky?
                    // std::cout << "INT8_LE" << std::endl;
                    for (size_t i = 0; i < array_size; ++i) {
                        uint8_t element =
                                *(reinterpret_cast<const uint8_t *>(curr_data_ptr));
                        bool result =
                                (element == 't' || element == 'T' || element == '1' ||
                                 element == true || element == static_cast<uint8_t>(1));
                        // std::cout << (result ? "true" : "false") << std::endl;
                        if (!result &&
                            !(element == 'f' || element == 'F' || element == '0' ||
                              element == false || element == static_cast<uint8_t>(0))) {
                            // std::cout << "before throw" << std::endl << std::flush;
                            // std::cout << "element: X" << element << "X" << std::endl;
                            throw std::exception();
                        }
                        //					uint8_t final = result ?
                        //static_cast<uint8_t>(true) : static_cast<uint8_t>(false);
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
                    write_element<
                            char>(write_ptr, curr_data_ptr, curr_array_size);
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

    const std::string base64_str =
            encode_base64_stream(reinterpret_cast<char *>(write_ptr_orig),
                          std::distance(write_ptr_orig, write_ptr));  // JTODO
    os << base64_str << '\n' << tr_indent;
}

}  // namespace tablator
