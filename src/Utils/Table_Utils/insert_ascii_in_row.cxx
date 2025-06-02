#include <stdexcept>

#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>

#include "../../data_size.hxx"
#include "../Table_Utils.hxx"

namespace tablator {
  void insert_ascii_in_row(Row &row, const Data_Type &data_type, const size_t &array_size,
                         const size_t &col_idx, const std::string &element,
                         const size_t &offset, const size_t &offset_end, bool dynamic_array_flag) {
	// JTODO
        std::vector<std::string> elements;
		// std::cout << "insert_ascii_in_row(), enter, array_size: " << array_size << ", offset: " << offset << ", offset_end: " << offset_end << std::endl;
  size_t curr_array_size = array_size;
  size_t curr_offset = offset;
  if (array_size != 1) {
	if (data_type == Data_Type::CHAR) {
	  curr_array_size = element.size();
	} else {
        boost::split(elements, element, boost::is_any_of(" "));
		size_t num_elements = elements.size();
		curr_array_size = num_elements; // JTODO
        if ((num_elements < array_size && !dynamic_array_flag) || num_elements > array_size) {
            throw std::runtime_error(
                    "Expected no more than " + std::to_string(array_size) + " elements, but found " +
                    std::to_string(num_elements) + ": '" + element + "'");
		}
	}
  }

  // std::cout << "dynamic_array_flag: " << dynamic_array_flag << std::endl;
  if (dynamic_array_flag) {
	// std::cout << "insert_ascii(), writing size to row: " << curr_array_size << std::endl;
	row.insert(static_cast<uint32_t>(curr_array_size), curr_offset);	
	curr_offset += sizeof(uint32_t);
  } else {
	// std::cout << "insert_ascii(), not writing size to row: " << curr_array_size << std::endl;
  }

	if (curr_array_size != 1 && data_type != Data_Type::CHAR) {
#if 0
        std::vector<std::string> elements;
        boost::split(elements, element, boost::is_any_of(" "));
		size_t num_elements = elements.size();
        if ((num_elements < array_size && !dynamic_array_flag) || num_elements > array_size) {
            throw std::runtime_error(
                    "Expected no more than " + std::to_string(array_size) + " elements, but found " +
                    std::to_string(num_elements) + ": '" + element + "'");
		}
#endif
		// std::cout << "insert_ascii(), curr_array_size: " << curr_array_size << ", curr_offset: " << curr_offset  << std::endl;
        auto element_offset = curr_offset;
        auto element_size = data_size(data_type);
        for (auto &e : elements) {
		  insert_ascii_in_row(row, data_type, 1, col_idx, e, element_offset,
							  element_offset + element_size, false /* dynamic_array_flag */);
            element_offset += element_size;
        }
		if (curr_array_size < array_size) {
		  // std::cout << "curr_array_size < array_size" << std::endl;
		}
    } else {
        switch (data_type) {
            case Data_Type::INT8_LE:
                // std::cout << "insert_ascii(), INT8_LE" << std::endl;
                if (element == "?" || element == " " || element[0] == '\0') {
                    row.set_null(data_type, array_size, col_idx, curr_offset, offset_end);
                } else {
                    bool result = (boost::iequals(element, "true") ||
                                   boost::iequals(element, "t") || element == "1");
                    if (!result && !(boost::iequals(element, "false") ||
                                     boost::iequals(element, "f") || element == "0")) {
                        throw std::exception();
                    }
                    row.insert(static_cast<uint8_t>(result), curr_offset);
                }
                break;
            case Data_Type::UINT8_LE: {
                /// Allow hex and octal input
                int result = std::stoi(element, nullptr, 0);
                if (result > std::numeric_limits<uint8_t>::max() ||
                    result < std::numeric_limits<uint8_t>::lowest())
                    throw std::exception();
                row.insert(static_cast<uint8_t>(result), curr_offset);
            } break;
            case Data_Type::INT16_LE: {
                int result = boost::lexical_cast<int>(element);
                if (result > std::numeric_limits<int16_t>::max() ||
                    result < std::numeric_limits<int16_t>::lowest())
                    throw std::exception();
                row.insert(static_cast<int16_t>(result), curr_offset);
            } break;
            case Data_Type::UINT16_LE: {
                int result = boost::lexical_cast<int>(element);
                if (result > std::numeric_limits<uint16_t>::max() ||
                    result < std::numeric_limits<uint16_t>::lowest())
                    throw std::exception();
                row.insert(static_cast<uint16_t>(result), curr_offset);
            } break;
            case Data_Type::INT32_LE: {
                long result = boost::lexical_cast<long>(element);
                if (result > std::numeric_limits<int32_t>::max() ||
                    result < std::numeric_limits<int32_t>::lowest())
                    throw std::exception();
                row.insert(static_cast<int32_t>(result), curr_offset);
            } break;
            case Data_Type::UINT32_LE: {
                long result = boost::lexical_cast<long>(element);
                if (result > std::numeric_limits<uint32_t>::max() ||
                    result < std::numeric_limits<uint32_t>::lowest())
                    throw std::exception();
                row.insert(static_cast<uint32_t>(result), curr_offset);
            } break;
            case Data_Type::INT64_LE:
                row.insert(boost::lexical_cast<int64_t>(element), curr_offset);
                break;
            case Data_Type::UINT64_LE:
                row.insert(boost::lexical_cast<uint64_t>(element), curr_offset);
                break;
            case Data_Type::FLOAT32_LE:
                row.insert(boost::lexical_cast<float>(element), curr_offset);
                break;
            case Data_Type::FLOAT64_LE:
                row.insert(boost::lexical_cast<double>(element), curr_offset);
                break;
            case Data_Type::CHAR:
			  // std::cout << "insert_ascii(), CHAR, before row.insert(), distance: " << offset_end - curr_offset << std::endl;
                row.insert(element, curr_offset, offset_end);
                break;
            default:
                throw std::runtime_error(
                        "Unknown data type in insert_ascii_in_row(): " +
                        to_string(data_type));
        }
    }
}
}  // namespace tablator
