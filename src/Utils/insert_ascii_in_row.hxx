#include "../Row.hxx"

namespace tablator {
void insert_ascii_in_row(const Data_Type &data_type, const size_t &array_size,
                         const size_t &column, const std::string &element,
                         const size_t &offset, const size_t &offset_end,
                         Row &row_string);
}
