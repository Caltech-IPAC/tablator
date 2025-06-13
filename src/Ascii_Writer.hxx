#pragma once

#include <iostream>

#include "Command_Line_Options.hxx"
#include "Data_Type.hxx"

namespace tablator {

class Ascii_Writer {
public:
    static constexpr const char DEFAULT_SEPARATOR = ' ';
    static constexpr const char IPAC_COLUMN_SEPARATOR = ' ';

    static void write_type_as_ascii(
            std::ostream &os, const Data_Type &type, size_t array_size,
			bool dynamic_array_flag,
            const uint8_t *data, const char &separator = DEFAULT_SEPARATOR,
            const Command_Line_Options &options = default_options);

    // Called by Ipac_Table_Writer when splitting array column into several columns.
    static void write_type_as_ascii_expand_array(
            std::ostream &os, const Data_Type &type, size_t array_size,
			bool dynamic_array_flag,
            const uint8_t *data, size_t col_width,
            const Command_Line_Options &options = default_options);

private:
    static void write_array_unit_as_ascii(
            std::ostream &os, const Data_Type &type, size_t array_size,
            const uint8_t *data, const Command_Line_Options &options = default_options);
};

// For backward compatibility
// JTODO who uses this?
#if 0
inline void write_type_as_ascii(std::ostream &os, const Data_Type &type,
                                size_t array_size, const uint8_t *data,
                                const char &separator = Ascii_Writer::DEFAULT_SEPARATOR,
                                const Command_Line_Options &options = default_options) {
  bool dynamic_array_flag = false; // JTODO
  Ascii_Writer::write_type_as_ascii(os, type, array_size, dynamic_array_flag,data, separator, 
                                             options);
}
#endif
}  // namespace tablator
