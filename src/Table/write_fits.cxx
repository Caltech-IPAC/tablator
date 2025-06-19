#include <longnam.h>

#include <CCfits/CCfits>
#include <algorithm>
#include <type_traits>

#include "../Data_Type_Adjuster.hxx"
#include "../Table.hxx"
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

// Format string of fixed-length array is array_size followed by fits_type_code.

// JTODO
// For the future (our version of CCFits doesn't support this):
// Per
// https://docs.astropy.org/en/stable/io/fits/usage/unfamiliar.html#variable-length-array-tables,
// format string of variable-length (dynamic) array is of the form Pt(max):

// The data type specification (i.e., the value of the TFORM
// keyword) uses an extra letter ‘P’ (or ‘Q’) and the format is:
//  rPt(max)
// where r may be 0 or 1 (typically omitted, as it is not applicable
// to variable length arrays), t is one of the letter codes for
// basic data types (L, B, I, J, etc.; currently, the X format is
// not supported for variable length array field in astropy), and
// max is the maximum number of elements of any array in the
// column. So, for a variable length field of int16, the
// corresponding format spec is, for example, ‘PJ(100)’.


std::string get_fits_format(tablator::Data_Type datatype_for_writing,
                            tablator::Data_Type raw_datatype, size_t array_size
#ifdef FUTURE
                            ,
                            bool dynamic_array_flag
#endif
) {

    char fits_type = get_fits_type_code(datatype_for_writing);

    // Set defaults and adjust for columns whose ulong values are slated to be written
    // as char strings.
#ifdef FUTURE
    bool variable_fits_array_flag = dynamic_array_flag;
    std::string array_size_str(std::to_string(array_size));

    if (fits_type == 'A' && raw_datatype == tablator::Data_Type::UINT64_LE) {
        array_size_str.assign(std::to_string(
                tablator::Data_Type_Adjuster::get_char_array_size_for_uint64_col(
                        array_size)));
        variable_fits_array_flag = true;
    }
    if (variable_fits_array_flag) {
        std::stringstream format_ss;
        format_ss << "P" << fits_type << "(" << array_size_str << ")";
        return format_ss.str();
    }
#else
    std::string array_size_str(std::to_string(array_size));

    if (fits_type == 'A' && raw_datatype == tablator::Data_Type::UINT64_LE) {
        array_size_str.assign(std::to_string(
                tablator::Data_Type_Adjuster::get_char_array_size_for_uint64_col(
                        array_size)));
    }
#endif
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

// **** Warning: For a given key-value pair, FITS is apparently
// willing to support either a long key or a long value, but not
// both. ****

// The function write_tablator_elements_as_keywords() stores
//  labeled_properties, PARAMs, and FIELD attributes as FITs keywords.
//  value_ elements (if such exist) of labeled_properties instances
//  are added to their parent's attributes_ element as the value
//  corresponding to the ATTR_IRSA_VALUE attribute.

// Since FITS has better support for long keyword values than for long
// keys, this function stores names of the aforementioned
// VOTable-style elements ("prop_labels") as part of keyword values
// rather than as FITS keys.  The FITS keys for these values are
// strings of the form VOTABLE_KEYWORD_HEAD followed by consecutive
// integers N; their only purpose is to support sorting by N so that
// values corresponding to a single VOTable element can be processed
// as a group.  The string LABEL_END_MARKER indicates where the
// prop_label ends and the string corresponding to the element's
// <value_> (or value of one of its attributes_) begins.  For example:

// key: {VOTABLE_KEYWORD_HEAD}3
// value: VOTABLE.RESOURCE.INFO.ADQL.<xmlattr>ucd{LABEL_END_MARKER}meta.adql

// In general,  prop_labels have the following form:

// <votable_element_name>.<prop_name>.{XMLATTR}.<attr_name>
//   (for elements of prop.attributes_), or
//
// <votable_element_name>.<prop_name>.{XMLATTR}.{ATTR_IRSA_VALUE}
//   (for prop.value_)

// where <votable_element_name> is on the lines of
// "VOTABLE.RESOURCE.INFO" and <prop_name> is the value of the
// property's ATTR_NAME attribute, assumed to be non-empty and
// distinct for INFO elements.

//(We can't yet convert VOTables with more than one RESOURCE to FITS format.
// 07Dec20)

void write_tablator_elements_as_keywords(
        fitsfile *fits_file, const tablator::Labeled_Properties &labeled_properties) {
    int status = 0;
    uint kwd_idx = 0;
    bool got_fits_keyword = false;

    for (const auto &label_and_prop : labeled_properties) {
        std::string label = label_and_prop.first;
        const auto &prop = label_and_prop.second;
        std::string value(prop.get_value());
        std::string comment;

        // Base for the prop_label with which we will store each of prop's
        // value and attributes.
        auto prop_label_base = label + tablator::DOT;

        const auto &prop_attributes = prop.get_attributes();
        auto name_iter = prop_attributes.find(tablator::ATTR_NAME);
        if (name_iter == prop_attributes.end()) {
            // Some versions of ccfits translate keywords to upper-case.
            name_iter = prop_attributes.find(boost::to_upper_copy(tablator::ATTR_NAME));
        }

        const auto value_iter = prop_attributes.find(tablator::ATTR_VALUE);
        const auto comment_iter = prop_attributes.find(tablator::ATTR_COMMENT);
        const auto ucd_iter = prop_attributes.find(tablator::ATTR_UCD);

        // FITS requires that key values be unique, but there could be
        // many INFO elements having attributes with the same names.
        // If <label> ends in INFO, we include in each <prop_label> the
        // value of the relevant NAME attribute as well as the name of
        // the attribute whose value is being stored.  (INFO elements
        // are assumed to have NAME attributes.)

        // value_ elements of labeled_properties are stored as values
        // of the ATTR_IRSA_VALUE attribute, so the corresponding
        // prop_label_base strings, like those corresponding to
        // components of the attributes_ element, must contain
        // XMLATTR_DOT.
        if (boost::ends_with(label, tablator::INFO)) {
            if (name_iter == prop_attributes.end() || (name_iter->second).empty()) {
                // Shouldn't happen!
                // std::cout << "*** Oops, couldn't find NAME. ***" << std::endl;
                continue;
            } else {
                // If this labeled_property originated as a FITS
                // keyword, it will have at most three attributes:
                // value, comment, and ucd.
                if (boost::starts_with(prop_label_base,
                                       tablator::VOTABLE_RESOURCE_DOT) &&
                    (!boost::starts_with(prop_label_base,
                                         tablator::VOTABLE_RESOURCE_TABLE_DOT)) &&
                    value_iter != prop_attributes.end() &&
                    prop_attributes.size() ==
                            static_cast<uint>(
                                    1 +
                                    (comment_iter == prop_attributes.end() ? 0 : 1) +
                                    (ucd_iter == prop_attributes.end() ? 0 : 1))) {
                    // Prepare to write it as a plain FITS keyword
                    // without the trappings used for translated
                    // VOTable elements.
                    got_fits_keyword = true;
                    prop_label_base = prop_label_base.substr(
                            tablator::VOTABLE_RESOURCE_DOT.size());
                    prop_label_base =
                            prop_label_base.substr(0, prop_label_base.size() - 1);
                } else {
                    prop_label_base = label + tablator::DOT + name_iter->second +
                                      tablator::DOT + tablator::XMLATTR_DOT;
                }
            }

        } else if (!boost::ends_with(prop_label_base, tablator::XMLATTR_DOT)) {
            // Could be e.g. (1) the RESOURCE-level "type" attribute,
            // displayed in VOTable format as <RESOURCE type =
            // "results"> and in IPAC table format as "\type =
            // 'results', or (2) PARAM- or FIELD-level attributes.
            if (boost::starts_with(prop_label_base, tablator::VOTABLE_RESOURCE_DOT) &&
                (!boost::starts_with(prop_label_base,
                                     tablator::VOTABLE_RESOURCE_TABLE_DOT)) &&
                value_iter != prop_attributes.end() &&
                prop_attributes.size() ==
                        static_cast<uint>(
                                1 + (comment_iter == prop_attributes.end() ? 0 : 1) +
                                (ucd_iter == prop_attributes.end() ? 0 : 1))) {
                // Prepare to write it as a plain FITS keyword.
                got_fits_keyword = true;
                prop_label_base =
                        prop_label_base.substr(tablator::VOTABLE_RESOURCE_DOT.size());
                prop_label_base = prop_label_base.substr(0, prop_label_base.size() - 1);
            } else {
                prop_label_base += tablator::XMLATTR_DOT;
            }
        }

        // Deal first with <value>, if non-empty (it is empty for INFO elements).
        if (!value.empty()) {
            std::string idx_label(tablator::VOTABLE_KEYWORD_HEAD);
            idx_label.append(std::to_string(kwd_idx));

            fits_write_key_longstr(fits_file, idx_label.c_str(),
                                   (prop_label_base + tablator::ATTR_IRSA_VALUE +
                                    tablator::LABEL_END_MARKER + value)
                                           .c_str(),
                                   NULL, &status);

            if (status != 0) {
                throw CCfits::FitsError(status);
            }
            ++kwd_idx;
        }

        // On to attributes
        if (got_fits_keyword) {
            // JTODO: figure out how to handle ucd for FITS files
            // in both write_fits() and read_fits().
            value.assign(value_iter->second);
            if (comment_iter != prop_attributes.end()) {
                comment.assign(comment_iter->second);
            }
            fits_write_key_longstr(fits_file, prop_label_base.c_str(), value.c_str(),
                                   comment.c_str(), &status);
            if (status != 0) {
                throw CCfits::FitsError(status);
            }
        } else {
            for (auto &attr : prop_attributes) {
                std::string idx_label(tablator::VOTABLE_KEYWORD_HEAD);
                idx_label.append(std::to_string(kwd_idx));
#ifdef FIXED_FITS_COMMENT
                // This step prepares us to store the comment in a special FITS way,
                // but as of 13Nov20, comments will be truncated or omitted if
                // comment.size() + value.size() > 65.
                if (attr.first == "comment") {
                    comment.assign(attr.second);
                } else
#endif
                    fits_write_key_longstr(fits_file, idx_label.c_str(),
                                           (prop_label_base + attr.first +
                                            tablator::LABEL_END_MARKER + attr.second)
                                                   .c_str(),
                                           comment.c_str(), &status);
                if (status != 0) {
                    throw CCfits::FitsError(status);
                }
                ++kwd_idx;
            }
        }
    }
}


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
// write to a stream or to a file.
//

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

    // In the loop below, we'll create and store tunit and tform
    // values in string form in helper vectors-of-strings so that the
    // value strings will persist after the loop ends; otherwise they
    // would be deallocated at the end of the loop. Following the
    // loop, we'll retrieve the char arrays underlying the elements of
    // these vectors-of-strings and load them into the
    // vectors-of-char-pointers needed for the call to
    // fits_create_tbl().

    std::vector<std::string> tunit_helper;
    std::vector<std::string> tform_helper;

    const auto &columns = get_columns();

    // Skip null_bitfield_flags column (col_idx == 0).  Corresponding
    // columns of the tablator and FITS tables have the same (1-based)
    // index.
    for (size_t col_idx = 1; col_idx < columns.size(); ++col_idx) {
        auto &column = columns[col_idx];

        ttype.emplace_back(column.get_name().c_str());

        const auto col_attrs = column.get_field_properties().get_attributes();
        auto unit = col_attrs.find(UNIT);
        if (unit == col_attrs.end()) {
            tunit_helper.emplace_back("");
        } else {
            tunit_helper.emplace_back(unit->second);
        }
        auto fits_format_str =
                get_fits_format(datatypes_for_writing[col_idx], column.get_type(),
                                column.get_array_size()
#ifdef FUTURE
                                        ,
                                column.get_dynamic_array_flag()
#endif
                );
        tform_helper.emplace_back(fits_format_str);
    }

    // Populate tform and tunit from their respective helpers.
    for (auto &form_str : tform_helper) {
        tform.emplace_back(form_str.c_str());
    }

    for (auto &unit_str : tunit_helper) {
        tunit.emplace_back(unit_str.c_str());
    }


    //********************/
    // Create FITS table.
    //********************/

    fits_create_tbl(fits_file, BINARY_TBL, 0, ttype.size(),
                    const_cast<char **>(ttype.data()),
                    const_cast<char **>(tform.data()),
                    const_cast<char **>(tunit.data()), "Table", &status);
    if (status != 0) {
        throw CCfits::FitsError(status);
    }
    // Warn users that this table uses the Long String Keyword convention.
    fits_write_key_longwarn(fits_file, &status);
    if (status != 0) {
        throw CCfits::FitsError(status);
    }

    //*********************************************************
    // Represent various elements using FITS keywords
    //*********************************************************

    //*************************************/
    // Set null values for numeric columns.
    //*************************************/

    for (size_t col_idx = 1; col_idx < columns.size(); ++col_idx) {
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
    bool include_column_attributes_f = true;
    const auto combined_labeled_attributes =
            combine_attributes_all_levels(include_column_attributes_f);

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


    write_tablator_elements_as_keywords(fits_file, combined_labeled_properties);

    //*********************************************************
    // Write column-level metadata and data.
    //*********************************************************

    const auto &offsets = get_offsets();
    const size_t number_of_rows(get_num_rows());

    // Retrieve table's data pointer.
    const uint8_t *data_start_ptr = get_data().data();

    // Cast away const, as FITS functions require.
    uint8_t *row_start_ptr = const_cast<uint8_t *>(data_start_ptr);

    // FITS row index is 1-based.
    for (size_t fits_row_idx = 1; fits_row_idx <= number_of_rows;
         ++fits_row_idx, row_start_ptr += get_row_size()) {
        // Skip null_bitfield_flags column.  Corresponding
        // columns of the tablator and FITS tables have the same (1-based)
        // index.
        auto tab_row_idx = fits_row_idx - 1;
        for (size_t col_idx = 1; col_idx < columns.size(); ++col_idx) {
            auto &column = columns[col_idx];

            uint8_t *curr_data_ptr = row_start_ptr + offsets[col_idx];

            Data_Type datatype_for_writing = datatypes_for_writing[col_idx];

            size_t array_size = column.get_array_size();
            if (column.get_dynamic_array_flag()) {
                array_size = *(reinterpret_cast<const uint32_t *>(curr_data_ptr));
                curr_data_ptr += DYNAMIC_ARRAY_OFFSET;
            }

            bool null_flag_is_set = is_null_value(tab_row_idx, col_idx);

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
