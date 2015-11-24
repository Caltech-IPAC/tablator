#include "../Row.hxx"

namespace tablator
{
void insert_ascii_in_row (const H5::PredType &type,
                          const std::string &element,
                          const size_t &column,
                          const std::vector<size_t> &offsets,
                          Row &row_string);
}
