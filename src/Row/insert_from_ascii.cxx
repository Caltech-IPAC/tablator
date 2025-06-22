#include <stdexcept>

#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>

#include "../Row.hxx"
#include "../data_size.hxx"

namespace tablator {
void Row::insert_from_ascii(const std::string &element, const Data_Type &data_type,
                            const size_t &array_size, const size_t &column,
                            const size_t &offset, const size_t &offset_end) {
    if (array_size != 1 && data_type != Data_Type::CHAR) {
        std::vector<std::string> elements;
        boost::split(elements, element, boost::is_any_of(" "));
        size_t num_elements = elements.size();
        if (num_elements != array_size) {
            throw std::runtime_error(
                    "Expected " + std::to_string(array_size) + " elements, but found " +
                    std::to_string(num_elements) + ": '" + element + "'");
        }
        auto element_offset = offset;
        auto element_size = data_size(data_type);
        for (auto &e : elements) {
            insert_from_ascii(e, data_type, 1, column, element_offset,
                              element_offset + element_size);
            element_offset += element_size;
        }
    } else {
        switch (data_type) {
            case Data_Type::INT8_LE:
                if (element == "?" || element == " " || element[0] == '\0') {
                    insert_null(data_type, array_size, column, offset, offset_end);
                } else {
                    bool result = (boost::iequals(element, "true") ||
                                   boost::iequals(element, "t") || element == "1");
                    if (!result && !(boost::iequals(element, "false") ||
                                     boost::iequals(element, "f") || element == "0")) {
                        throw std::exception();
                    }
                    insert(static_cast<uint8_t>(result), offset);
                }
                break;
            case Data_Type::UINT8_LE: {
                /// Allow hex and octal input
                int result = std::stoi(element, nullptr, 0);
                if (result > std::numeric_limits<uint8_t>::max() ||
                    result < std::numeric_limits<uint8_t>::lowest())
                    throw std::exception();
                insert(static_cast<uint8_t>(result), offset);
            } break;
            case Data_Type::INT16_LE: {
                int result = boost::lexical_cast<int>(element);
                if (result > std::numeric_limits<int16_t>::max() ||
                    result < std::numeric_limits<int16_t>::lowest())
                    throw std::exception();
                insert(static_cast<int16_t>(result), offset);
            } break;
            case Data_Type::UINT16_LE: {
                int result = boost::lexical_cast<int>(element);
                if (result > std::numeric_limits<uint16_t>::max() ||
                    result < std::numeric_limits<uint16_t>::lowest())
                    throw std::exception();
                insert(static_cast<uint16_t>(result), offset);
            } break;
            case Data_Type::INT32_LE: {
                long result = boost::lexical_cast<long>(element);
                if (result > std::numeric_limits<int32_t>::max() ||
                    result < std::numeric_limits<int32_t>::lowest())
                    throw std::exception();
                insert(static_cast<int32_t>(result), offset);
            } break;
            case Data_Type::UINT32_LE: {
                long result = boost::lexical_cast<long>(element);
                if (result > std::numeric_limits<uint32_t>::max() ||
                    result < std::numeric_limits<uint32_t>::lowest())
                    throw std::exception();
                insert(static_cast<uint32_t>(result), offset);
            } break;
            case Data_Type::INT64_LE:
                insert(boost::lexical_cast<int64_t>(element), offset);
                break;
            case Data_Type::UINT64_LE:
                insert(boost::lexical_cast<uint64_t>(element), offset);
                break;
            case Data_Type::FLOAT32_LE:
                insert(boost::lexical_cast<float>(element), offset);
                break;
            case Data_Type::FLOAT64_LE:
                insert(boost::lexical_cast<double>(element), offset);
                break;
            case Data_Type::CHAR:
                insert(element, offset, offset_end);
                break;
            default:
                throw std::runtime_error("Unknown data type in insert_from_ascii(): " +
                                         to_string(data_type));
        }
    }
}
}  // namespace tablator
