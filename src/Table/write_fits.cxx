#include "../Table.hxx"

#include <longnam.h>
#include <CCfits/CCfits>
#include <algorithm>

#include "../Data_Type_Adjuster.hxx"
#include "../Utils/Vector_Utils.hxx"
#include "../fits_keyword_mapping.hxx"
#include "../to_string.hxx"

// JTODO Descriptions and attributes get lost in conversion to and from fits.

template <typename data_type>
void write_column(fitsfile *fits_file, int fits_type, int col_id, const uint8_t *data,
                  hsize_t array_size, size_t row) {
    int status = 0;
    fits_write_col(fits_file, fits_type, col_id + 1, row, 1, array_size,
                   reinterpret_cast<data_type *>(const_cast<uint8_t *>(data)), &status);
    if (status != 0) throw CCfits::FitsError(status);
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
    /// Remove the file because cfitsio will fail if the file still
    /// exists.
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

        /// I have to reopen the file because otherwise fits_close_file
        /// will delete the memory
        fits_reopen_file(fits_file, &reopen_file, &status);
        if (status != 0) {
            throw CCfits::FitsError(status);
        }

        fits_close_file(fits_file, &status);
        if (status != 0) {
            throw CCfits::FitsError(status);
        }

        os.write(static_cast<const char *>(buffer), buffer_size);
        /// This also free's buffer.
        fits_close_file(reopen_file, &status);
        if (status != 0) {
            throw CCfits::FitsError(status);
        }
    } catch (...) {
        free(buffer);
        throw;
    }
}

/// We separate out the write_fits implementation so that we can
/// insert a read of the memory image so that we can write to a
/// stream.
void tablator::Table::write_fits(
        fitsfile *fits_file,
        const std::vector<Data_Type> &datatypes_for_writing) const {
    int status = 0;
    std::vector<string> fits_names;
    std::vector<string> fits_types;

    std::vector<string> fits_units;
    std::vector<const char *> ttype, tunit;
    const auto &columns = get_columns();
    for (size_t i = 0; i < columns.size(); ++i) {
        auto &column = columns[i];
        ttype.push_back(column.get_name().c_str());
        std::string array_size_str(std::to_string(column.get_array_size()));
        char fits_type;

        switch (datatypes_for_writing[i]) {
            case Data_Type::INT8_LE:
                fits_type = 'L';
                break;
            case Data_Type::UINT8_LE:
                fits_type = 'B';
                break;
            case Data_Type::INT16_LE:
                fits_type = 'I';
                break;
            case Data_Type::UINT16_LE:
                fits_type = 'U';
                break;
            case Data_Type::INT32_LE:
                fits_type = 'J';
                break;
            case Data_Type::UINT32_LE:
                fits_type = 'V';
                break;
            case Data_Type::INT64_LE:
                fits_type = 'K';
                break;
            case Data_Type::UINT64_LE:
                throw std::runtime_error(
                        "Unsupported uint64 data type when writing fits "
                        "data: , column #" +
                        std::to_string(i) + ", original data type " +
                        to_string(column.get_type()));
                break;
            case Data_Type::FLOAT32_LE:
                fits_type = 'E';
                break;
            case Data_Type::FLOAT64_LE:
                fits_type = 'D';
                break;
            case Data_Type::CHAR:
                fits_type = 'A';
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
        fits_types.push_back(array_size_str + fits_type);

        const auto col_atts = column.get_field_properties().get_attributes();
        auto unit = col_atts.find(UNIT);
        if (unit == col_atts.end()) {
            tunit.push_back("");
        } else {
            tunit.push_back(unit->second.c_str());
        }
    }

    // We have to store the fits_type in a string and then set the
    // c_str(), because otherwise the string will get deallocated.
    std::vector<const char *> tform;
    for (auto &t : fits_types) tform.push_back(t.c_str());

    fits_create_tbl(fits_file, BINARY_TBL, 0, ttype.size(),
                    const_cast<char **>(ttype.data()),
                    const_cast<char **>(tform.data()),
                    const_cast<char **>(tunit.data()), "Table", &status);
    if (status != 0) throw CCfits::FitsError(status);

    fits_write_key_longwarn(fits_file, &status);
    if (status != 0) throw CCfits::FitsError(status);

    assert(get_resource_elements().size() > 0);

    // Combine and write properties and trailing info for all levels at which they are
    // defined.
    auto combined_labeled_properties = combine_labeled_properties_all_levels();
    const auto combined_labeled_trailing_info_lists =
            combine_trailing_info_lists_all_levels();
    combined_labeled_properties.insert(combined_labeled_properties.end(),
                                       combined_labeled_trailing_info_lists.begin(),
                                       combined_labeled_trailing_info_lists.end());


    for (auto &label_and_prop : combined_labeled_properties) {
        auto keyword_mapping = fits_keyword_mapping(true);
        std::string name = label_and_prop.first;
        const auto &prop = label_and_prop.second;
        auto i = keyword_mapping.find(name);
        if (i != keyword_mapping.end()) {
            name = i->second;
        }
        std::string comment, value(prop.get_value());
        for (auto &att : prop.get_attributes()) {
            if (att.first == "comment") {
                comment = att.second;
            } else if (att.first != "ucd") {
                if (!value.empty()) {
                    value += ", ";
                }
                // JTODO Serge, are we allowed to change how we write
                // attributes for fits?  We could make it easier to
                // restore the originals when reading (right now we
                // don't even try).
                value += att.first + ": " + att.second;
            }
        }
        fits_write_key_longstr(fits_file, name.c_str(), value.c_str(), comment.c_str(),
                               &status);
        if (status != 0) {
            throw CCfits::FitsError(status);
        }
    }

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
