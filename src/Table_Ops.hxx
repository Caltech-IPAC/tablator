#pragma once

#include "Table.hxx"

namespace tablator {

Table combine_tables(const Table &src1, const Table &src2);
Table add_counter_column(const Table &src_table, const std::string &col_name);

}  // namespace tablator
