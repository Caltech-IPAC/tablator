#include <stdexcept>

#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>

#include "../Row.hxx"
#include "../data_size.hxx"

namespace tablator {

// Caller has handled nulls (except possibly for INT8_LE).
#if 0
void Row::insert_from_ascii(const std::string &value, const Data_Type &data_type,
                            const size_t &array_size, const size_t &col_idx,
                            const size_t &offset, const size_t &offset_end,
                            const size_t &idx_in_dynamic_cols_list) {
    set_array_size_if_dynamic(idx_in_dynamic_cols_list, array_size);

    if (array_size != 1 && data_type != Data_Type::CHAR) {
        std::vector<std::string> elements;
        boost::split(elements, value, boost::is_any_of(" "));
        size_t num_elements = elements.size();
        if (num_elements != array_size) {
            throw std::runtime_error(
                    "Expected " + std::to_string(array_size) + " elements, but found " +
                    std::to_string(num_elements) + ": '" + value + "'");
        }
        auto element_offset = offset;
        auto element_size = get_data_size(data_type);
        for (auto &e : elements) {
            insert_from_ascii(e, data_type, 1, col_idx, element_offset,
                              element_offset + element_size,
                              DEFAULT_IDX_IN_DYNAMIC_COLS_LIST);
            element_offset += element_size;
        }
    } else {
        switch (data_type) {
            case Data_Type::INT8_LE:
                if (value == "?" || value == " " || value[0] == '\0') {
                    insert_null(data_type, array_size, col_idx, offset, offset_end);
                } else {
                    bool result = (boost::iequals(value, "true") ||
                                   boost::iequals(value, "t") || value == "1");
                    if (!result && !(boost::iequals(value, "false") ||
                                     boost::iequals(value, "f") || value == "0")) {
                        throw std::exception();
                    }
                    insert(static_cast<uint8_t>(result), offset);
                }
                break;
            case Data_Type::UINT8_LE: {
                /// Allow hex and octal input
                int result = std::stoi(value, nullptr, 0);
                if (result > std::numeric_limits<uint8_t>::max() ||
                    result < std::numeric_limits<uint8_t>::lowest())
                    throw std::exception();
                insert(static_cast<uint8_t>(result), offset);
            } break;
            case Data_Type::INT16_LE: {
                int result = boost::lexical_cast<int>(value);
                if (result > std::numeric_limits<int16_t>::max() ||
                    result < std::numeric_limits<int16_t>::lowest())
                    throw std::exception();
                insert(static_cast<int16_t>(result), offset);
            } break;
            case Data_Type::UINT16_LE: {
                int result = boost::lexical_cast<int>(value);
                if (result > std::numeric_limits<uint16_t>::max() ||
                    result < std::numeric_limits<uint16_t>::lowest())
                    throw std::exception();
                insert(static_cast<uint16_t>(result), offset);
            } break;
            case Data_Type::INT32_LE: {
                long result = boost::lexical_cast<long>(value);
                if (result > std::numeric_limits<int32_t>::max() ||
                    result < std::numeric_limits<int32_t>::lowest())
                    throw std::exception();
                insert(static_cast<int32_t>(result), offset);
            } break;
            case Data_Type::UINT32_LE: {
                long result = boost::lexical_cast<long>(value);
                if (result > std::numeric_limits<uint32_t>::max() ||
                    result < std::numeric_limits<uint32_t>::lowest())
                    throw std::exception();
                insert(static_cast<uint32_t>(result), offset);
            } break;
            case Data_Type::INT64_LE:
                insert(boost::lexical_cast<int64_t>(value), offset);
                break;
            case Data_Type::UINT64_LE:
                insert(boost::lexical_cast<uint64_t>(value), offset);
                break;
            case Data_Type::FLOAT32_LE:
                insert(boost::lexical_cast<float>(value), offset);
                break;
            case Data_Type::FLOAT64_LE:
                insert(boost::lexical_cast<double>(value), offset);
                break;
            case Data_Type::CHAR:
                insert(value, offset, offset_end, DEFAULT_IDX_IN_DYNAMIC_COLS_LIST);
                break;
            default:
                throw std::runtime_error("Unknown data type in insert_from_ascii(): " +
                                         to_string(data_type));
        }
    }
}
#endif
void Row::insert_from_ascii(const std::string &value, const Data_Type &data_type,
                            const size_t &array_size,
                            const size_t &offset, const size_t &offset_end, const size_t &col_idx, bool dynamic_array_flag) {
  //  std::cout << "insert_from_ascii(), enter, daf: " << dynamic_array_flag << std::endl;
  if (dynamic_array_flag) {
    set_dynamic_array_size(col_idx, array_size);
  }

    if (array_size != 1 && data_type != Data_Type::CHAR) {
        std::vector<std::string> elements;
        boost::split(elements, value, boost::is_any_of(" "));
        size_t num_elements = elements.size();
        if (num_elements != array_size) {
            throw std::runtime_error(
                    "Expected " + std::to_string(array_size) + " elements, but found " +
                    std::to_string(num_elements) + ": '" + value + "'");
        }
        auto element_offset = offset;
        auto element_size = get_data_size(data_type);
        for (auto &e : elements) {
            insert_from_ascii(e, data_type, 1, element_offset,
                              element_offset + element_size, col_idx, dynamic_array_flag);
            element_offset += element_size;
        }
    } else {
        switch (data_type) {
            case Data_Type::INT8_LE:
                if (value == "?" || value == " " || value[0] == '\0') {
				  insert_null(data_type, array_size, offset, offset_end, col_idx, dynamic_array_flag);
                } else {
                    bool result = (boost::iequals(value, "true") ||
                                   boost::iequals(value, "t") || value == "1");
                    if (!result && !(boost::iequals(value, "false") ||
                                     boost::iequals(value, "f") || value == "0")) {
                        throw std::exception();
                    }
                    insert(static_cast<uint8_t>(result), offset);
                }
                break;
            case Data_Type::UINT8_LE: {
                /// Allow hex and octal input
                int result = std::stoi(value, nullptr, 0);
                if (result > std::numeric_limits<uint8_t>::max() ||
                    result < std::numeric_limits<uint8_t>::lowest())
                    throw std::exception();
                insert(static_cast<uint8_t>(result), offset);
            } break;
            case Data_Type::INT16_LE: {
                int result = boost::lexical_cast<int>(value);
                if (result > std::numeric_limits<int16_t>::max() ||
                    result < std::numeric_limits<int16_t>::lowest())
                    throw std::exception();
                insert(static_cast<int16_t>(result), offset);
            } break;
            case Data_Type::UINT16_LE: {
                int result = boost::lexical_cast<int>(value);
                if (result > std::numeric_limits<uint16_t>::max() ||
                    result < std::numeric_limits<uint16_t>::lowest())
                    throw std::exception();
                insert(static_cast<uint16_t>(result), offset);
            } break;
            case Data_Type::INT32_LE: {
                long result = boost::lexical_cast<long>(value);
                if (result > std::numeric_limits<int32_t>::max() ||
                    result < std::numeric_limits<int32_t>::lowest())
                    throw std::exception();
                insert(static_cast<int32_t>(result), offset);
            } break;
            case Data_Type::UINT32_LE: {
                long result = boost::lexical_cast<long>(value);
                if (result > std::numeric_limits<uint32_t>::max() ||
                    result < std::numeric_limits<uint32_t>::lowest())
                    throw std::exception();
                insert(static_cast<uint32_t>(result), offset);
            } break;
            case Data_Type::INT64_LE:
                insert(boost::lexical_cast<int64_t>(value), offset);
                break;
            case Data_Type::UINT64_LE:
                insert(boost::lexical_cast<uint64_t>(value), offset);
                break;
            case Data_Type::FLOAT32_LE:
                insert(boost::lexical_cast<float>(value), offset);
                break;
            case Data_Type::FLOAT64_LE:
                insert(boost::lexical_cast<double>(value), offset);
                break;
            case Data_Type::CHAR:
			  insert(value, offset, offset_end, col_idx, dynamic_array_flag);
                break;
            default:
                throw std::runtime_error("Unknown data type in insert_from_ascii(): " +
                                         to_string(data_type));
        }
    }
}


}  // namespace tablator
