#include <CCfits/CCfits>

#include "../Table.hxx"
#include "../fits_keyword_mapping.hxx"

namespace {
template <typename T>
void read_scalar_column(uint8_t *position, CCfits::Column &c, const size_t &rows,
                        const size_t &row_size) {
    std::vector<T> v;
    c.read(v, 1, rows);
    uint8_t *current = position;
    for (auto &element : v) {
        *reinterpret_cast<T *>(current) = element;
        current += row_size;
    }
}

template <typename T>
void read_vector_column(fitsfile *fits_file, uint8_t *position, CCfits::Column &c,
                        const size_t &rows, const size_t &row_size) {
    /// Use the C api because the C++ api (Column::readArrays) is
    /// horrendously slow.
    int status(0), anynul(0);
    std::vector<T> temp_array(c.repeat());

    auto get_matched_datatype = CCfits::FITSUtil::MatchType<T>();
    uint8_t *current = position;
    for (size_t row = 0; row < rows; ++row) {
        uint8_t *element_start = current;

        fits_read_col(fits_file, get_matched_datatype(), c.index(), row + 1, 1,
                      c.repeat(), NULL, temp_array.data(), &anynul, &status);

        for (size_t offset = 0; offset < c.repeat(); ++offset) {
            *reinterpret_cast<T *>(current) = temp_array[offset];
            current += sizeof(T);
        }
        current = element_start + row_size;
    }
}

template <typename T>
void read_column(fitsfile *fits_file, uint8_t *position, CCfits::Column &c,
                 const bool &is_array, const size_t &rows, const size_t &row_size) {
    if (!is_array)
        read_scalar_column<T>(position, c, rows, row_size);
    else
        read_vector_column<T>(fits_file, position, c, rows, row_size);
}
}  // namespace

// FIXME: This does not copy any keywords
void tablator::Table::read_fits(const boost::filesystem::path &path) {
    CCfits::FITS fits(path.string(), CCfits::Read, false);
    if (fits.extension().empty())
        throw std::runtime_error("Could not find any extensions in this file: " +
                                 path.string());
    CCfits::ExtHDU &table_extension = *(fits.extension().begin()->second);
    CCfits::BinTable *table(dynamic_cast<CCfits::BinTable *>(&table_extension));

    std::vector<std::string> fits_ignored_keywords{{"LONGSTRN"}};
    auto keyword_mapping = fits_keyword_mapping(false);
    table_extension.readAllKeys();
    for (auto &k : table_extension.keyWord()) {
        std::string name(k.first), value;
        if (std::find(fits_ignored_keywords.begin(), fits_ignored_keywords.end(),
                      name) != fits_ignored_keywords.end())
            continue;

        /// Annoyingly, CCfits does not have a way to just return the
        /// value.  You have to give it something to put it in.
        Property p(k.second->value(value));
        auto i = keyword_mapping.find(name);
        if (i != keyword_mapping.end()) {
            name = i->second;
            p.add_attribute("ucd", name);
        }
        if (!k.second->comment().empty())
            p.add_attribute("comment", k.second->comment());
        add_labeled_property(std::make_pair(name, p));
    }

    /// CCfits is 1 based, not 0 based.
    const bool has_null_bitfield_flags(table->column().size() > 0 &&
                                       table->column(1).name() ==
                                               null_bitfield_flags_name &&
                                       table->column(1).type() == CCfits::Tbyte);
    auto &columns = get_columns();
    auto &offsets = get_offsets();
    auto &data = get_data();


    if (!has_null_bitfield_flags) {
        append_column(columns, offsets, null_bitfield_flags_name, Data_Type::UINT8_LE,
                      (table->column().size() + 7) / 8,
                      Field_Properties(null_bitfield_flags_description, {}));
    }

    for (size_t i = 0; i < table->column().size(); ++i) {
        CCfits::Column &c = table->column(i + 1);
        size_t array_size = 1;
        if (std::isdigit(c.format().at(0))) array_size = std::stoll(c.format());
        switch (c.type()) {
            case CCfits::Tlogical:
                append_column(columns, offsets, c.name(), Data_Type::INT8_LE,
                              array_size);
                break;
            case CCfits::Tbyte:
                append_column(columns, offsets, c.name(), Data_Type::UINT8_LE,
                              array_size);
                break;
            case CCfits::Tshort:
                append_column(columns, offsets, c.name(), Data_Type::INT16_LE,
                              array_size);
                break;
            case CCfits::Tushort:
                append_column(columns, offsets, c.name(), Data_Type::UINT16_LE,
                              array_size);
                break;
            case CCfits::Tint:
                append_column(columns, offsets, c.name(), Data_Type::INT32_LE,
                              array_size);
                break;
            case CCfits::Tuint:
                append_column(columns, offsets, c.name(), Data_Type::UINT32_LE,
                              array_size);
                break;
            case CCfits::Tlong:
                // The Tlong type code is used for 32-bit integer columns when reading.
                append_column(columns, offsets, c.name(), Data_Type::INT32_LE,
                              array_size);
                break;
            case CCfits::Tulong:
                // The Tulong type code is used for 32-bit unsigned integer columns when
                // reading.
                append_column(columns, offsets, c.name(), Data_Type::UINT32_LE,
                              array_size);
                break;
            case CCfits::Tlonglong:
                append_column(columns, offsets, c.name(), Data_Type::INT64_LE,
                              array_size);
                break;
            case CCfits::Tfloat: {
                Field_Properties nan_nulls;
                nan_nulls.get_values().null =
                        std::to_string(std::numeric_limits<float>::quiet_NaN());
                append_column(columns, offsets, c.name(), Data_Type::FLOAT32_LE,
                              array_size, nan_nulls);
            } break;
            case CCfits::Tdouble: {
                Field_Properties nan_nulls;
                nan_nulls.get_values().null =
                        std::to_string(std::numeric_limits<double>::quiet_NaN());
                append_column(columns, offsets, c.name(), Data_Type::FLOAT64_LE,
                              array_size, nan_nulls);
            } break;
            case CCfits::Tstring:
                append_column(columns, offsets, c.name(), Data_Type::CHAR, c.width());
                break;
            default:
                throw std::runtime_error(
                        "Unsupported data type in the fits file for "
                        "column '" +
                        c.name() + "'");
        }
    }

    /// table->rows () returns an int, so there may be issues with more
    /// than 2^32 rows

    data.resize(table->rows() * row_size());

    /// Exit early if there is no data in the table.  Otherwise CCfits
    /// dies in read_column() :(
    if (data.empty()) {
        return;
    }

    fitsfile *fits_pointer = fits.fitsPointer();
    const size_t column_data_offset(has_null_bitfield_flags ? 0 : 1);
    for (size_t i = 0; i < table->column().size(); ++i) {
        const size_t offset(offsets[i + column_data_offset]);
        /// CCfits is 1 based, not 0 based.
        CCfits::Column &c = table->column(i + 1);
        const bool is_array(std::isdigit(c.format().at(0)) &&
                            (std::stoll(c.format()) != 1));
        switch (c.type()) {
            case CCfits::Tlogical: {
                if (!is_array) {
                    std::vector<int> v;
                    c.read(v, 1, table->rows());
                    size_t element_offset = offset;
                    for (auto &element : v) {
                        data[element_offset] = element;
                        element_offset += row_size();
                    }
                } else {
                    // FIXME: Use the C api because Column::readArrays is
                    // horrendously slow.
                    std::vector<std::valarray<int> > v;
                    c.readArrays(v, 1, table->rows());
                    size_t start_offset_for_row = offset;
                    for (auto &array : v) {
                        auto element_offset = start_offset_for_row;
                        for (auto &element : array) {
                            data[element_offset] = element;
                            ++element_offset;
                        }
                        start_offset_for_row += row_size();
                    }
                }
            } break;
            case CCfits::Tbyte:
                read_column<uint8_t>(fits_pointer, data.data() + offset, c, is_array,
                                     table->rows(), row_size());
                break;
            case CCfits::Tshort:
                read_column<int16_t>(fits_pointer, data.data() + offset, c, is_array,
                                     table->rows(), row_size());
                break;
            case CCfits::Tushort:
                read_column<uint16_t>(fits_pointer, data.data() + offset, c, is_array,
                                      table->rows(), row_size());
                break;
            case CCfits::Tuint:
            case CCfits::Tulong:
                read_column<uint32_t>(fits_pointer, data.data() + offset, c, is_array,
                                      table->rows(), row_size());
                break;
            case CCfits::Tint:
            case CCfits::Tlong:
                read_column<int32_t>(fits_pointer, data.data() + offset, c, is_array,
                                     table->rows(), row_size());
                break;
            case CCfits::Tlonglong:
                read_column<int64_t>(fits_pointer, data.data() + offset, c, is_array,
                                     table->rows(), row_size());
                break;
            case CCfits::Tfloat:
                read_column<float>(fits_pointer, data.data() + offset, c, is_array,
                                   table->rows(), row_size());
                break;
            case CCfits::Tdouble:
                read_column<double>(fits_pointer, data.data() + offset, c, is_array,
                                    table->rows(), row_size());
                break;
            case CCfits::Tstring: {
                std::vector<std::string> v;
                c.read(v, 1, table->rows());
                size_t element_offset = offset;
                for (auto &element : v) {
                    for (size_t j = 0; j < element.size(); ++j)
                        data[element_offset + j] = element[j];
                    for (int j = element.size(); j < c.width(); ++j)
                        data[element_offset + j] = '\0';
                    element_offset += row_size();
                }
            } break;
            default:
                throw std::runtime_error(
                        "Unsupported data type in the fits file for "
                        "column " +
                        c.name());
        }
        // FIXME: This should get the comment, but the comment()
        // function is protected???
        if (!c.unit().empty()) {
            columns[i + column_data_offset].get_field_properties().set_attributes(
                    {{"unit", c.unit()}});
        }
    }
}
