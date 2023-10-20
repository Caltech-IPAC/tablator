#include "../Table.hxx"

#include <longnam.h>
#include <CCfits/CCfits>
#include <algorithm>
#include <type_traits>

#include "../Data_Type_Adjuster.hxx"
#include "../Utils/Vector_Utils.hxx"
#include "../to_string.hxx"

// Note: Descriptions and field-level attributes get lost in conversion to and from
// fits.

namespace {

char get_fits_type_code(tablator::Data_Type datatype_for_writing) {
    static const std::map<tablator::Data_Type, char> code_lookup = {
            {tablator::Data_Type::INT8_LE, 'L'},
            {tablator::Data_Type::UINT8_LE, 'B'},
            {tablator::Data_Type::INT16_LE, 'I'},
            {tablator::Data_Type::UINT16_LE, 'U'},
            {tablator::Data_Type::INT32_LE, 'J'},
            {tablator::Data_Type::UINT32_LE, 'V'},
            {tablator::Data_Type::INT64_LE, 'K'},

            // Not supported by our cfitsio version.
            // {tablator::Data_Type::UINT64_LE, 'W'},

            {tablator::Data_Type::FLOAT32_LE, 'E'},
            {tablator::Data_Type::FLOAT64_LE, 'D'},
            {tablator::Data_Type::CHAR, 'A'}};


    const auto iter = code_lookup.find(datatype_for_writing);
    if (iter != code_lookup.end()) {
        return iter->second;
    }

    // FIXME: Take this out when we update to a cfitsio version that supports this type.
    if (datatype_for_writing == tablator::Data_Type::UINT64_LE) {
        throw std::runtime_error(
                "Data type uint64 is unsupported when writing fits data.");
    }

    throw std::runtime_error("Unknown data type " + to_string(datatype_for_writing) +
                             " when writing fits data ");
}

/**********************************************************/

// Format string is fits_type_code followed by array_size.

std::string get_fits_format(tablator::Data_Type datatype_for_writing,
                            tablator::Data_Type raw_datatype, size_t array_size) {
    char fits_type = get_fits_type_code(datatype_for_writing);

    // Set default and adjust for columns whose ulong values are slated to be written
    // as char strings.
    std::string array_size_str(std::to_string(array_size));
    if (fits_type == 'A' && raw_datatype == tablator::Data_Type::UINT64_LE) {
        array_size_str.assign(std::to_string(
                tablator::Data_Type_Adjuster::get_char_array_size_for_uint64_col(
                        array_size)));
    }
    return array_size_str + fits_type;
}

/**********************************************************/

int get_fits_datatype(tablator::Data_Type datatype_for_writing) {
    static const std::map<tablator::Data_Type, int> datatype_lookup = {
            {tablator::Data_Type::INT8_LE, TLOGICAL},
            {tablator::Data_Type::UINT8_LE, TBYTE},
            {tablator::Data_Type::INT16_LE, TSHORT},
            {tablator::Data_Type::UINT16_LE, TUSHORT},
            {tablator::Data_Type::INT32_LE, TINT},
            {tablator::Data_Type::UINT32_LE, TUINT},
            {tablator::Data_Type::INT64_LE, TLONG},
            {tablator::Data_Type::UINT64_LE, TULONG},
            {tablator::Data_Type::FLOAT32_LE, TFLOAT},
            {tablator::Data_Type::FLOAT64_LE, TDOUBLE},
            {tablator::Data_Type::CHAR, TSTRING},
    };

    const auto iter = datatype_lookup.find(datatype_for_writing);
    if (iter != datatype_lookup.end()) {
        return iter->second;
    }
    throw std::runtime_error(
            "Unknown data type when writing fits "
            "data: " +
            to_string(datatype_for_writing));
}

/**********************************************************/

char *get_ptr_to_null(tablator::Data_Type datatype_for_writing) {
    static std::map<tablator::Data_Type, int> datatype_lookup = {
            {tablator::Data_Type::INT8_LE, tablator::get_null<int8_t>()},
            {tablator::Data_Type::UINT8_LE, tablator::get_null<uint8_t>()},
            {tablator::Data_Type::INT16_LE, tablator::get_null<int16_t>()},
            {tablator::Data_Type::UINT16_LE, tablator::get_null<uint16_t>()},
            {tablator::Data_Type::INT32_LE, tablator::get_null<int32_t>()},
            {tablator::Data_Type::UINT32_LE, tablator::get_null<uint32_t>()},
            {tablator::Data_Type::INT64_LE, tablator::get_null<int64_t>()},
            {tablator::Data_Type::UINT64_LE, tablator::get_null<uint64_t>()},
            {tablator::Data_Type::FLOAT32_LE, tablator::get_null<float>()},
            {tablator::Data_Type::FLOAT64_LE, tablator::get_null<double>()},
            {tablator::Data_Type::CHAR, tablator::get_null<char>()}};

    const auto iter = datatype_lookup.find(datatype_for_writing);
    if (iter != datatype_lookup.end()) {
        return reinterpret_cast<char *>(&(iter->second));
    }
    return NULL;
}

/**********************************************************/

template <typename data_type>
void write_column(fitsfile *fits_file, int fits_type, int col_idx, const uint8_t *data,
                  hsize_t array_size, size_t row_idx) {
    int status = 0;
    fits_write_col(fits_file, fits_type, col_idx, row_idx, 1, array_size,
                   reinterpret_cast<data_type *>(const_cast<uint8_t *>(data)), &status);
    if (status != 0) {
        throw CCfits::FitsError(status);
    }
}

/**********************************************************/

void write_column_null(fitsfile *fits_file, int col_idx, hsize_t array_size,
                       size_t row_idx) {
    int status = 0;
    fits_write_col_null(fits_file, col_idx, row_idx, 1, array_size, &status);
    if (status != 0) {
        throw CCfits::FitsError(status);
    }
}


}  // namespace

/**********************************************************/
/**********************************************************/


void tablator::Table::write_fits(std::ostream &os) const {
    write_fits(os, Data_Type_Adjuster(*this).get_datatypes_for_writing(
                           Format::Enums::FITS));
}

/**********************************************************/

void tablator::Table::write_fits(const boost::filesystem::path &filename) const {
    write_fits(filename, Data_Type_Adjuster(*this).get_datatypes_for_writing(
                                 Format::Enums::FITS));
}

/**********************************************************/

void tablator::Table::write_fits(fitsfile *fits_file) const {
    write_fits(fits_file, Data_Type_Adjuster(*this).get_datatypes_for_writing(
                                  Format::Enums::FITS));
}

/**********************************************************/

void tablator::Table::write_fits(
        const boost::filesystem::path &filename,
        const std::vector<Data_Type> &datatypes_for_writing) const {
    // Remove the file because cfitsio will fail if the file still
    // exists.
    boost::filesystem::remove(filename);
    int status = 0;
    fitsfile *fits_file;
    fits_create_file(&fits_file, filename.c_str(), &status);
    if (status != 0) throw CCfits::FitsError(status);

    write_fits(fits_file, datatypes_for_writing);
    fits_close_file(fits_file, &status);
    if (status != 0) {
        throw CCfits::FitsError(status);
    }
}

/**********************************************************/

void tablator::Table::write_fits(
        std::ostream &os, const std::vector<Data_Type> &datatypes_for_writing) const {
    size_t buffer_size(2880);
    void *buffer = malloc(buffer_size);
    try {
        fitsfile *fits_file, *reopen_file;
        int status = 0;
        fits_create_memfile(&fits_file, &buffer, &buffer_size, 0, std::realloc,
                            &status);
        if (status != 0) {
            throw CCfits::FitsError(status);
        }
        write_fits(fits_file, datatypes_for_writing);

        // I have to reopen the file because otherwise fits_close_file
        // will delete the memory
        fits_reopen_file(fits_file, &reopen_file, &status);
        if (status != 0) {
            throw CCfits::FitsError(status);
        }

        fits_close_file(fits_file, &status);
        if (status != 0) {
            throw CCfits::FitsError(status);
        }

        os.write(static_cast<const char *>(buffer), buffer_size);
        // This also free's buffer.
        fits_close_file(reopen_file, &status);
        if (status != 0) {
            throw CCfits::FitsError(status);
        }
    } catch (...) {
        free(buffer);
        throw;
    }
}

/**********************************************************/
/**********************************************************/

// We separate out the write_fits implementation so that we can
// insert a read of the memory image so that we can write to a
// stream.

void tablator::Table::write_fits(
        fitsfile *fits_file,
        const std::vector<Data_Type> &datatypes_for_writing) const {
    int status = 0;

    //****************************************************************/
    // Load per-column lists of values needed to create FITS table.
    //****************************************************************/

    std::vector<string> fits_types;

    // According to the FITS standard, the TTYPEn keyword shall have a value
    // equal to the n-th field name.
    std::vector<const char *> ttype, tunit;

    const auto &columns = get_columns();

    // Skip null_bitfield_flags column.
    for (size_t column_idx = 1; column_idx < columns.size(); ++column_idx) {
        auto &column = columns[column_idx];

        ttype.push_back(column.get_name().c_str());

        const auto col_attrs = column.get_field_properties().get_attributes();
        auto unit = col_attrs.find(UNIT);
        if (unit == col_attrs.end()) {
            tunit.push_back("");
        } else {
            tunit.push_back(unit->second.c_str());
        }


        // We have to store the fits_type in a string and then set the
        // c_str(), because otherwise the string will get deallocated.
        auto fits_type_str =
                get_fits_format(datatypes_for_writing[column_idx], column.get_type(),
                                column.get_array_size());
        fits_types.push_back(fits_type_str);
    }

    std::vector<const char *> tform;
    for (auto &t : fits_types) {
        tform.push_back(t.c_str());
    }

    //********************/
    // Create FITS table.
    //********************/

    fits_create_tbl(fits_file, BINARY_TBL, 0, ttype.size(),
                    const_cast<char **>(ttype.data()),
                    const_cast<char **>(tform.data()),
                    const_cast<char **>(tunit.data()), "Table", &status);
    if (status != 0) throw CCfits::FitsError(status);

    // Warn users that this table uses the Long String Keyword convention.
    fits_write_key_longwarn(fits_file, &status);
    if (status != 0) throw CCfits::FitsError(status);

    //*************************************/
    // Set null values for numeric columns.
    //*************************************/

    for (size_t col_idx = 1; col_idx < columns.size(); ++col_idx) {
        auto &column = columns[col_idx];
        auto datatype_for_writing = datatypes_for_writing[col_idx];

        std::stringstream key_ss;
        key_ss << "TNULL" << col_idx;

        // FITS uses its own default null value for float and double.
        // Using our own get_ptr_to_null() for those datatypes leads
        // to datatype conversion overflow error.

        // Adding TNULLn for INT8_LE (even if not calling
        // write_column_null() for that datatype) causes errors, so
        // skipping that datatype also. 19Oct23.

        if (datatype_for_writing != Data_Type::INT8_LE &&
            datatype_for_writing != Data_Type::FLOAT32_LE &&
            datatype_for_writing != Data_Type::FLOAT64_LE) {
            fits_write_key(fits_file, get_fits_datatype(datatype_for_writing),
                           key_ss.str().c_str(), get_ptr_to_null(datatype_for_writing),
                           (char *)NULL, &status);
        }

        if (status != 0) {
            throw CCfits::FitsError(status);
        }
    }

    assert(get_resource_elements().size() > 0);
    const auto combined_labeled_attributes = combine_attributes_all_levels();

    //*********************************************************************/
    // Combine and write properties and trailing info for all levels
    // at which they are defined.
    //*********************************************************************/

    auto combined_labeled_properties = combine_labeled_properties_all_levels();
    const auto combined_labeled_trailing_info_lists =
            combine_trailing_info_lists_all_levels();
    combined_labeled_properties.insert(combined_labeled_properties.end(),
                                       combined_labeled_trailing_info_lists.begin(),
                                       combined_labeled_trailing_info_lists.end());

    combined_labeled_properties.insert(combined_labeled_properties.end(),
                                       combined_labeled_attributes.begin(),
                                       combined_labeled_attributes.end());


    // Labeled_properties are stored as keywords whose values are strings
    // of the form

    // label + DOT + XMLATTR_DOT + ATTR_IRSA_VALUE : value  (for prop.value_)
    // label + DOT + XMLATTR_DOT + attrname : attrvalue  (for elements of
    // prop.attributes_)

    // where label includes value of prop's ATTR_NAME attribute for
    // uniqueness.
    for (const auto &label_and_prop : combined_labeled_properties) {
        std::string label = label_and_prop.first;
        const auto &prop = label_and_prop.second;
        std::string value(prop.get_value());
        std::string comment;

        // Base for the (distinct) keywords with which we will store each of prop's
        // value and attributes.
        auto keyword_base = label + DOT;
        const auto &attributes = prop.get_attributes();
        auto name_iter = attributes.find(ATTR_NAME);

        // FITS wants keywords to be unique, but there could be many
        // INFO elements having attributes with the same names.  If label ends in
        // INFO, we include in each keyword the value of the relevant NAME
        // attribute as well as the name of the attribute whose value
        // is being stored.

        // (INFO elements are assumed to have NAME attributes.)
        if (boost::ends_with(label, INFO)) {
            if (name_iter == attributes.end() || (name_iter->second).empty()) {
                // Shouldn't happen!
            } else {
                keyword_base = label + DOT + name_iter->second + DOT;
            }
        }

        if (!boost::ends_with(keyword_base, XMLATTR_DOT)) {
            keyword_base += XMLATTR_DOT;
        }

        if (!value.empty()) {
            fits_write_key_longstr(fits_file, (keyword_base + ATTR_IRSA_VALUE).c_str(),
                                   value.c_str(), comment.c_str(), &status);
        }

        for (auto &attr : prop.get_attributes()) {
#ifdef FIXED_FITS_COMMENT
            // This step prepares us to store the comment in a special FITS way,
            // but as of 13Nov20, comments will be truncated or omitted if
            // comment.size() + value.size() > 65.
            if (attr.first == "comment") {
                comment.assign(attr.second);
            } else
#endif
                fits_write_key_longstr(fits_file, (keyword_base + attr.first).c_str(),
                                       attr.second.c_str(), comment.c_str(), &status);
            if (status != 0) {
                throw CCfits::FitsError(status);
            }
        }
    }

    //*******************/
    // Write table data.
    //*******************/

    const auto &offsets = get_offsets();
    const uint8_t *row_pointer(get_data().data());
    const size_t number_of_rows(num_rows());
    for (size_t row_idx = 1; row_idx <= number_of_rows; ++row_idx) {
        // Skip null_bitfield_flags column.
        for (size_t col_idx = 1; col_idx < columns.size(); ++col_idx) {
            auto &column = columns[col_idx];
            const uint8_t *offset_data = row_pointer + offsets[col_idx];
            size_t curr_row_start_offset = (row_idx - 1) * row_size();
            size_t array_size = column.get_array_size();

            if (is_null(curr_row_start_offset, col_idx)) {
                write_column_null(fits_file, col_idx, array_size, row_idx);
            } else {
                switch (datatypes_for_writing[col_idx]) {
                    case Data_Type::INT8_LE: {
                        write_column<bool>(fits_file, TLOGICAL, col_idx, offset_data,
                                           array_size, row_idx);
                    } break;
                    case Data_Type::UINT8_LE: {
                        write_column<uint8_t>(fits_file, TBYTE, col_idx, offset_data,
                                              array_size, row_idx);
                    } break;
                    case Data_Type::INT16_LE: {
                        write_column<int16_t>(fits_file, TSHORT, col_idx, offset_data,
                                              array_size, row_idx);
                    } break;
                    case Data_Type::UINT16_LE: {
                        write_column<uint16_t>(fits_file, TUSHORT, col_idx, offset_data,
                                               array_size, row_idx);
                    } break;
                    case Data_Type::INT32_LE: {
                        write_column<int32_t>(fits_file, TINT, col_idx, offset_data,
                                              array_size, row_idx);
                    } break;
                    case Data_Type::UINT32_LE: {
                        write_column<uint32_t>(fits_file, TUINT, col_idx, offset_data,
                                               array_size, row_idx);
                    } break;
                    case Data_Type::INT64_LE: {
                        write_column<int64_t>(fits_file, TLONGLONG, col_idx,
                                              offset_data, array_size, row_idx);
                    } break;
                    case Data_Type::UINT64_LE: {
                        // Our version of cfitsio doesn't support TULONGLONG.  19Oct23
                        throw std::runtime_error(
                                "Unsupported uint64 data type when writing fits "
                                "data: , column #" +
                                std::to_string(col_idx) + ", original data type " +
                                to_string(column.get_type()));
                    } break;
                    case Data_Type::FLOAT32_LE: {
                        write_column<float>(fits_file, TFLOAT, col_idx, offset_data,
                                            array_size, row_idx);
                    } break;
                    case Data_Type::FLOAT64_LE: {
                        write_column<double>(fits_file, TDOUBLE, col_idx, offset_data,
                                             array_size, row_idx);
                    } break;
                    case Data_Type::CHAR: {
                        // Column's true data_type might be either CHAR or UINT64.
                        if (is_null(curr_row_start_offset, col_idx)) {
                            size_t active_array_size =
                                    (column.get_type() == Data_Type::CHAR)
                                            ? array_size
                                            : Data_Type_Adjuster::
                                                      get_char_array_size_for_uint64_col(
                                                              array_size);
                            write_column_null(fits_file, col_idx, active_array_size,
                                              row_idx);

                        } else {
                            std::string temp_string;
                            if (column.get_type() == Data_Type::CHAR) {
                                // Really a CHAR column, not a rewritten UINT64_LE
                                temp_string.assign(
                                        reinterpret_cast<const char *>(offset_data),
                                        offsets[col_idx + 1] - offsets[col_idx]);
                            } else {
                                // Really UINT64_LE
                                auto curr_ptr = offset_data;
                                for (size_t j = 0; j < array_size; ++j) {
                                    if (j > 0) {
                                        temp_string.append(" ");
                                    }
                                    temp_string.append(std::to_string(
                                            *reinterpret_cast<const uint64_t *>(
                                                    curr_ptr)));
                                    curr_ptr += sizeof(uint64_t);
                                }
                            }
                            char *temp_chars = const_cast<char *>(temp_string.c_str());
                            fits_write_col(fits_file, TSTRING, col_idx, row_idx, 1, 1,
                                           &temp_chars, &status);
                            if (status != 0) {
                                throw CCfits::FitsError(status);
                            }
                        }
                    } break;
                    default:
                        throw std::runtime_error(
                                "Unknown data type when writing fits "
                                "data: " +
                                to_string(column.get_type()));
                        break;
                }
            }
        }
        row_pointer += row_size();
    }
}
