#pragma once

#include <iostream>

#include "Data_Type.hxx"

namespace tablator {

static constexpr const char DEFAULT_SEPARATOR = ' ';
static constexpr const char IPAC_COLUMN_SEPARATOR = ' ';

void write_array_unit_as_ascii(std::ostream &os, const Data_Type &type,
                               const size_t &array_size, const uint8_t *data);

void write_type_as_ascii(std::ostream &os, const Data_Type &type,
                         const size_t &array_size, const uint8_t *data,
                         const char &separator = DEFAULT_SEPARATOR);

// Called when splitting array column into several columns for IPAC_TABLE format.
void write_type_as_ascii(std::ostream &os, const Data_Type &type,
                         const size_t &array_size, const uint8_t *data,
                         size_t col_width);

}  // namespace tablator
