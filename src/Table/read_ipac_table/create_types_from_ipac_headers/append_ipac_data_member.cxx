#include "../../../Table.hxx"

void tablator::Table::append_ipac_data_member(std::vector<Column> &columns,
                                              std::vector<size_t> &offsets,
                                              const std::string &name,
                                              const std::string &data_type,
                                              const size_t &num_elements) {
    /// We use "string".compare (0,size,t) because it is valid to
    /// abbreviate the type name
    std::string type_str = boost::to_lower_copy(data_type);
    const size_t type_str_len(type_str.size());

    if (std::string("boolean").compare(0, type_str_len, type_str) == 0) {
        tablator::append_column(columns, offsets, name, Data_Type::INT8_LE);
    } else if (std::string("byte").compare(0, type_str_len, type_str) == 0) {
        tablator::append_column(columns, offsets, name, Data_Type::UINT8_LE);
    } else if (std::string("short").compare(0, type_str_len, type_str) == 0) {
        tablator::append_column(columns, offsets, name, Data_Type::INT16_LE);
    } else if (std::string("ushort").compare(0, type_str_len, type_str) == 0) {
        tablator::append_column(columns, offsets, name, Data_Type::UINT16_LE);
    } else if (std::string("int").compare(0, type_str_len, type_str) == 0) {
        tablator::append_column(columns, offsets, name, Data_Type::INT32_LE);
    } else if (std::string("uint").compare(0, type_str_len, type_str) == 0) {
        tablator::append_column(columns, offsets, name, Data_Type::UINT32_LE);
    } else if (std::string("long").compare(0, type_str_len, type_str) == 0) {
        tablator::append_column(columns, offsets, name, Data_Type::INT64_LE);
    } else if (std::string("ulong").compare(0, type_str_len, type_str) == 0) {
        tablator::append_column(columns, offsets, name, Data_Type::UINT64_LE);
    } else if (std::string("float").compare(0, type_str_len, type_str) == 0) {
        tablator::append_column(columns, offsets, name, Data_Type::FLOAT32_LE);
    } else if (std::string("double").compare(0, type_str_len, type_str) == 0 ||
               std::string("real").compare(0, type_str_len, type_str) == 0) {
        tablator::append_column(columns, offsets, name, Data_Type::FLOAT64_LE);
    } else if (std::string("char").compare(0, type_str_len, type_str) == 0 ||
               std::string("date").compare(0, type_str_len, type_str) == 0) {
        tablator::append_column(columns, offsets, name, Data_Type::CHAR, num_elements);
    } else {
        throw std::runtime_error("Unknown data type in IPAC table: " + data_type);
    }
}
