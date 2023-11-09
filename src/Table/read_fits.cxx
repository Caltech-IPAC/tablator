#include <regex>
#include <unordered_map>

#include <CCfits/CCfits>

#include "../Table.hxx"
#include "../fits_keyword_ucd_mapping.hxx"

// Note: ccfits_table->rows () returns an int, so there may be issues
// with FITS files containing more than 2^32 rows.

// JTODO Descriptions and field-level attributes get lost in conversion to and from
// fits.

namespace {

//====================================================
// Helper function for reading keywords
//====================================================


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

//====================================================
// Helper functions/classes for parsing binary data
//====================================================

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


//=====================================================
//====================================================

// Helper class that stores column-level metadata for later use by
// functions that read and store binary data.

class Column_Info_Manager {
public:
    // This un-templatized version is called for data_types for which
    // CCfits::getNullValue() is not defined.
    void store_data_type_info_for_column(tablator::Data_Type tab_data_type,
                                         size_t array_size) {
        column_info_list_.emplace_back(tab_data_type, array_size, false /* got_null */,
                                       0 /* null_list_idx */);
    }

    //=====================================================

    template <typename T>
    void store_info_for_column(const CCfits::Column &fits_col,
                               tablator::Data_Type tab_data_type, size_t array_size) {
        T null_value = 0;
        bool got_null = false;
        if (is_null_value_supported(tab_data_type)) {
            // CCCfits::getNullValue() is surprisingly slow, in my
            // limited experience.  Storing its value as we do here
            // allows us to call this function only once for each column
            // rather than once for every combination of column and row.
            got_null = fits_col.getNullValue(&null_value);
        }
        size_t null_list_idx = 0;
        if (got_null) {
            null_list_idx = store_null_value_for_type(tab_data_type, null_value);
        }
        column_info_list_.emplace_back(tab_data_type, array_size, got_null,
                                       null_list_idx);
    }

    //=====================================================

    template <typename T>
    void retrieve_info_for_column(tablator::Data_Type &tab_data_type,
                                  size_t &array_size, bool &got_null, T &null_value,
                                  size_t tab_col_idx) const {
        // The vector column_info_list_ is indexed from 0, but the
        // non-null_bitfield_flags columns of the tablator table are
        // indexed from 1.
        const Column_Info &col_info = column_info_list_.at(tab_col_idx - 1);
        tab_data_type = col_info.tab_data_type_;
        array_size = col_info.array_size_;
        got_null = col_info.got_null_;
        if (got_null) {
            null_value = retrieve_null_value_for_type<T>(tab_data_type,
                                                         col_info.null_list_idx_);
        }
    }

    //=====================================================

    size_t retrieve_array_size_for_column(size_t tab_col_idx) const {
        const Column_Info &col_info = column_info_list_.at(tab_col_idx - 1);
        return col_info.array_size_;
    }

private:
    //=====================================================

    bool is_null_value_supported(tablator::Data_Type data_type) const {
        return (data_type >= tablator::Data_Type::UINT8_LE &&
                data_type <= tablator::Data_Type::UINT64_LE);
    }

    //=====================================================

    template <typename T>
    size_t store_null_value_for_type(tablator::Data_Type tab_data_type, T null_value) {
        size_t idx = 0;
        switch (tab_data_type) {
            case tablator::Data_Type::UINT8_LE: {
                idx = uint8_null_values_.size();
                uint8_null_values_.emplace_back(null_value);
            } break;
            case tablator::Data_Type::INT16_LE: {
                idx = int16_null_values_.size();
                int16_null_values_.emplace_back(null_value);
            } break;
            case tablator::Data_Type::UINT16_LE: {
                idx = uint16_null_values_.size();
                uint16_null_values_.emplace_back(null_value);
            } break;
            case tablator::Data_Type::INT32_LE: {
                idx = int32_null_values_.size();
                int32_null_values_.emplace_back(null_value);
            } break;
            case tablator::Data_Type::UINT32_LE: {
                idx = uint32_null_values_.size();
                uint32_null_values_.emplace_back(null_value);
            } break;
            case tablator::Data_Type::INT64_LE: {
                idx = int64_null_values_.size();
                int64_null_values_.emplace_back(null_value);
            } break;
            default:
                throw std::runtime_error("Unknown tablator data type");
        }
        return idx;
    }

    template <typename T>
    T retrieve_null_value_for_type(tablator::Data_Type tab_data_type,
                                   size_t null_list_idx) const {
        switch (tab_data_type) {
            case tablator::Data_Type::UINT8_LE: {
                return uint8_null_values_.at(null_list_idx);
            } break;
            case tablator::Data_Type::INT16_LE: {
                return int16_null_values_.at(null_list_idx);
            } break;
            case tablator::Data_Type::UINT16_LE: {
                return uint16_null_values_.at(null_list_idx);
            } break;
            case tablator::Data_Type::INT32_LE: {
                return int32_null_values_.at(null_list_idx);
            } break;
            case tablator::Data_Type::UINT32_LE: {
                return uint32_null_values_.at(null_list_idx);
            } break;
            case tablator::Data_Type::INT64_LE: {
                return int64_null_values_.at(null_list_idx);
            } break;
            default:
                throw std::runtime_error(
                        "tablator data type not supported for this function.XXX");
        }
        return 0;
    }

    //=====================================================

    struct Column_Info {
        Column_Info(tablator::Data_Type tdt, size_t as, bool gn, size_t nli)
                : tab_data_type_(tdt),
                  array_size_(as),
                  got_null_(gn),
                  null_list_idx_(nli) {}

        tablator::Data_Type tab_data_type_;
        size_t array_size_;
        bool got_null_;
        size_t null_list_idx_;
    };

    // Storage for type-specific per-column null_values
    std::vector<uint8_t> uint8_null_values_;
    std::vector<int16_t> int16_null_values_;
    std::vector<uint16_t> uint16_null_values_;
    std::vector<int32_t> int32_null_values_;
    std::vector<uint32_t> uint32_null_values_;
    std::vector<int64_t> int64_null_values_;


    std::vector<Column_Info> column_info_list_;
};


//====================================================
//====================================================


class Row_Column_Element_Reader {
public:
    Row_Column_Element_Reader(const Column_Info_Manager &col_info_manager,
                              const std::vector<size_t> &offsets)
            : col_info_manager_(col_info_manager), offsets_(offsets){};


    template <typename T>
    void read_element_given_column_and_row(tablator::Row &row, fitsfile *fits_file,
                                           const CCfits::Column &fits_col,
                                           size_t fits_row_idx, size_t tab_col_idx) {
        // Use the C api because the C++ api (Column::readArrays) is
        // horrendously slow.
        tablator::Data_Type tab_data_type;
        size_t array_size;
        bool got_null = false;
        T null_value = 0;
        col_info_manager_.retrieve_info_for_column(tab_data_type, array_size, got_null,
                                                   null_value, tab_col_idx);

        size_t col_offset = offsets_[tab_col_idx];
        size_t next_col_offset = offsets_[tab_col_idx + 1];

        auto get_matched_datatype = CCfits::FITSUtil::MatchType<T>();
        char *curr_ptr = row.get_data().data() + col_offset;

        std::vector<T> temp_array(array_size);
        int status = 0;
        int anynul = 0;

        // Note: As of 13Oct23, setting nulval to tablator::get_null() results in
        // extraneous nulls.
        fits_read_col(fits_file, get_matched_datatype(), fits_col.index(), fits_row_idx,
                      1, array_size, NULL /* nulval */, temp_array.data(), &anynul,
                      &status);

        if (anynul) {
            // Indicate that all array_size values are null.

            // Note: As of 13Oct23, the only value I have seen for anynul is 0.
            row.set_null(tab_data_type, array_size, tab_col_idx, col_offset,
                         next_col_offset);
        } else {
            for (size_t array_offset = 0; array_offset < array_size; ++array_offset) {
                if (got_null && temp_array[array_offset] == null_value) {
                    // Indicate that a single value in the array is null.
                    row.set_null(tab_data_type, 1 /* array_size */, tab_col_idx,
                                 col_offset + (array_offset * sizeof(T)),
                                 next_col_offset);
                } else {
                    *reinterpret_cast<T *>(curr_ptr) = temp_array[array_offset];
                    curr_ptr += sizeof(T);
                }
            }
        }
    }

    //====================================================

    // Note: FITS thinks of elements of Tstring columns as strings---or
    // possibly arrays of strings---but tablator thinks of each such
    // element as a single character array, not an array of arrays.
    // This function does not check whether individual elements of that
    // single array are undefined/null.
    void read_string_given_column_and_row(tablator::Row &row, fitsfile *fits_file,
                                          const CCfits::Column &fits_col,
                                          size_t fits_row_idx, size_t tab_col_idx) {
        // array_size is the size of the column in bytes/chars.
        size_t array_size =
                col_info_manager_.retrieve_array_size_for_column(tab_col_idx);

        size_t col_offset = offsets_[tab_col_idx];
        size_t next_col_offset = offsets_[tab_col_idx + 1];

        // If this string column is a vector column, all of the array
        // elements (henceforth "substrings") have the same size: the
        // value of CCfits::Column::width().
        size_t substring_size = fits_col.width();
        size_t num_substrings = array_size / substring_size;

        if (array_size != num_substrings * substring_size) {
            throw std::runtime_error(
                    "array_size is not an exact multiple of substring_size.");
        }

        // fits_read_col_str() wants a char** argument.  Create such an
        // element using vectors so as not to have to deal with memory
        // allocation and deallocation.
        std::vector<char *> ptr_vec;

        std::vector<std::vector<char> > data_vec(num_substrings,
                                                 std::vector<char>(substring_size + 1));
        for (size_t i = 0; i < num_substrings; ++i) {
            ptr_vec.emplace_back(data_vec.at(i).data());
        }
        // Finally, the char**.
        char **data_str = ptr_vec.data();

        tablator::Data_Type tab_col_type = tablator::Data_Type::CHAR;
        char nulstr[] = "";
        int status = 0;
        int anynul = 0;

        fits_read_col_str(fits_file, fits_col.index(), fits_row_idx, 1 /* firstelem */,
                          num_substrings /* nelements */, nulstr, data_str, &anynul,
                          &status);

        if (anynul) {
            row.set_null(tab_col_type, fits_col.repeat(), tab_col_idx, col_offset,
                         next_col_offset);
        } else {
            char *current = row.get_data().data() + col_offset;
            for (size_t i = 0; i < num_substrings; ++i) {
                char *element = data_str[i];
                size_t elt_length = strlen(element);
                size_t j = 0;
                for (/* */; j < elt_length && j < substring_size; ++j) {
                    *(current + j) = *(element + j);
                }
                for (/* */; j < substring_size; ++j) {
                    *(current + j) = '\0';
                }
            }
        }
    }

private:
    const Column_Info_Manager &col_info_manager_;
    const std::vector<size_t> &offsets_;
};
//====================================================

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

    //*********************************
    // Read and store keywords
    //*********************************

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
        static std::regex attr_regex{"^(.*)\\." + XMLATTR + "\\." + "(.*)$"};
        static std::regex info_regex{"^((?:.*\\.)?" + INFO + ")" + "\\." + "(.*)$"};

        bool convert_value_to_attr = false;

        std::smatch attr_match;
        if (std::regex_search(keyword, attr_match, attr_regex)) {
            // Undo write_fits() hackery: extract keyword and attrname from
            // "keyword.XMLATTR.name".
            keyword.assign(attr_match[1]);
            name.assign(attr_match[2]);
            label.assign(keyword);

            // If keyword indicates that we are looking at an INFO element,
            // further undo hackery by dropping everything past INFO from label.
            std::smatch info_match;
            if (std::regex_search(keyword, info_match, info_regex)) {
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


    //*********************************
    // Read and store binary data
    //*********************************

    // We make two passes through the FITS file's columns.  The first
    // pass is to extract column metadata and create tablator columns;
    // the second pass is to read and store data.

    // Create null_bitfield_flags column for internal use.
    tablator::append_column(columns, offsets, null_bitfield_flags_name,
                            Data_Type::UINT8_LE,
                            bits_to_bytes(ccfits_table->column().size()),
                            Field_Properties(null_bitfield_flags_description, {}));


    // Check whether the FITS file already has a null_bitfield_flag
    // column and if so, prepare to skip it; we'll populate the column
    // we just created instead.  (The FITS file would have this column
    // only if the file had been generated by an earlier (misguided)
    // version of tablator.)

    const bool has_null_bitfield_flags(ccfits_table->column().size() > 0 &&
                                       ccfits_table->column(1).name() ==
                                               null_bitfield_flags_name &&
                                       ccfits_table->column(1).type() == CCfits::Tbyte);


    Column_Info_Manager col_info_manager;

    for (size_t fits_col_idx = 1; fits_col_idx <= ccfits_table->column().size();
         ++fits_col_idx) {
        CCfits::Column &fits_col = ccfits_table->column(fits_col_idx);

        if (fits_col_idx == 1 && has_null_bitfield_flags) {
            // Skip the null_bitfield_flags column, if it exists.
            continue;
        }
        size_t array_size = get_array_size(fits_col);

        // Negative type indicates array-valued column.
        int abs_fits_type = abs(fits_col.type());

        switch (abs_fits_type) {
            case CCfits::Tlogical: {
                // std::cout << "Tlogical: " << CCfits::Tlogical << std::endl;
                tablator::append_column(columns, offsets, fits_col.name(),
                                        Data_Type::INT8_LE, array_size);
                col_info_manager.store_data_type_info_for_column(Data_Type::UINT8_LE,
                                                                 array_size);
            } break;
            case CCfits::Tbyte:
                // std::cout << "Tbyte: " << CCfits::Tbyte << std::endl;
                tablator::append_column(columns, offsets, fits_col.name(),
                                        Data_Type::UINT8_LE, array_size);
                col_info_manager.store_info_for_column<uint8_t>(
                        fits_col, Data_Type::UINT8_LE, array_size);
                break;
            case CCfits::Tshort: {
                // std::cout << "Tshort: " << CCfits::Tshort << std::endl;
                tablator::append_column(columns, offsets, fits_col.name(),
                                        Data_Type::INT16_LE, array_size);
                col_info_manager.store_info_for_column<int16_t>(
                        fits_col, Data_Type::INT16_LE, array_size);
            } break;
            case CCfits::Tushort: {
                // std::cout << "Tushort: " << CCfits::Tushort << std::endl;
                tablator::append_column(columns, offsets, fits_col.name(),
                                        Data_Type::UINT16_LE, array_size);
                col_info_manager.store_info_for_column<uint16_t>(
                        fits_col, Data_Type::UINT16_LE, array_size);
            } break;
            case CCfits::Tint: {
                tablator::append_column(columns, offsets, fits_col.name(),
                                        Data_Type::INT32_LE, array_size);
                col_info_manager.store_info_for_column<int32_t>(
                        fits_col, Data_Type::INT32_LE, array_size);
            } break;
            case CCfits::Tuint: {
                tablator::append_column(columns, offsets, fits_col.name(),
                                        Data_Type::UINT32_LE, array_size);
                col_info_manager.store_info_for_column<uint32_t>(
                        fits_col, Data_Type::UINT32_LE, array_size);
            } break;
            case CCfits::Tlong: {
                // The Tlong type code is used for 32-bit integer columns when reading.
                tablator::append_column(columns, offsets, fits_col.name(),
                                        Data_Type::INT32_LE, array_size);
                col_info_manager.store_info_for_column<int32_t>(
                        fits_col, Data_Type::INT32_LE, array_size);
            } break;
            case CCfits::Tulong: {
                // The Tulong type code is used for 32-bit unsigned integer columns when
                // reading.
                tablator::append_column(columns, offsets, fits_col.name(),
                                        Data_Type::UINT32_LE, array_size);
                col_info_manager.store_info_for_column<uint32_t>(
                        fits_col, Data_Type::UINT32_LE, array_size);
            } break;
            case CCfits::Tlonglong: {
                tablator::append_column(columns, offsets, fits_col.name(),
                                        Data_Type::INT64_LE, array_size);
                col_info_manager.store_info_for_column<int64_t>(
                        fits_col, Data_Type::INT64_LE, array_size);
            } break;
            case CCfits::Tfloat: {
                Field_Properties nan_nulls;
                nan_nulls.get_values().null =
                        std::to_string(std::numeric_limits<float>::quiet_NaN());
                tablator::append_column(columns, offsets, fits_col.name(),
                                        Data_Type::FLOAT32_LE, array_size, nan_nulls);
                col_info_manager.store_data_type_info_for_column(Data_Type::FLOAT32_LE,
                                                                 array_size);
            } break;
            case CCfits::Tdouble: {
                Field_Properties nan_nulls;
                nan_nulls.get_values().null =
                        std::to_string(std::numeric_limits<double>::quiet_NaN());
                tablator::append_column(columns, offsets, fits_col.name(),
                                        Data_Type::FLOAT64_LE, array_size, nan_nulls);
                col_info_manager.store_data_type_info_for_column(Data_Type::FLOAT64_LE,
                                                                 array_size);
            } break;
            case CCfits::Tstring:
                tablator::append_column(columns, offsets, fits_col.name(),
                                        Data_Type::CHAR, fits_col.width());
                col_info_manager.store_data_type_info_for_column(Data_Type::CHAR,
                                                                 array_size);
                break;
            default:
                throw std::runtime_error(
                        "Appending columns, unsupported data type in the fits file for "
                        "column '" +
                        fits_col.name() + "'");
        }
    }

    Row_Column_Element_Reader element_reader(col_info_manager, offsets);

    size_t row_size = tablator::row_size(offsets);
    size_t num_rows = ccfits_table->rows();

    std::vector<uint8_t> data;
    data.reserve(row_size * num_rows);

    // CCfits dies in read_element_given_column_and_row() if there is no data in the
    // table. :(

    if (row_size > 0 && num_rows > 0) {
        fitsfile *fits_pointer = fits.fitsPointer();

        // Tablator columns are 0-based and FITS columns are 1-based.
        // The tablator table has a null_bitfield_flags column in the
        // 0th place.  That column will be populated as a side effect
        // of populating the other columns of the tablator table.

        // If the FITS file was generated by a previous (misguided)
        // version of tablator, it has its own null_bitfield_flags
        // column in the 1st place.  In that case, the index of each
        // column of the FITS file is 1 more than the index of its
        // counterpart in the tablator table.  if the FITS file does
        // not have a null_bitfield_flags column, then the indices of
        // corresponding columns in the two tables are the same.

        // The FITS file's null_bitfield_flags column, if it exists,
        // is skipped by the code below which retrieves and stores
        // data from the actual data-bearing columns of the FITS file.

        size_t col_idx_adjuster = (has_null_bitfield_flags) ? 0 : 1;

        for (size_t j = 0; j < num_rows; ++j) {
            size_t fits_row_idx = j + 1;

            // Row by row, we'll populate <curr_row> and append it to <data>.
            Row curr_row(row_size);

            curr_row.fill_with_zeros();

            for (size_t i = 0; i < ccfits_table->column().size(); ++i) {
                size_t fits_col_idx = i + 1;

                if (fits_col_idx == 1 && has_null_bitfield_flags) {
                    // Skip the null_bitfield_flags column, if it exists.
                    continue;
                }

                size_t tab_col_idx = fits_col_idx - 1 + col_idx_adjuster;

                CCfits::Column &fits_col = ccfits_table->column(fits_col_idx);
                switch (abs(fits_col.type())) {
                    case CCfits::Tlogical: {
                        // Use template type uint8_t here; FITS doesn't support int8_t.
                        // null_value is not defined for this type.
                        element_reader.read_element_given_column_and_row<uint8_t>(
                                curr_row, fits_pointer, fits_col, fits_row_idx,
                                tab_col_idx);
                    } break;
                    case CCfits::Tbyte: {
                        element_reader.read_element_given_column_and_row<uint8_t>(
                                curr_row, fits_pointer, fits_col, fits_row_idx,
                                tab_col_idx);
                    } break;
                    case CCfits::Tshort: {
                        // std::cout << "Tshort" << std::endl;
                        element_reader.read_element_given_column_and_row<int16_t>(
                                curr_row, fits_pointer, fits_col, fits_row_idx,
                                tab_col_idx);
                    } break;
                    case CCfits::Tushort: {
                        // std::cout << "Tushort" << std::endl;
                        element_reader.read_element_given_column_and_row<uint16_t>(
                                curr_row, fits_pointer, fits_col, fits_row_idx,
                                tab_col_idx);
                    } break;
                    case CCfits::Tint:
                    case CCfits::Tlong: {
                        // std::cout << "Tint or Tlong" << std::endl;
                        element_reader.read_element_given_column_and_row<int32_t>(
                                curr_row, fits_pointer, fits_col, fits_row_idx,
                                tab_col_idx);
                    } break;
                    case CCfits::Tuint:
                    case CCfits::Tulong: {
                        // std::cout << "Tuint or Tulong" << std::endl;
                        element_reader.read_element_given_column_and_row<uint32_t>(
                                curr_row, fits_pointer, fits_col, fits_row_idx,
                                tab_col_idx);
                    } break;
                    case CCfits::Tlonglong: {
                        // std::cout << "Tlonglong" << std::endl;
                        element_reader.read_element_given_column_and_row<int64_t>(
                                curr_row, fits_pointer, fits_col, fits_row_idx,
                                tab_col_idx);
                    } break;
                    case CCfits::Tfloat: {
                        // std::cout << "Tfloat" << std::endl;
                        element_reader.read_element_given_column_and_row<float>(
                                curr_row, fits_pointer, fits_col, fits_row_idx,
                                tab_col_idx);
                    } break;
                    case CCfits::Tdouble: {
                        // std::cout << "Tdouble" << std::endl;
                        element_reader.read_element_given_column_and_row<double>(
                                curr_row, fits_pointer, fits_col, fits_row_idx,
                                tab_col_idx);
                    } break;
                    case CCfits::Tstring: {
                        // std::cout << "Tstring" << std::endl;
                        // read_element_given_column_and_row() calls fits_read_col(),
                        // which is not supported for CCfits::Tstring.
                        element_reader.read_string_given_column_and_row(
                                curr_row, fits_pointer, fits_col, fits_row_idx,
                                tab_col_idx);
                    } break;
                    default:
                        throw std::runtime_error(
                                "Loading columns, unsupported data type in the fits "
                                "file for column " +
                                fits_col.name());
                }
                // FIXME: This should get the comment, but the comment()
                // function is protected???
                if (!fits_col.unit().empty()) {
                    columns[tab_col_idx].get_field_properties().set_attributes(
                            {{"unit", fits_col.unit()}});
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
