#include <regex>

#include <CCfits/CCfits>

#include "../Table.hxx"
#include "../fits_keyword_ucd_mapping.hxx"

// JTODO Descriptions and field-level attributes get lost in conversion to and from
// fits.

namespace {

// There is more than one way to indicate the array_size of a FITS column.
size_t get_array_size(const CCfits::Column &fits_col) {
    size_t array_size = 1;
    if (std::isdigit(fits_col.format().at(0))) {
        array_size = std::stoll(fits_col.format());
    } else {
        // Capture e.g. "PK(5)".
        static std::regex format_regex{R"(([0-9]+))"};
        std::smatch number_match;
        bool format_contains_paren_number =
                std::regex_search(fits_col.format(), number_match, format_regex);
        if (format_contains_paren_number) {
            array_size = std::stoi(number_match[1]);
        }
    }
    return array_size;
}


// CCfits does not make it easy to retrieve keyword values.  Only
// keyword values of keytypes Tint, Tfloat, Tdouble, and Tstring can
// be retrieved from CCfits in string form.

// This function retrieves values of the keytypes mentioned above as
// strings. It retrieves values of keytypes implicitly convertible to
// int as int and then converts them to string.  It propagates the
// exception CCfits throws when asked to retrieve a value of any other
// keytype, e.g. Tcomplex.

void get_keyword_value_as_string(std::string &value_str,
                                 const CCfits::Keyword *keyword) {
    if ((keyword->keytype() == CCfits::Tint || keyword->keytype() == CCfits::Tfloat ||
         keyword->keytype() == CCfits::Tdouble ||
         keyword->keytype() == CCfits::Tstring)) {
        keyword->value(value_str);
    } else if (keyword->keytype() == CCfits::Tlogical) {
        bool value_bool;
        keyword->value(value_bool);
        value_str.assign(value_bool ? "true" : "false");
    } else {
        // If the keytype is not convertible to int, value() will throw an exception.
        int value_int;
        keyword->value(value_int);
        value_str.assign(std::to_string(value_int));
    }
}

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
    // Use the C api because the C++ api (Column::readArrays) is
    // horrendously slow.
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


void tablator::Table::read_fits(const boost::filesystem::path &path) {
    CCfits::FITS fits(path.string(), CCfits::Read, false);
    if (fits.extension().empty())
        throw std::runtime_error("Could not find any extensions in this file: " +
                                 path.string());
    CCfits::ExtHDU &table_extension = *(fits.extension().begin()->second);
    CCfits::BinTable *ccfits_table(dynamic_cast<CCfits::BinTable *>(&table_extension));

    static std::vector<std::string> fits_ignored_keywords{{"LONGSTRN"}};
    static const auto keyword_ucd_mapping = fits_keyword_ucd_mapping(false);
    table_extension.readAllKeys();

    std::vector<Column> columns;
    std::vector<size_t> offsets = {0};


    // Extract labeled_properties/trailing_info_lists/attributes.
    // If this table was created by write_fits(), those elements
    // would have been converted to labeled_properties and stored as keywords
    // of the form

    // ELEMENT.<prop_name>.XMLATTR.<attr_name> : attr_value or
    // ELEMENT.<prop_name>.XMLATTR.ATTR_IRSA_VALUE : prop.value

    // where <prop_name> is the value of prop's ATTR_NAME attribute and is
    // assumed to be non-empty for INFO elements, of which we might have many.
    //(We can't yet convert to FITS format VOTables with more than one RESOURCE.
    // 07Dec20)

    Labeled_Properties combined_labeled_properties;
    std::string prev_keyword = "";
    std::string prev_label = "";
    Property prop;

    for (auto &kwd : table_extension.keyWord()) {
        std::string keyword(kwd.first);
        std::string kwd_value;

        // Annoyingly, CCfits does not have a way to just return the
        // value.  You have to give it something to put it in.
        get_keyword_value_as_string(kwd_value, kwd.second);

        // If `kwd` was generated from a labeled_property by
        // write_fits(), then the `keyword` string includes the value
        // of ATTR_NAME for that property.  We group together all kwds
        // with the same keyword and construct from them a
        // labeled_property whose label is `label`.

        // Set to defaults and adjust as needed.
        std::string label(keyword);
        std::string name(keyword);

        // Used to undo write_fits() hackery.
        static std::regex attr_expr{"^(.*)\\." + XMLATTR + "\\." + "(.*)$"};
        static std::regex info_expr{"^((?:.*\\.)?" + INFO + ")" + "\\." + "(.*)$"};

        bool convert_value_to_attr = false;

        std::smatch attr_match;
        if (std::regex_search(keyword, attr_match, attr_expr)) {
            // Undo write_fits() hackery: extract keyword and attrname from
            // "keyword.XMLATTR.name".
            keyword.assign(attr_match[1]);
            name.assign(attr_match[2]);
            label.assign(keyword);

            // If keyword indicates that we are looking at an INFO element,
            // further undo hackery by dropping everything past INFO from label.
            std::smatch info_match;
            if (std::regex_search(keyword, info_match, info_expr)) {
                label.assign(info_match[1]);
            } else {
                // Otherwise, prepare for an attribute at the level of e.g. RESOURCE or
                // TABLE.
                label += DOT + XMLATTR;
            }

        } else {
            // Plain old keyword straight from FITS (not via tablator's write_fits()).
            if (std::find(fits_ignored_keywords.begin(), fits_ignored_keywords.end(),
                          name) != fits_ignored_keywords.end()) {
                continue;
            }
            // Prepare to store it with ATTR_IRSA_VALUE as an attribute.
            convert_value_to_attr = true;
        }

        if (keyword != prev_keyword) {
            // Save the prop-in-progress with the appropriate label and prepare to move
            // on.
            if (!prev_label.empty() && !prop.empty()) {
                combined_labeled_properties.emplace_back(
                        std::make_pair(prev_label, prop));
                prop.clear();
            }

            prev_keyword.assign(keyword);
            prev_label.assign(label);
        }

        if (convert_value_to_attr) {
            prop.add_attribute(ATTR_VALUE, kwd_value);
        } else if (name == tablator::ATTR_IRSA_VALUE) {
            // if kwd came from FITS-ified labeled_properties.value_ via write_fits()
            prop.set_value(kwd_value);
        } else {
            prop.add_attribute(name, kwd_value);
        }

        // Concoct UCD attributes from names which are keys of keyword_ucd_mapping.
        auto i = keyword_ucd_mapping.find(name);
        if (i != keyword_ucd_mapping.end()) {
            // JTODO only if no other UCD attr is present for this keyword?
            prop.add_attribute("ucd", i->second);
        }

        if (!kwd.second->comment().empty()) {
            prop.add_attribute("comment", kwd.second->comment());
        }
    }

    // Add the final prop, if appropriate.
    if (!prev_label.empty() && !prop.empty()) {
        combined_labeled_properties.emplace_back(std::make_pair(prev_label, prop));
    }

    // Distribute the labeled_properties between assorted class members at assorted
    // levels.
    Labeled_Properties resource_element_labeled_properties;
    std::vector<Property> resource_element_trailing_infos;
    ATTRIBUTES resource_element_attributes;
    std::vector<Property> table_element_trailing_infos;
    ATTRIBUTES table_element_attributes;
    distribute_metadata(resource_element_labeled_properties,
                        resource_element_trailing_infos, resource_element_attributes,
                        table_element_trailing_infos, table_element_attributes,
                        combined_labeled_properties);

    // Tablator columns are 0-based and FITS columns are 1-based.  The
    // tablator table has a null_bitfield_flags column in the 0th
    // place.  The FITS file might have a null_bitfield_flags column
    // as its 1st column if it was generated by a previous (misguided)
    // version of tablator.

    const bool has_null_bitfield_flags(ccfits_table->column().size() > 0 &&
                                       ccfits_table->column(1).name() ==
                                               null_bitfield_flags_name &&
                                       ccfits_table->column(1).type() == CCfits::Tbyte);

    if (!has_null_bitfield_flags) {
        tablator::append_column(columns, offsets, null_bitfield_flags_name,
                                Data_Type::UINT8_LE,
                                bits_to_bytes(ccfits_table->column().size()),
                                Field_Properties(null_bitfield_flags_description, {}));
    }

    for (size_t i = 0; i < ccfits_table->column().size(); ++i) {
        size_t fits_col_idx = i + 1;
        CCfits::Column &c = ccfits_table->column(fits_col_idx);
        size_t array_size = get_array_size(c);

        // Negative type indicates array-valued column.
        switch (abs(c.type())) {
            case CCfits::Tlogical:
                tablator::append_column(columns, offsets, c.name(), Data_Type::INT8_LE,
                                        array_size);
                break;
            case CCfits::Tbyte:
                tablator::append_column(columns, offsets, c.name(), Data_Type::UINT8_LE,
                                        array_size);
                break;
            case CCfits::Tshort: {
                tablator::append_column(columns, offsets, c.name(), Data_Type::INT16_LE,
                                        array_size);
            } break;
            case CCfits::Tushort: {
                tablator::append_column(columns, offsets, c.name(),
                                        Data_Type::UINT16_LE, array_size);
            } break;
            case CCfits::Tint: {
                tablator::append_column(columns, offsets, c.name(), Data_Type::INT32_LE,
                                        array_size);
            } break;
            case CCfits::Tuint: {
                tablator::append_column(columns, offsets, c.name(),
                                        Data_Type::UINT32_LE, array_size);
            } break;
            case CCfits::Tlong: {
                // The Tlong type code is used for 32-bit integer columns when reading.
                tablator::append_column(columns, offsets, c.name(), Data_Type::INT32_LE,
                                        array_size);
            } break;
            case CCfits::Tulong: {
                // The Tulong type code is used for 32-bit unsigned integer columns when
                // reading.
                tablator::append_column(columns, offsets, c.name(),
                                        Data_Type::UINT32_LE, array_size);
            } break;
            case CCfits::Tlonglong: {
                tablator::append_column(columns, offsets, c.name(), Data_Type::INT64_LE,
                                        array_size);
            } break;
            case CCfits::Tfloat: {
                Field_Properties nan_nulls;
                nan_nulls.get_values().null =
                        std::to_string(std::numeric_limits<float>::quiet_NaN());
                tablator::append_column(columns, offsets, c.name(),
                                        Data_Type::FLOAT32_LE, array_size, nan_nulls);
            } break;
            case CCfits::Tdouble: {
                Field_Properties nan_nulls;
                nan_nulls.get_values().null =
                        std::to_string(std::numeric_limits<double>::quiet_NaN());
                tablator::append_column(columns, offsets, c.name(),
                                        Data_Type::FLOAT64_LE, array_size, nan_nulls);
            } break;
            case CCfits::Tstring:
                tablator::append_column(columns, offsets, c.name(), Data_Type::CHAR,
                                        c.width());
                break;
            default:
                throw std::runtime_error(
                        "Appending columns, unsupported data type in the fits file for "
                        "column '" +
                        c.name() + "'");
        }
    }

    // ccfits_table->rows () returns an int, so there may be issues with more
    // than 2^32 rows

    std::vector<uint8_t> data;
    size_t row_size = tablator::row_size(offsets);
    data.resize(ccfits_table->rows() * row_size);


    // CCfits dies in read_column() if there is no data in the table. :(
    if (!data.empty()) {
        fitsfile *fits_pointer = fits.fitsPointer();

        // Tablator columns are 0-based and FITS columns are 1-based.
        // The tablator table has a null_bitfield_flags column in the
        // 0th place.  The code that follows allows for the unlikely
        // possibility that the FITS table has a null_bitfield_flags
        // column in the 1st place.  In that case, the index of each
        // column of the FITS table is 1 more than the index of its
        // counterpart in the tablator table.  if the FITS file does
        // not have a null_bitfield_flags column, then the indices of
        // corresponding columns in the two tables are the same.

        // Warning: This code does not properly handle nulls.

        size_t col_idx_adjuster = (has_null_bitfield_flags) ? 0 : 1;

        //  We'll populate the tablator table column by column.
        for (size_t i = 0; i < ccfits_table->column().size(); ++i) {
            size_t fits_col_idx = i + 1;
            size_t tab_col_idx = i + col_idx_adjuster;
            size_t offset(offsets[tab_col_idx]);

            CCfits::Column &c = ccfits_table->column(fits_col_idx);
            bool is_array = (get_array_size(c) > 1);
            switch (abs(c.type())) {
                case CCfits::Tlogical: {
                    if (!is_array) {
                        std::vector<int> v;
                        c.read(v, 1, ccfits_table->rows());
                        size_t element_offset = offset;
                        for (auto &element : v) {
                            data[element_offset] = element;
                            element_offset += row_size;
                        }
                    } else {
                        // FIXME: Use the C api because Column::readArrays is
                        // horrendously slow.
                        std::vector<std::valarray<int>> v;
                        c.readArrays(v, 1, ccfits_table->rows());
                        size_t start_offset_within_row = offset;
                        for (auto &array : v) {
                            auto element_offset = start_offset_within_row;
                            for (auto &element : array) {
                                data[element_offset] = element;
                                ++element_offset;
                            }
                            start_offset_within_row += row_size;
                        }
                    }
                } break;

                case CCfits::Tbyte: {
                    read_column<uint8_t>(fits_pointer, data.data() + offset, c,
                                         is_array, ccfits_table->rows(), row_size);
                } break;
                case CCfits::Tshort: {
                    read_column<int16_t>(fits_pointer, data.data() + offset, c,
                                         is_array, ccfits_table->rows(), row_size);
                } break;
                case CCfits::Tushort: {
                    read_column<uint16_t>(fits_pointer, data.data() + offset, c,
                                          is_array, ccfits_table->rows(), row_size);
                } break;
                case CCfits::Tuint:
                case CCfits::Tulong: {
                    read_column<uint32_t>(fits_pointer, data.data() + offset, c,
                                          is_array, ccfits_table->rows(), row_size);
                } break;
                case CCfits::Tint:
                case CCfits::Tlong: {
                    read_column<int32_t>(fits_pointer, data.data() + offset, c,
                                         is_array, ccfits_table->rows(), row_size);
                } break;
                case CCfits::Tlonglong: {
                    read_column<int64_t>(fits_pointer, data.data() + offset, c,
                                         is_array, ccfits_table->rows(), row_size);
                } break;
                case CCfits::Tfloat: {
                    read_column<float>(fits_pointer, data.data() + offset, c, is_array,
                                       ccfits_table->rows(), row_size);
                } break;
                case CCfits::Tdouble: {
                    read_column<double>(fits_pointer, data.data() + offset, c, is_array,
                                        ccfits_table->rows(), row_size);
                } break;
                case CCfits::Tstring: {
                    std::vector<std::string> v;
                    c.read(v, 1, ccfits_table->rows());
                    size_t element_offset = offset;
                    for (auto &element : v) {
                        for (size_t j = 0; j < element.size(); ++j)
                            data[element_offset + j] = element[j];
                        for (int j = element.size(); j < c.width(); ++j)
                            data[element_offset + j] = '\0';
                        element_offset += row_size;
                    }
                } break;
                default:
                    throw std::runtime_error(
                            "Loading columns, unsupported data type in the fits file "
                            "for "
                            "column " +
                            c.name());
            }
            // FIXME: This should get the comment, but the comment()
            // function is protected???
            if (!c.unit().empty()) {
                columns[tab_col_idx].get_field_properties().set_attributes(
                        {{"unit", c.unit()}});
            }
        }
    }
    const auto table_element =
            Table_Element::Builder(columns, offsets, data)
                    .add_trailing_info_list(table_element_trailing_infos)
                    .add_attributes(table_element_attributes)
                    .build();
    add_resource_element(
            Resource_Element::Builder(table_element)
                    .add_labeled_properties(resource_element_labeled_properties)
                    .add_trailing_info_list(resource_element_trailing_infos)
                    .add_attributes(resource_element_attributes)
                    .build());
}
