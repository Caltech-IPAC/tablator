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
void read_element_given_column_and_row(fitsfile *fits_file, std::vector<char> &data,
                                       size_t col_offset_within_row, CCfits::Column &c,
                                       size_t array_size, size_t &fits_row_idx) {
    char *start_pos = data.data() + col_offset_within_row;

    // Use the C api because the C++ api (Column::readArrays) is
    // horrendously slow.
    int status(0), anynul(0);
    std::vector<T> temp_array(array_size);

    auto get_matched_datatype = CCfits::FITSUtil::MatchType<T>();
    char *curr_pos = start_pos;

    fits_read_col(fits_file, get_matched_datatype(), c.index(), fits_row_idx, 1,
                  array_size, NULL /* void *nulval */, temp_array.data(), &anynul,
                  &status);

    for (size_t array_offset = 0; array_offset < array_size; ++array_offset) {
        *reinterpret_cast<T *>(curr_pos) = temp_array[array_offset];
        curr_pos += sizeof(T);
    }
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

    size_t row_size = tablator::row_size(offsets);
    size_t num_rows = ccfits_table->rows();

    // Row by row, we'll populate <curr_row> and append it to <data>.
    Row curr_row(row_size);
    std::vector<uint8_t> data;
    // JTODO  Row::data is vector<char> and Data_Element::data is vector<uint8_t>.

    // CCfits dies in read_element_given_column_and_row() if there is no data in the
    // table. :(
    if (row_size > 0 && num_rows > 0) {
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

        for (size_t j = 0; j < num_rows; ++j) {
            size_t fits_row_idx = j + 1;

            curr_row.fill_with_zeros();
            auto &curr_row_data = curr_row.get_data();

            for (size_t i = 0; i < ccfits_table->column().size(); ++i) {
                size_t fits_col_idx = i + 1;
                size_t tab_col_idx = i + col_idx_adjuster;
                size_t offset(offsets[tab_col_idx]);

                CCfits::Column &c = ccfits_table->column(fits_col_idx);
                size_t array_size = get_array_size(c);
                switch (abs(c.type())) {
                    case CCfits::Tlogical: {
                        if (array_size == 1) {
                            std::vector<int> v(1);
                            c.read(v, fits_row_idx, fits_row_idx);
                            auto &element = v.at(0);
                            curr_row_data[offset] = element;
                        } else {
                            // FIXME: Use the C api because Column::readArrays is
                            // horrendously slow.
                            std::valarray<int> array_vals;
                            c.read(array_vals, fits_row_idx);
                            auto element_offset = offset;
                            for (auto &element : array_vals) {
                                curr_row_data[element_offset] = element;
                                ++element_offset;
                            }
                        }
                    } break;
                    case CCfits::Tbyte: {
                        read_element_given_column_and_row<uint8_t>(
                                fits_pointer, curr_row_data, offset, c, array_size,
                                fits_row_idx);
                    } break;
                    case CCfits::Tshort: {
                        read_element_given_column_and_row<int16_t>(
                                fits_pointer, curr_row_data, offset, c, array_size,
                                fits_row_idx);
                    } break;
                    case CCfits::Tushort: {
                        read_element_given_column_and_row<uint16_t>(
                                fits_pointer, curr_row_data, offset, c, array_size,
                                fits_row_idx);
                    } break;
                    case CCfits::Tuint:
                    case CCfits::Tulong: {
                        read_element_given_column_and_row<uint32_t>(
                                fits_pointer, curr_row_data, offset, c, array_size,
                                fits_row_idx);
                    } break;
                    case CCfits::Tint:
                    case CCfits::Tlong: {
                        read_element_given_column_and_row<int32_t>(
                                fits_pointer, curr_row_data, offset, c, array_size,
                                fits_row_idx);
                    } break;
                    case CCfits::Tlonglong: {
                        read_element_given_column_and_row<int64_t>(
                                fits_pointer, curr_row_data, offset, c, array_size,
                                fits_row_idx);
                    } break;
                    case CCfits::Tfloat: {
                        read_element_given_column_and_row<float>(
                                fits_pointer, curr_row_data, offset, c, array_size,
                                fits_row_idx);
                    } break;
                    case CCfits::Tdouble: {
                        read_element_given_column_and_row<double>(
                                fits_pointer, curr_row_data, offset, c, array_size,
                                fits_row_idx);
                    } break;
                    case CCfits::Tstring: {
                        std::vector<std::string> v;
                        c.read(v, fits_row_idx, fits_row_idx);
                        auto &element = v.at(0);
                        for (size_t j = 0; j < element.size(); ++j) {
                            curr_row_data[offset + j] = element[j];
                        }
                        for (int j = element.size(); j < c.width(); ++j) {
                            curr_row_data[offset + j] = '\0';
                        }
                    } break;
                    default:
                        throw std::runtime_error(
                                "Loading columns, unsupported data type in the fits "
                                "file "
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
            tablator::append_row(data, curr_row);
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
