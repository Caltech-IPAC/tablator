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
                "Data type uint64 is not supported when writing fits data.");
    }

    throw std::runtime_error("Unknown data type " + to_string(datatype_for_writing) +
                             " when writing fits data ");
}

/**********************************************************/

// Format string is array_size followed by fits_type_code.

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
    static int8_t null_int8 = tablator::get_null<int8_t>();
    static uint8_t null_uint8 = tablator::get_null<uint8_t>();
    static int16_t null_int16 = tablator::get_null<int16_t>();
    static uint16_t null_uint16 = tablator::get_null<uint16_t>();
    static int32_t null_int32 = tablator::get_null<int32_t>();
    static uint32_t null_uint32 = tablator::get_null<uint32_t>();
    static int64_t null_int64 = tablator::get_null<int64_t>();
    static uint64_t null_uint64 = tablator::get_null<uint64_t>();
    static float null_float = tablator::get_null<float>();
    static double null_double = tablator::get_null<double>();
    static char null_char = tablator::get_null<char>();

    static const std::map<tablator::Data_Type, char *> datatype_lookup = {
            {tablator::Data_Type::INT8_LE, reinterpret_cast<char *>(&null_int8)},
            {tablator::Data_Type::UINT8_LE, reinterpret_cast<char *>(&null_uint8)},
            {tablator::Data_Type::INT16_LE, reinterpret_cast<char *>(&null_int16)},
            {tablator::Data_Type::UINT16_LE, reinterpret_cast<char *>(&null_uint16)},
            {tablator::Data_Type::INT32_LE, reinterpret_cast<char *>(&null_int32)},
            {tablator::Data_Type::UINT32_LE, reinterpret_cast<char *>(&null_uint32)},
            {tablator::Data_Type::INT64_LE, reinterpret_cast<char *>(&null_int64)},
            {tablator::Data_Type::UINT64_LE, reinterpret_cast<char *>(&null_uint64)},
            {tablator::Data_Type::FLOAT32_LE, reinterpret_cast<char *>(&null_float)},
            {tablator::Data_Type::FLOAT64_LE, reinterpret_cast<char *>(&null_double)},
            {tablator::Data_Type::CHAR, reinterpret_cast<char *>(&null_char)}};

    const auto iter = datatype_lookup.find(datatype_for_writing);
    if (iter != datatype_lookup.end()) {
        return iter->second;
    }
    return NULL;
}

/**********************************************************/

// FITS index values, including fits_col_idx, are 1-based.
void write_null_given_column_and_row(fitsfile *fits_file, size_t fits_col_idx,
                                     size_t fits_row_idx, size_t array_size) {
    int status = 0;
    fits_write_col_null(fits_file, fits_col_idx, fits_row_idx, 1 /* firstelem */,
                        array_size, &status);
    if (status != 0) {
        throw CCfits::FitsError(status);
    }
}

/**********************************************************/

template <typename data_type>
void write_element_given_column_and_row(fitsfile *fits_file, int fits_type,
                                        size_t fits_col_idx, size_t fits_row_idx,
                                        uint8_t *data_ptr, size_t array_size) {
    int status = 0;
    fits_write_col(fits_file, fits_type, fits_col_idx, fits_row_idx, 1, array_size,
                   reinterpret_cast<data_type *>(data_ptr), &status);

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
    // Load lists of column-level data needed to create FITS table.
    // ****************************************************************/

    // According to the FITS standard, the value of the TTYPEn keyword is
    // the name of the n-th field/column.

    std::vector<const char *> ttype;
    std::vector<const char *> tunit;
    std::vector<const char *> tform;

    // Helper used to populate tform.
    std::vector<std::string> fits_format_strings;

    const auto &columns = get_columns();

    // Skip null_bitfield_flags column (col_idx == 0).  Corresponding
    // columns of the tablator and FITS tables have the same (1-based)
    // index.
    for (size_t col_idx = 1; col_idx < columns.size(); ++col_idx) {
        auto &column = columns[col_idx];

        ttype.push_back(column.get_name().c_str());

        const auto col_attrs = column.get_field_properties().get_attributes();
        auto unit = col_attrs.find(UNIT);
        if (unit == col_attrs.end()) {
            tunit.push_back("");
        } else {
            tunit.push_back(unit->second.c_str());
        }

        auto fits_format_str =
                get_fits_format(datatypes_for_writing[col_idx], column.get_type(),
                                column.get_array_size());

        // We store the fits_format_str string in a vector which will
        // survive past the end of this block, because otherwise the
        // string, allocated during this block, would be deallocated
        // when the block ends.  We then retrieve the char array
        // underlying the vector's copy of the fits_format_str string
        // and load it into the tform vector for the call to
        // fits_create_tbl().

        fits_format_strings.emplace_back(fits_format_str);

        tform.push_back(fits_format_strings.back().c_str());
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
        // write_null_given_column_and_row() for that datatype) causes
        // errors, so skipping that datatype also. 19Oct23.

        if (datatype_for_writing != Data_Type::INT8_LE &&
            datatype_for_writing != Data_Type::FLOAT32_LE &&
            datatype_for_writing != Data_Type::FLOAT64_LE) {
            fits_write_key(fits_file, get_fits_datatype(datatype_for_writing),
                           key_ss.str().c_str(), get_ptr_to_null(datatype_for_writing),
                           (char *)NULL /* comment */, &status);
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
    const size_t number_of_rows(num_rows());

    // Retrieve table's data pointer.
    const uint8_t *data_start_ptr = get_data().data();

    // Cast away const, as FITS functions require.
    uint8_t *row_start_ptr = const_cast<uint8_t *>(data_start_ptr);

    // FITS row index is 1-based.
    for (size_t fits_row_idx = 1; fits_row_idx <= number_of_rows;
         ++fits_row_idx, row_start_ptr += row_size()) {
        // Skip null_bitfield_flags column.  Corresponding
        // columns of the tablator and FITS tables have the same (1-based)
        // index.
        for (size_t col_idx = 1; col_idx < columns.size(); ++col_idx) {
            auto &column = columns[col_idx];

            uint8_t *curr_data_ptr = row_start_ptr + offsets[col_idx];
            size_t curr_row_start_offset = (fits_row_idx - 1) * row_size();
            Data_Type datatype_for_writing = datatypes_for_writing[col_idx];

            size_t array_size = column.get_array_size();
            bool null_flag_is_set = is_null(curr_row_start_offset, col_idx);

            bool all_or_nothing_null =
                    ((array_size == 1) || (datatype_for_writing == Data_Type::CHAR) ||
                     (datatype_for_writing == Data_Type::INT8_LE));

            if (all_or_nothing_null && null_flag_is_set) {
                // No need to inspect individual array elements.

                // Since our version of FITS doesn't support
                // UINT64_LE, columns of that type must be written as
                // CHAR, and their array_size must be adjusted
                // accordingly before we call FITS write functions.
                size_t active_array_size = array_size;
                if (column.get_type() == Data_Type::UINT64_LE) {
                    active_array_size =
                            Data_Type_Adjuster::get_char_array_size_for_uint64_col(
                                    array_size);
                }
                write_null_given_column_and_row(fits_file, col_idx, fits_row_idx,
                                                active_array_size);
                continue;
            }
            switch (datatype_for_writing) {
                case Data_Type::INT8_LE: {
                    write_element_given_column_and_row<bool>(fits_file, TLOGICAL,
                                                             col_idx, fits_row_idx,
                                                             curr_data_ptr, array_size);
                } break;
                case Data_Type::UINT8_LE: {
                    write_element_given_column_and_row<uint8_t>(
                            fits_file, TBYTE, col_idx, fits_row_idx, curr_data_ptr,
                            array_size);
                } break;
                case Data_Type::INT16_LE: {
                    write_element_given_column_and_row<int16_t>(
                            fits_file, TSHORT, col_idx, fits_row_idx, curr_data_ptr,
                            array_size);
                } break;
                case Data_Type::UINT16_LE: {
                    write_element_given_column_and_row<uint16_t>(
                            fits_file, TUSHORT, col_idx, fits_row_idx, curr_data_ptr,
                            array_size);
                } break;
                case Data_Type::INT32_LE: {
                    write_element_given_column_and_row<int32_t>(
                            fits_file, TINT, col_idx, fits_row_idx, curr_data_ptr,
                            array_size);
                } break;
                case Data_Type::UINT32_LE: {
                    write_element_given_column_and_row<uint32_t>(
                            fits_file, TUINT, col_idx, fits_row_idx, curr_data_ptr,
                            array_size);
                } break;
                case Data_Type::INT64_LE: {
                    write_element_given_column_and_row<int64_t>(
                            fits_file, TLONGLONG, col_idx, fits_row_idx, curr_data_ptr,
                            array_size);
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
                    write_element_given_column_and_row<float>(
                            fits_file, TFLOAT, col_idx, fits_row_idx, curr_data_ptr,
                            array_size);
                } break;
                case Data_Type::FLOAT64_LE: {
                    write_element_given_column_and_row<double>(
                            fits_file, TDOUBLE, col_idx, fits_row_idx, curr_data_ptr,
                            array_size);
                } break;
                case Data_Type::CHAR: {
                    // Column's true data_type might be either CHAR or UINT64.
                    std::string temp_string;
                    if (column.get_type() == Data_Type::CHAR) {
                        // Really a CHAR column, not a rewritten UINT64_LE
                        temp_string.assign(
                                reinterpret_cast<const char *>(curr_data_ptr),
                                offsets[col_idx + 1] - offsets[col_idx]);
                    } else {
                        // Really UINT64_LE
                        auto curr_ptr = curr_data_ptr;
                        for (size_t j = 0; j < array_size; ++j) {
                            if (j > 0) {
                                temp_string.append(" ");
                            }
                            temp_string.append(std::to_string(
                                    *reinterpret_cast<const uint64_t *>(curr_ptr)));
                            curr_ptr += sizeof(uint64_t);
                        }
                    }
                    char *temp_chars = const_cast<char *>(temp_string.c_str());
                    // Tell FITS to write the entire char array as a single string.
                    fits_write_col(fits_file, TSTRING, col_idx, fits_row_idx, 1, 1,
                                   &temp_chars, &status);
                    if (status != 0) {
                        throw CCfits::FitsError(status);
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
}
