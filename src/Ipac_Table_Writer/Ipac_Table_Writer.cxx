#include "../Ipac_Table_Writer.hxx"

#include "../Data_Type_Adjuster.hxx"
#include "../Table.hxx"

// This file implements public functions of the Ipac_Table_Writer class.

/**********************************************************/
/* Delegators to Data_Type_Adjuster-friendly analogues */
/**********************************************************/

// The IPAC table format does not support array-valued non-char columns.
// A array column of fixed size n named "col" of non-char type is formatted
// as single-valued columns named "col_0", "col_1", ... "col_{n-1}".

void tablator::Ipac_Table_Writer::write_ipac_table(const tablator::Table &table,
                                                   std::ostream &os) {
    write_ipac_table(table, os,
                     Data_Type_Adjuster(table).get_datatypes_for_writing(
                             Format::Enums::IPAC_TABLE));
}

// more to come...

/**********************************************************/
/* Helper functions */
/**********************************************************/

std::string tablator::Ipac_Table_Writer::to_ipac_string(const Data_Type &type) {
    /// Write out unsigned integers as integers for backwards compatibility
    switch (type) {
        case Data_Type::INT8_LE:
        case Data_Type::UINT8_LE:
        case Data_Type::INT16_LE:
        case Data_Type::UINT16_LE:
        case Data_Type::INT32_LE:
            return "int";
        /// Unsigned 32 bit ints do not fit in ints, so we use a long.
        case Data_Type::UINT32_LE:
        case Data_Type::INT64_LE:
        case Data_Type::UINT64_LE:
            return "long";
        case Data_Type::FLOAT32_LE:
            return "float";
        case Data_Type::FLOAT64_LE:
            return "double";
        case Data_Type::CHAR:
            return "char";
        default:
            throw std::runtime_error(
                    "Unexpected HDF5 data type in tablator::Table::to_ipac_string");
    }
}

/*******************************************************/


// This function does the right thing for columns of type UINT64_LE even if
// their active_datatype is CHAR.

std::vector<size_t> tablator::Ipac_Table_Writer::get_column_widths(const Table &table) {
    std::vector<size_t> widths;
    auto &columns = table.columns;
    auto column(std::next(columns.begin()));
    // First column is the null bitfield flags, which are not written
    // out in ipac_tables.
    widths.push_back(0);
    for (; column != columns.end(); ++column) {
        size_t header_size(
                column->name.size() +
                (column->array_size == 1
                         ? 0
                         : 1 + std::to_string(column->array_size - 1).size()));
        auto unit = column->field_properties.attributes.find("unit");
        if (unit != column->field_properties.attributes.end()) {
            header_size = std::max(header_size, unit->second.size());
        }
        if (column->type == Data_Type::CHAR) {
            // The minimum of 4 is to accomodate the length of the
            // literals 'char' and 'null'.
            widths.push_back(
                    std::max((size_t)4, std::max(header_size, column->array_size)));
        } else {
            // buffer_size = 1 (sign) + 1 (leading digit) + 1
            // (decimal) + 1 (exponent sign) + 3 (exponent) (value
            // could be e.g. uint64 as well as double, but double's
            // buffer_size is generous enough).
            const size_t buffer_size(7);
            widths.push_back(
                    std::max(header_size,
                             std::numeric_limits<double>::max_digits10 + buffer_size));
        }
    }
    return widths;
}
