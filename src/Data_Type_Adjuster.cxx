#include "Data_Type_Adjuster.hxx"
#include "Data_Type.hxx"


// The conversion of uint64-valued columns to char in IPAC table
// format is not currently supported for columns whose values are arrays of size
// > 1.

// Note: the FITS workaround should be removed when we update to a
// sufficiently recent version of CFITSIO.
std::vector<tablator::Data_Type>
tablator::Data_Type_Adjuster::get_datatypes_for_writing(
        const Format::Enums &enum_format) const {
    auto columns = table_.columns;
    std::vector<tablator::Data_Type> adjusted_datatypes;
    adjusted_datatypes.reserve(columns.size());
    bool format_ipac = (enum_format == Format::Enums::IPAC_TABLE);
    bool format_votable = (enum_format == Format::Enums::VOTABLE);
    bool format_fits = (enum_format == Format::Enums::FITS);
    bool adjust_uint64 = (format_ipac || format_votable || format_fits);
    for (size_t col = 0; col <= columns.size(); ++col) {
        auto orig_datatype = columns[col].type;
        auto adjusted_datatype = orig_datatype;  // set default and adjust
        if ((orig_datatype == tablator::Data_Type::UINT64_LE) && adjust_uint64) {
            if (contains_large_uint64_val(col)) {
                if (format_ipac) {
                    auto array_size = columns[col].array_size;
                    if (array_size > 1) {
                        throw std::runtime_error(
                                "Tables with array-valued columns of type uint64 "
                                "containing large values cannot currently be written "
                                "in IPAC format.");
                    }
                }
                adjusted_datatype = tablator::Data_Type::CHAR;
            } else {
                adjusted_datatype = tablator::Data_Type::INT64_LE;
            }
        } else if (format_votable) {  // adjust all unsigned types
            if (orig_datatype == tablator::Data_Type::UINT32_LE) {
                adjusted_datatype = tablator::Data_Type::INT64_LE;
            } else if (orig_datatype == tablator::Data_Type::UINT16_LE) {
                adjusted_datatype = tablator::Data_Type::INT32_LE;
            }
        }
        adjusted_datatypes.emplace_back(adjusted_datatype);
    }

    return adjusted_datatypes;
}

bool tablator::Data_Type_Adjuster::contains_large_uint64_val(size_t col) const {
    for (size_t table_row_offset(0), row(0); table_row_offset < table_.data.size();
         table_row_offset += table_.row_size(), ++row) {
        auto curr_ptr = table_.data.data() + table_row_offset + table_.offsets[col];
        auto array_size = table_.columns[col].array_size;
        for (size_t j = 0; j < array_size; ++j) {
            uint64_t val = *reinterpret_cast<const uint64_t *>(curr_ptr);
            if (val > std::numeric_limits<int64_t>::max()) {
                return true;
            }
            curr_ptr += sizeof(uint64_t);
        }
    }
    return false;
}
