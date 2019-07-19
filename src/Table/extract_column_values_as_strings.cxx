#include "../Table.hxx"

std::vector<std::string> tablator::Table::extract_column_values_as_strings(
        const std::string &colname) const {
    std::vector<std::string> vals;

    auto column_iter = find_column(colname);
    if (column_iter == columns.end()) {
        throw std::runtime_error("Table does not contain a column named " + colname +
                                 ".");
    }

    const size_t offset(column_offset(colname));

    for (size_t element_offset = offset; element_offset < data.size();
         element_offset += row_size()) {
        auto curr_ptr = data.data() + element_offset;
        switch (column_iter->type) {
            case Data_Type::INT8_LE:
                vals.emplace_back(std::to_string(static_cast<int>(*curr_ptr)));
                break;
            case Data_Type::UINT8_LE: {
                std::stringstream ss;
                ss << "0x" << std::hex << static_cast<const uint16_t>(*curr_ptr)
                   << std::dec;
                vals.emplace_back(ss.str());
            } break;
            case Data_Type::INT16_LE:
                vals.emplace_back(
                        std::to_string(*reinterpret_cast<const int16_t *>(curr_ptr)));
                break;
            case Data_Type::UINT16_LE:
                vals.emplace_back(
                        std::to_string(*reinterpret_cast<const uint16_t *>(curr_ptr)));
                break;
            case Data_Type::INT32_LE:
                vals.emplace_back(
                        std::to_string(*reinterpret_cast<const int32_t *>(curr_ptr)));
                break;
            case Data_Type::UINT32_LE:
                vals.emplace_back(
                        std::to_string(*reinterpret_cast<const uint32_t *>(curr_ptr)));
                break;
            case Data_Type::INT64_LE:
                vals.emplace_back(
                        std::to_string(*reinterpret_cast<const int64_t *>(curr_ptr)));
                break;
            case Data_Type::UINT64_LE:
                vals.emplace_back(
                        std::to_string(*reinterpret_cast<const uint64_t *>(curr_ptr)));
                break;
            case Data_Type::FLOAT32_LE:
                vals.emplace_back(
                        std::to_string(*reinterpret_cast<const float *>(curr_ptr)));
                break;
            case Data_Type::FLOAT64_LE:
                vals.emplace_back(
                        std::to_string(*reinterpret_cast<const double *>(curr_ptr)));
                break;
            case Data_Type::CHAR: {
                /// The characters in the type can be shorter than the
                /// number of allowed bytes, so add a .c_str() that
                /// will terminate the string at the first null.
                std::string element(reinterpret_cast<const char *>(curr_ptr),
                                    column_iter->array_size);
                vals.emplace_back(element.c_str());
            } break;
            default:
                throw std::runtime_error("Column " + colname +
                                         " has unsupported datatype.");
        }
    }
    return vals;
}
