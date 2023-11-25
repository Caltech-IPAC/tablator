#pragma once

#include <iostream>

#include "Data_Type.hxx"

namespace tablator {




class Ascii_Writer {
public:
    static constexpr const char DEFAULT_SEPARATOR = ' ';
    static constexpr const char IPAC_COLUMN_SEPARATOR = ' ';

    static void write_type_as_ascii(std::ostream &os, const Data_Type &type,
                                    const size_t &array_size, const uint8_t *data,
                                    const char &separator = DEFAULT_SEPARATOR);

    // Called by Ipac_Table_Writer when splitting array column into several columns.
    static void write_type_as_ascii_expand_array(std::ostream &os,
                                                 const Data_Type &type,
                                                 const size_t &array_size,
                                                 const uint8_t *data, size_t col_width);


    static size_t get_adjusted_string_length_for_double(double dub_var);

private:
    static const std::string truncate_fishy_decimals(double dub_var);

    static std::pair<size_t, bool> get_adjusted_string_length_for_double(
            const std::string &string_var, size_t point_pos);


    static void write_array_unit_as_ascii(std::ostream &os, const Data_Type &type,
                                          const size_t &array_size,
                                          const uint8_t *data);
};

// For backward compatibility
inline void write_type_as_ascii(
        std::ostream &os, const Data_Type &type, const size_t &array_size,
        const uint8_t *data, const char &separator = Ascii_Writer::DEFAULT_SEPARATOR) {
    return Ascii_Writer::write_type_as_ascii(os, type, array_size, data, separator);
}

}  // namespace tablator
