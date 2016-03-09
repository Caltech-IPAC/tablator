#include "../Row.hxx"

namespace tablator
{
void insert_ascii_in_row (const H5::DataType &type, const size_t &column,
                          const std::string &element, const size_t &offset,
                          const size_t &offset_end, Row &row_string);
}
