#include "../Table.hxx"

#include <longnam.h>
#include <CCfits/CCfits>
#include <algorithm>

#include "../Data_Type_Adjuster.hxx"
#include "../Utils/Vector_Utils.hxx"
#include "../to_string.hxx"

// JTODO Descriptions and field-level attributes get lost in conversion to and from
// fits.

namespace {
template <typename data_type>
void write_column(fitsfile *fits_file, int format_str, int col_id, const uint8_t *data,
                  hsize_t array_size, size_t row) {
    int status = 0;
    fits_write_col(fits_file, format_str, col_id + 1, row, 1, array_size,
                   reinterpret_cast<data_type *>(const_cast<uint8_t *>(data)), &status);
    if (status != 0) throw CCfits::FitsError(status);
}
}  // namespace

/**********************************************************/

// The function write_labeled_properties_as_keywords() stores
//  labeled_properties as FITs keywords.  value_ elements (if such
//  exist) of labeled_properties instances are added to their parent's
//  attributes_ element as the value corresponding to the
//  ATTR_IRSA_VALUE attribute.

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

void write_labeled_properties_as_keywords(
        fitsfile *fits_file,
        const std::vector<std::pair<std::string, tablator::Property>>
                &labeled_properties) {
    int status = 0;
    uint kwd_idx = 0;

    for (const auto &label_and_prop : labeled_properties) {
        std::string label = label_and_prop.first;
        const auto &prop = label_and_prop.second;
        std::string value(prop.get_value());
        std::string comment;

        // Base for the prop_label with which we will store each of prop's
        // value and attributes.
        auto prop_label_base = label + tablator::DOT;

        const auto &attributes = prop.get_attributes();
        auto name_iter = attributes.find(tablator::ATTR_NAME);

        if (name_iter == attributes.end()) {
            name_iter = attributes.find(boost::to_upper_copy(tablator::ATTR_NAME));
        }

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
            if (name_iter == attributes.end() || (name_iter->second).empty()) {
                // Shouldn't happen!
                std::cout << "*** Oops, couldn't find NAME. ***" << std::endl;
            } else {
                prop_label_base = label + tablator::DOT + name_iter->second +
                                  tablator::DOT + tablator::XMLATTR_DOT;
            }
        } else if (!boost::ends_with(prop_label_base, tablator::XMLATTR_DOT)) {
            // e.g. if the table was originally in FITS or IPAC_TABLE format
            // JTODO standardize?
            prop_label_base += tablator::XMLATTR_DOT;
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
        for (auto &attr : prop.get_attributes()) {
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

// We separate out the write_fits implementation so that we can
// write to a stream or to a file.
//
void tablator::Table::write_fits(
        fitsfile *fits_file,
        const std::vector<Data_Type> &datatypes_for_writing) const {
    int status = 0;


    //*********************************************************
    // Create FITS table
    //*********************************************************

    // According to the FITS standard, the TTYPEn keyword's value shall be
    // equal to the n-th field's name, and the TFORMn keyword's value to the
    // n-th field's format code.

    std::vector<const char *> ttype;
    std::vector<const char *> tunit;

    // We store formats as (copies of) strings for now and
    // convert to const char* for the fits function call.
    std::vector<string> format_strs;

    //  Load types, units, and formats
    const auto &columns = get_columns();
    for (size_t column_idx = 0; column_idx < columns.size(); ++column_idx) {
        auto &column = columns[column_idx];

        ttype.emplace_back(column.get_name().c_str());

        const auto &col_attrs = column.get_field_properties().get_attributes();
        auto unit_p = col_attrs.find(UNIT);
        if (unit_p == col_attrs.end()) {
            tunit.emplace_back("");
        } else {
            tunit.emplace_back(unit_p->second.c_str());
        }

        std::string array_size_str(std::to_string(column.get_array_size()));
        char format_str;
        switch (datatypes_for_writing[column_idx]) {
            case Data_Type::INT8_LE:
                format_str = 'L';
                break;
            case Data_Type::UINT8_LE:
                format_str = 'B';
                break;
            case Data_Type::INT16_LE:
                format_str = 'I';
                break;
            case Data_Type::UINT16_LE:
                format_str = 'U';
                break;
            case Data_Type::INT32_LE:
                format_str = 'J';
                break;
            case Data_Type::UINT32_LE:
                format_str = 'V';
                break;
            case Data_Type::INT64_LE:
                format_str = 'K';
                break;
            case Data_Type::UINT64_LE:
                throw std::runtime_error(
                        "Unsupported uint64 data type when writing fits "
                        "data: , column #" +
                        std::to_string(column_idx) + ", original data type " +
                        to_string(column.get_type()));
                break;
            case Data_Type::FLOAT32_LE:
                format_str = 'E';
                break;
            case Data_Type::FLOAT64_LE:
                format_str = 'D';
                break;
            case Data_Type::CHAR:
                format_str = 'A';
                if (column.get_type() == Data_Type::UINT64_LE) {
                    // Adjust array_size as well as data_type.
                    array_size_str.assign(std::to_string(
                            Data_Type_Adjuster::get_char_array_size_for_uint64_col(
                                    column.get_array_size())));
                }
                break;
            default:
                throw std::runtime_error(
                        "In column '" + column.get_name() +
                        "': unknown data type when writing fits data: " +
                        to_string(column.get_type()));
                break;
        }
        format_strs.emplace_back(array_size_str + format_str);
    }

    // We have to store the format_str in a string and then set the
    // c_str(), because otherwise the string will get deallocated.
    std::vector<const char *> tform;
    for (auto &t : format_strs) {
        tform.push_back(t.c_str());
    }

    fits_create_tbl(fits_file, BINARY_TBL, 0, ttype.size(),
                    const_cast<char **>(ttype.data()),
                    const_cast<char **>(tform.data()),
                    const_cast<char **>(tunit.data()), "Table", &status);
    if (status != 0) {
        throw CCfits::FitsError(status);
    }

    fits_write_key_longwarn(fits_file, &status);
    if (status != 0) {
        throw CCfits::FitsError(status);
    }

    //*********************************************************
    // Represent various elements using FITS keywords
    //*********************************************************

    assert(get_resource_elements().size() > 0);
    const auto combined_labeled_attributes = combine_attributes_all_levels();

    // Combine and write properties and trailing info for all levels at which they are
    // defined.
    auto combined_labeled_properties = combine_labeled_properties_all_levels();
    const auto combined_labeled_trailing_info_lists =
            combine_trailing_info_lists_all_levels();
    combined_labeled_properties.insert(combined_labeled_properties.end(),
                                       combined_labeled_trailing_info_lists.begin(),
                                       combined_labeled_trailing_info_lists.end());

    combined_labeled_properties.insert(combined_labeled_properties.end(),
                                       combined_labeled_attributes.begin(),
                                       combined_labeled_attributes.end());


    write_labeled_properties_as_keywords(fits_file, combined_labeled_properties);

    //*********************************************************
    // Write column-level metadata and data.
    //*********************************************************

    const auto &offsets = get_offsets();
    const uint8_t *row_pointer(get_data().data());
    const size_t number_of_rows(num_rows());
    for (size_t row = 1; row <= number_of_rows; ++row) {
        for (size_t i = 0; i < columns.size(); ++i) {
            auto &column = columns[i];
            const uint8_t *offset_data = row_pointer + offsets[i];

            switch (datatypes_for_writing[i]) {
                case Data_Type::INT8_LE:

                    write_column<bool>(fits_file, TLOGICAL, i, offset_data,
                                       column.get_array_size(), row);
                    break;
                case Data_Type::UINT8_LE:
                    write_column<uint8_t>(fits_file, TBYTE, i, offset_data,
                                          column.get_array_size(), row);
                    break;
                case Data_Type::INT16_LE:
                    write_column<int16_t>(fits_file, TSHORT, i, offset_data,
                                          column.get_array_size(), row);
                    break;
                case Data_Type::UINT16_LE:
                    write_column<uint16_t>(fits_file, TUSHORT, i, offset_data,
                                           column.get_array_size(), row);
                    break;
                case Data_Type::INT32_LE:
                    write_column<int32_t>(fits_file, TINT, i, offset_data,
                                          column.get_array_size(), row);
                    break;
                case Data_Type::UINT32_LE:
                    write_column<uint32_t>(fits_file, TUINT, i, offset_data,
                                           column.get_array_size(), row);
                    break;
                case Data_Type::INT64_LE:
                    write_column<int64_t>(fits_file, TLONGLONG, i, offset_data,
                                          column.get_array_size(), row);
                    break;
                case Data_Type::UINT64_LE:
                    throw std::runtime_error(
                            "Unsupported uint64 data type when writing fits "
                            "data: , column #" +
                            std::to_string(i) + ", original data type " +
                            to_string(column.get_type()));
                    break;
                case Data_Type::FLOAT32_LE:
                    write_column<float>(fits_file, TFLOAT, i, offset_data,
                                        column.get_array_size(), row);
                    break;
                case Data_Type::FLOAT64_LE:
                    write_column<double>(fits_file, TDOUBLE, i, offset_data,
                                         column.get_array_size(), row);
                    break;
                case Data_Type::CHAR: {
                    std::string temp_string;
                    if (column.get_type() == Data_Type::CHAR) {
                        // Really a CHAR column, not a rewritten UINT64_LE
                        temp_string.assign(reinterpret_cast<const char *>(offset_data),
                                           offsets[i + 1] - offsets[i]);
                    } else {
                        // Really UINT64_LE
                        auto curr_ptr = offset_data;
                        auto array_size = column.get_array_size();
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
                    fits_write_col(fits_file, TSTRING, i + 1, row, 1, 1, &temp_chars,
                                   &status);
                    if (status != 0) throw CCfits::FitsError(status);
                } break;
                default:
                    throw std::runtime_error(
                            "Unknown data type when writing fits "
                            "data: " +
                            to_string(column.get_type()));
                    break;
            }
        }
        row_pointer += row_size();
    }
}
