#pragma once

#include <array>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <tuple>

#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>

#include <H5Cpp.h>
#include <CCfits/CCfits>

#include "Ascii_Writer.hxx"
#include "Column.hxx"
#include "Command_Line_Options.hxx"
#include "Common.hxx"
#include "Data_Element.hxx"
#include "Field_Properties.hxx"
#include "Format.hxx"
#include "Group_Element.hxx"
#include "Ipac_Table_Writer.hxx"
#include "Property.hxx"
#include "Resource_Element.hxx"
#include "Row.hxx"
#include "Table_Element.hxx"
#include "Utils/Null_Utils.hxx"
#include "Utils/Table_Utils.hxx"

namespace tablator {

class Table {
public:
    friend Table add_counter_column(const Table &src_table,
                                    const std::string &col_name);
    friend Table combine_tables(const Table &src1, const Table &src2);

    static constexpr char const *FIXLEN_KEYWORD = "fixlen";
    static constexpr char const *ROWS_RETRIEVED_KEYWORD = "RowsRetrieved";
    static constexpr const char *DEFAULT_NULL_VALUE = "null";

private:
    struct Options {
        ATTRIBUTES attributes_;
        std::string description_;
        std::vector<std::string> comments_;
        std::vector<Field> params_;
        Labeled_Properties labeled_properties_;
        std::vector<Group_Element> group_elements_;
        std::vector<Property> trailing_info_list_;

        void set_attributes(
                const std::initializer_list<std::pair<const std::string, std::string>>
                        &attributes) {
            attributes_ = attributes;
        }

        void add_attributes(const ATTRIBUTES &attributes) {
            attributes_.insert(attributes.begin(), attributes.end());
        }

        void add_attribute(const STRING_PAIR &attr_pair) {
            attributes_.emplace(attr_pair);
        }

        void set_description(const std::string &description) {
            description_ = description;
        }

        void add_comments(const std::vector<std::string> &comments) {
            comments_.insert(comments_.end(), comments.begin(), comments.end());
        }

        void add_comment(const std::string &comment) {
            comments_.emplace_back(comment);
        }

        void set_params(const std::vector<Field> &params) { params_ = params; }

        void add_params(const std::vector<Field> &params) {
            params_.insert(params_.end(), params.begin(), params.end());
        }

        void add_params(const std::string &params_xml) {
            tablator::ptree_readers::add_params_from_xml_string(params_, params_xml);
        }

        void add_param(const Field &param) { params_.emplace_back(param); }

        void add_labeled_properties(const Labeled_Properties &labeled_properties) {
            labeled_properties_.insert(labeled_properties_.end(),
                                       labeled_properties.begin(),
                                       labeled_properties.end());
        }

        // For backward compatibility, Table implements its analogue of
        // this function itself rather than delegating to Options.
        void add_labeled_property(const Labeled_Property &labeled_property) {
            labeled_properties_.emplace_back(labeled_property);
        }

        void add_group_elements(const std::vector<Group_Element> &group_elements) {
            group_elements_.insert(group_elements_.end(), group_elements.begin(),
                                   group_elements.end());
        }

        void add_group_element(const Group_Element &group_element) {
            group_elements_.emplace_back(group_element);
        }

        void add_trailing_info_list(const std::vector<Property> &trailing_info_list) {
            trailing_info_list_.insert(trailing_info_list_.end(),
                                       trailing_info_list.begin(),
                                       trailing_info_list.end());
        }

        void add_trailing_info(const Property &trailing_info) {
            trailing_info_list_.emplace_back(trailing_info);
        }
    };  // end of Options struct


public:
    class Builder {
    public:
        // JTODO check that resource_elements contains at least one Table_Element?
        Builder(std::vector<Resource_Element> &resource_elements)
                : resource_elements_(resource_elements) {}

        Table build() { return Table(resource_elements_, options_); }

        Builder &set_attributes(
                const std::initializer_list<std::pair<const std::string, std::string>>
                        &attributes) {
            options_.set_attributes(attributes);
            return *this;
        }


        Builder &add_attributes(const ATTRIBUTES &attributes) {
            options_.add_attributes(attributes);
            return *this;
        }

        Builder &add_attribute(const STRING_PAIR &attr_pair) {
            options_.add_attribute(attr_pair);
            return *this;
        }

        Builder &add_description(const std::string &description) {
            options_.set_description(description);
            return *this;
        }

        Builder &add_comments(const std::vector<std::string> &comments) {
            options_.add_comments(comments);
            return *this;
        }

        Builder &add_comment(const std::string &comment) {
            options_.add_comment(comment);
            return *this;
        }

        Builder &add_params(const std::vector<Field> &params) {
            options_.params_ = params;
            return *this;
        }

        Builder &add_labeled_properties(const Labeled_Properties &labeled_properties) {
            options_.add_labeled_properties(labeled_properties);
            return *this;
        }

        Builder &add_group_elements(const std::vector<Group_Element> &group_elements) {
            options_.add_group_elements(group_elements);
            return *this;
        }

        Builder &add_trailing_info_list(
                const std::vector<Property> &trailing_info_list) {
            options_.add_trailing_info_list(trailing_info_list);
            return *this;
        }

    private:
        std::vector<Resource_Element> resource_elements_;
        Options options_;
    };  // end of Builder class


    // constructors
    Table(const std::vector<Column> &Columns,
          const std::map<std::string, std::string> &property_map);

    Table(const std::vector<Column> &Columns,
          const tablator::Labeled_Properties &property_pair_vec);
    Table(const std::vector<Column> &Columns)
            : Table(Columns, std::map<std::string, std::string>()) {}

    Table(const boost::filesystem::path &input_path, const Format &format);
    Table(const boost::filesystem::path &input_path) { read_unknown(input_path); }
    Table(std::istream &input_stream) { read_unknown(input_stream); }
    Table(std::istream &input_stream, const Format &format);


    // validators
    void validate_column_index(size_t col_idx) const {
        if (col_idx >= get_columns().size()) {
            throw std::runtime_error("Invalid column index " + std::to_string(col_idx) +
                                     ".");
        }
    }

    void validate_row_index(size_t row_idx) const {
        if (row_idx >= get_num_rows()) {
            throw std::runtime_error("Invalid row index  " + std::to_string(row_idx) +
                                     ".");
        }
    }


    // column accessors
    std::vector<Column>::const_iterator find_column(const std::string &name) const {
        const auto &columns = get_columns();
        return std::find_if(columns.begin(), columns.end(),
                            [&](const Column &c) { return c.get_name() == name; });
    }

    size_t get_column_index(const std::string &name) const {
        const auto column_iter = find_column(name);
        const auto &columns = get_columns();
        if (column_iter == columns.end()) {
            throw std::runtime_error("Unable to find column '" + name + "' in table.");
        }
        return std::distance(columns.begin(), column_iter);
    }

    size_t get_column_offset(size_t col_idx) const {
        validate_column_index(col_idx);
        return get_offsets().at(col_idx);
    }

    size_t get_column_offset(const std::string &name) const {
        auto col_idx = get_column_index(name);
        return get_offsets().at(col_idx);
    }

    size_t column_offset(size_t col_idx) const { return get_column_offset(col_idx); }
    size_t column_offset(const std::string &name) const {
        return get_column_offset(name);
    }

    std::vector<size_t> find_column_ids(
            const std::vector<std::string> &col_names) const {
        std::vector<size_t> col_ids;

        for (const std::string &col_name : col_names) {
            size_t col_id = get_column_index(col_name);
            col_ids.emplace_back(col_id);
        }
        return col_ids;
    }

    std::vector<size_t> find_omitted_column_ids(
            const std::vector<size_t> &col_ids) const {
        const auto &columns = get_columns();
        std::vector<size_t> all_col_ids(columns.size() - 1);
        std::iota(all_col_ids.begin(), all_col_ids.end(), 1);

        std::vector<size_t> sorted_col_ids(col_ids.begin(), col_ids.end());
        std::sort(sorted_col_ids.begin(), sorted_col_ids.end());

        std::vector<size_t> diff_vec;
        diff_vec.reserve(columns.size());
        std::set_difference(all_col_ids.begin(), all_col_ids.end(),
                            sorted_col_ids.begin(), sorted_col_ids.end(),
                            std::back_inserter(diff_vec));
        return diff_vec;
    }

    std::vector<size_t> find_omitted_column_ids(
            const std::vector<std::string> &col_names) const {
        std::vector<size_t> col_ids = find_column_ids(col_names);
        return find_omitted_column_ids(col_ids);
    }


    // table modifiers

    void append_row(const Row &row) {
	  assert(row.get_data().size() == get_row_size());
        tablator::append_row(get_data(), row);
    }

    void unsafe_append_row(const char *row) {
        tablator::unsafe_append_row(get_data(), row, get_row_size());
    }


    void append_rows(const Table &table2);

    // write functions

    void write(std::ostream &os, const std::string &table_name, const Format &format,
               const Command_Line_Options &options = default_options) const;
    void write(const boost::filesystem::path &path, const Format &format,
               const Command_Line_Options &options = default_options) const;
    void write(const boost::filesystem::path &path,
               const Command_Line_Options &options = default_options) const {
        write(path, Format(path), options);
    }
    void write_hdf5(std::ostream &os) const;
    void write_hdf5(const boost::filesystem::path &p) const;

    void write_ipac_table(std::ostream &os,
                          const Command_Line_Options &options = default_options) const {
        Ipac_Table_Writer::write(*this, os, options);
    }
    void write_ipac_table(const boost::filesystem::path &p,
                          const Command_Line_Options &options = default_options) const {
        boost::filesystem::ofstream os(p);
        write_ipac_table(os, options);
    }

    void write_ipac_subtable_by_row(
            std::ostream &os, std::vector<size_t> requested_row_ids,
            const Command_Line_Options &options = default_options) const {
        Ipac_Table_Writer::write_subtable_by_row(*this, os, requested_row_ids, options);
    }

    void write_ipac_subtable_by_column_and_row(
            std::ostream &os, const std::vector<size_t> &column_ids,
            std::vector<size_t> requested_row_ids,
            const Command_Line_Options options = default_options) const {
        Ipac_Table_Writer::write_subtable_by_column_and_row(*this, os, column_ids,
                                                            requested_row_ids, options);
    }

    void write_ipac_subtable_by_row(
            std::ostream &os, size_t start_row, size_t row_count,
            const Command_Line_Options &options = default_options) const {
        Ipac_Table_Writer::write_subtable_by_row(*this, os, start_row, row_count,
                                                 options);
    }

    void write_ipac_subtable_by_column_and_row(
            std::ostream &os, const std::vector<size_t> &column_ids, size_t start_row,
            size_t row_count,
            const Command_Line_Options options = default_options) const {
        Ipac_Table_Writer::write_subtable_by_column_and_row(
                *this, os, column_ids, start_row, row_count, options);
    }

    void write_ipac_subtable_by_column_and_row(
            std::ostream &os, const std::vector<size_t> &column_ids,
            const Command_Line_Options options = default_options) const {
        Ipac_Table_Writer::write_subtable_by_column_and_row(*this, os, column_ids, 0,
                                                            num_rows(), options);
    }

    void write_single_ipac_record(std::ostream &os, size_t row_idx,
                                  const Command_Line_Options &options) const {
        Ipac_Table_Writer::write_single_record(*this, os, row_idx, options);
    }

    void write_single_ipac_record(std::ostream &os,
                                  const std::vector<size_t> &included_column_ids,
                                  size_t row_idx,
                                  const Command_Line_Options &options) const {
        Ipac_Table_Writer::write_single_record(*this, os, included_column_ids, row_idx,
                                               options);
    }

    void write_consecutive_ipac_records(std::ostream &os, size_t start_row,
                                        size_t row_count,
                                        const Command_Line_Options &options) const {
        Ipac_Table_Writer::write_consecutive_records(*this, os, start_row, row_count,
                                                     options);
    }

    void write_consecutive_ipac_records(std::ostream &os,
                                        const std::vector<size_t> &included_column_ids,
                                        size_t start_row, size_t row_count,
                                        const Command_Line_Options &options) const {
        Ipac_Table_Writer::write_consecutive_records(*this, os, included_column_ids,
                                                     start_row, row_count, options);
    }

    void write_selected_ipac_records(std::ostream &os,
                                     std::vector<size_t> const &requested_row_ids,
                                     const Command_Line_Options &options) const {
        Ipac_Table_Writer::write_selected_records(*this, os, requested_row_ids,
                                                  options);
    }

    void write_selected_ipac_records(std::ostream &os,
                                     const std::vector<size_t> &included_column_ids,
                                     std::vector<size_t> const &requested_row_ids,
                                     const Command_Line_Options &options) const {
        Ipac_Table_Writer::write_selected_records(*this, os, included_column_ids,
                                                  requested_row_ids, options);
    }


    void write_dsv(std::ostream &os, const char &separator,
                   const Command_Line_Options &options = default_options) const;

    void write_sql_create_table(std::ostream &os, const std::string &table_name,
                                const Format::Enums &sql_type) const {
        using namespace std::string_literals;
        write_sql_create_table(os, table_name, sql_type, ""s, ""s);
    }
    void write_sql_create_table(std::ostream &os, const std::string &table_name,
                                const Format::Enums &sql_type,
                                bool is_nologging) const {
        using namespace std::string_literals;
        write_sql_create_table(os, table_name, sql_type, ""s, ""s, is_nologging);
    }
    void write_sql_create_table(std::ostream &os, const std::string &table_name,
                                const Format::Enums &sql_type,
                                const std::string &point_column_name,
                                const std::string &polygon_column_name) const {
        write_sql_create_table(os, table_name, sql_type, point_column_name,
                               polygon_column_name, false);
    }

    void write_sql_create_table(std::ostream &os, const std::string &table_name,
                                const Format::Enums &sql_type,
                                const std::string &point_column_name,
                                const std::string &polygon_column_name,
                                bool is_nologging) const;

    void write_sql_inserts(
            std::ostream &os, const std::string &table_name,
            const Command_Line_Options &options = default_options) const {
        write_sql_inserts(os, table_name, STRING_PAIR(), std::vector<STRING_PAIR>(),
                          options);
    }

    void write_sql_inserts(std::ostream &os, const std::string &table_name,
                           const STRING_PAIR &point_input_names,
                           const std::vector<STRING_PAIR> &polygon_input_names,
                           const Command_Line_Options &options) const;

    void write_sql(std::ostream &os, const std::string &table_name,
                   const Format::Enums &sql_type,
                   const Command_Line_Options &options) const {
        write_sql_create_table(os, table_name, sql_type);
        os << ";\n";
        write_sql_inserts(os, table_name, options);
    }
    void write_sqlite_db(const boost::filesystem::path &path,
                         const Command_Line_Options &options) const;

    void write_fits(std::ostream &os) const;

    void write_fits(const boost::filesystem::path &filename) const;

    void write_fits(fitsfile *fits_file) const;

    void write_tabledata(std::ostream &os, const Format::Enums &output_format,
                         const Command_Line_Options &options) const;

    void write_html(std::ostream &os, const Command_Line_Options &options) const;

    boost::property_tree::ptree generate_property_tree() const;

    void read_unknown(const boost::filesystem::path &path);
    void read_unknown(std::istream &input_stream);
    void read_ipac_table(std::istream &input_stream);
    void read_ipac_table(const boost::filesystem::path &path) {
        boost::filesystem::ifstream input_stream(path);
        read_ipac_table(input_stream);
    }
    void read_fits(const boost::filesystem::path &path);
    void read_hdf5(const boost::filesystem::path &path);
    void read_json5(std::istream &input_stream);
    void read_json5(const boost::filesystem::path &path) {
        boost::filesystem::ifstream input_stream(path);
        read_json5(input_stream);
    }
    void read_json(std::istream &input_stream);
    void read_json(const boost::filesystem::path &path) {
        boost::filesystem::ifstream input_stream(path);
        read_json(input_stream);
    }

    void read_votable(std::istream &input_stream);
    void read_votable(const boost::filesystem::path &path) {
        boost::filesystem::ifstream input_stream(path);
        read_votable(input_stream);
    }

    void read_dsv(std::istream &input_stream, const Format &format);
    void read_dsv(const boost::filesystem::path &path, const Format &format) {
        if (path == "-") {
            read_dsv(std::cin, format);
        } else {
            boost::filesystem::ifstream input_stream(path);
            read_dsv(input_stream, format);
        }
    }

    void read_dsv_rows(const std::list<std::vector<std::string>> &dsv) {
        set_data(read_dsv_rows(get_columns(), get_offsets(), dsv));
    }

    void set_column_info(std::list<std::vector<std::string>> &dsv) {
        set_column_info(get_columns(), get_offsets(), dsv);
    };


    // Following the VOTable convention, we use the most significant
    // bit for the first column.
    bool is_null_value(size_t row_idx, size_t col_idx) const {
        auto row_offset = row_idx * get_row_size();
        auto col_bit_offset = (col_idx - 1) / 8;
        auto pos = row_offset + col_bit_offset;
        if (pos >= get_data().size()) {
            throw std::runtime_error("invalid pos " + std::to_string(pos) +
                                     "; data size is " +
                                     std::to_string(get_data().size()));
        }
        return is_null_MSB(get_data(), row_offset, col_idx);
    }

    // Deprecated because of the use of row_offset rather than row_idx.
    bool is_null(size_t row_offset, size_t col_idx) const {
        auto pos = row_offset + (col_idx - 1) / 8;
        if (pos >= get_data().size()) {
            throw std::runtime_error("invalid pos " + std::to_string(pos) +
                                     "; data size is " +
                                     std::to_string(get_data().size()));
        }
        return get_data().at(pos) & (128 >> ((col_idx - 1) % 8));
    }


    // extractors

    std::string extract_value_as_string(
            const std::string &col_name, size_t row_idx,
            const Command_Line_Options &options = default_options) const;

    std::string extract_value_as_string(
            size_t col_id, size_t row_idx,
            const Command_Line_Options &options = default_options) const;

    std::vector<std::string> extract_column_values_as_strings(
            const std::string &colname,
            const Command_Line_Options &options = default_options) const;

    template <typename T>
    std::vector<T> extract_value(const std::string &col_name, size_t row_idx) const {
        auto col_idx = get_column_index(col_name);
        return extract_value<T>(col_idx, row_idx);
    }

    template <typename T>
    std::vector<T> extract_value(size_t col_idx, size_t row_idx) const {
        std::vector<T> val_array;
        extract_value(val_array, col_idx, row_idx);
        return val_array;
    }

    template <typename T>
    void extract_value(std::vector<T> &val_array, size_t col_idx,
                       size_t row_idx) const {
        static_assert(!std::is_same<T, char>::value,
                      "extract_value() is not supported for columns of type char; "
                      "please use extract_values_as_string().");

        validate_column_index(col_idx);
        validate_row_index(row_idx);

        const auto &columns = get_columns();
        auto &column = columns[col_idx];
        auto array_size = column.get_array_size();
        size_t row_offset = row_idx * get_row_size();
        if (is_null_value(row_idx, col_idx)) {
            for (size_t i = 0; i < array_size; ++i) {
                val_array.emplace_back(get_null<T>());
            }
        } else {
            // JTODO what if an element is null?  Assume already has get_null() value?
            size_t base_offset = row_offset + get_offsets().at(col_idx);
            uint8_t const *curr_data = get_data().data() + base_offset;
            size_t element_size = data_size(column.get_type());

            for (size_t i = 0; i < array_size; ++i) {
                val_array.emplace_back(*(reinterpret_cast<const T *>(curr_data)));
                curr_data += element_size;
            }
        }
    }

    const uint8_t *extract_value_ptr(size_t col_idx, size_t row_idx) const;

    template <typename T>
    std::vector<T> extract_column(const std::string &col_name) const {
        auto col_idx = get_column_index(col_name);
        return extract_column<T>(col_idx);
    }

    template <typename T>
    std::vector<T> extract_column(size_t col_idx) const {
        static_assert(!std::is_same<T, char>::value,
                      "extract_column() is not supported for columns of type char; "
                      "please use extract_column_values_as_strings().");
        validate_column_index(col_idx);

        const auto &columns = get_columns();
        auto &column = columns[col_idx];
        size_t row_count = num_rows();
        std::vector<T> col_vec;
        col_vec.reserve(row_count * column.get_array_size());
        for (size_t curr_row_idx = 0; curr_row_idx < row_count; ++curr_row_idx) {
            extract_value<T>(col_vec, col_idx, curr_row_idx);
        }
        return col_vec;
    }


    // inserters
    void insert_null_into_row(tablator::Row &row, size_t col_idx,
                              uint32_t array_size) const;

    void insert_null_into_row(tablator::Row &row, size_t col_idx) const {
        insert_null_into_row(row, col_idx, get_columns().at(col_idx).get_array_size());
    }

    void insert_array_element_into_row(tablator::Row &row, size_t col_idx,
                                       size_t elt_idx, const uint8_t *data_ptr) const;

    void insert_ptr_value_into_row(Row &row, size_t col_idx, const uint8_t *data_ptr,
                                   uint32_t array_size) const;

    void insert_ptr_value_into_row(Row &row, size_t col_idx,
                                   const uint8_t *data_ptr) const {
        insert_ptr_value_into_row(row, col_idx, data_ptr,
                                  get_columns().at(col_idx).get_array_size());
    }


public:
    void insert_string_column_value_into_row(Row &row, size_t col_idx,
                                             const uint8_t *data_ptr,
                                             uint32_t curr_array_size) const;

    void insert_string_column_value_into_row(Row &row, size_t col_idx,
                                             const uint8_t *data_ptr) {
        insert_string_column_value_into_row(row, col_idx, data_ptr,
                                            get_columns().at(col_idx).get_array_size());
    }

    template <typename T>
    void insert_column_value_into_row(Row &row, size_t col_idx, const T &element,
                                      uint32_t array_size) const {
        insert_ptr_value_into_row(
                row, col_idx, reinterpret_cast<const uint8_t *>(&element), array_size);
    }

    template <typename T>
    void insert_column_value_into_row(Row &row, size_t col_idx,
                                      const T &element) const {
        insert_column_value_into_row<T>(row, col_idx, element,
                                        get_columns().at(col_idx).get_array_size());
    }

    void retain_only_selected_rows(const std::set<size_t> &selected_row_idx_list) {
        tablator::retain_only_selected_rows(get_data(), selected_row_idx_list,
                                            get_num_rows(), get_row_size());
    }

    // accessors

    size_t get_row_size() const { return tablator::get_row_size(get_offsets()); }
    size_t get_num_rows() const {
        return tablator::get_num_rows(get_offsets(), get_data());
    }
    size_t get_num_columns() const { return get_columns().size(); }

    // called by query_server to trim result set
    void resize_data(const size_t &new_num_rows) {
        tablator::resize_data(get_data(), new_num_rows, get_row_size());
    }

    // deprecated
    inline void resize_rows(const size_t &new_num_rows) { resize_data(new_num_rows); }
    size_t row_size() const { return get_row_size(); }
    size_t num_rows() const { return get_num_rows(); }

    //===========================================================

    // getters for Optional elements

    ATTRIBUTES &get_attributes() { return options_.attributes_; }
    const ATTRIBUTES &get_attributes() const { return options_.attributes_; }

    std::string &get_description() { return options_.description_; }
    const std::string &get_description() const { return options_.description_; }

    std::vector<std::string> &get_comments() { return options_.comments_; }
    const std::vector<std::string> &get_comments() const { return options_.comments_; }

    std::vector<Field> &get_params() { return options_.params_; }
    const std::vector<Field> &get_params() const { return options_.params_; }


    Labeled_Properties &get_labeled_properties() {
        return options_.labeled_properties_;
    }
    const Labeled_Properties &get_labeled_properties() const {
        return options_.labeled_properties_;
    }

    std::vector<Group_Element> &get_group_elements() {
        return options_.group_elements_;
    }
    const std::vector<Group_Element> &get_group_elements() const {
        return options_.group_elements_;
    }

    std::vector<Property> &get_trailing_info_list() {
        return options_.trailing_info_list_;
    }

    const std::vector<Property> &get_trailing_info_list() const {
        return options_.trailing_info_list_;
    }

    //===========================================================

    // getters for non-Optional elements

    std::vector<Resource_Element> &get_resource_elements() {
        return resource_elements_;
    }

    const std::vector<Resource_Element> &get_resource_elements() const {
        return resource_elements_;
    }

    Resource_Element &get_results_resource_element() {
        assert(get_resource_elements().size() > get_results_resource_idx());
        return get_resource_elements().at(get_results_resource_idx());
    }


    const Resource_Element &get_results_resource_element() const {
        assert(get_resource_elements().size() > get_results_resource_idx());
        return get_resource_elements().at(get_results_resource_idx());
    }

    std::vector<Column> &get_columns() {
        return get_results_resource_element().get_columns();
    }
    const std::vector<Column> &get_columns() const {
        return get_results_resource_element().get_columns();
    }

    std::vector<size_t> &get_offsets() {
        return get_results_resource_element().get_offsets();
    }

    const std::vector<size_t> &get_offsets() const {
        return get_results_resource_element().get_offsets();
    }

    Labeled_Properties &get_resource_element_labeled_properties() {
        return get_results_resource_element().get_labeled_properties();
    }

    const Labeled_Properties &get_resource_element_labeled_properties() const {
        return get_results_resource_element().get_labeled_properties();
    }

    const std::vector<Field> &get_resource_element_params() const {
        return get_results_resource_element().get_params();
    }

    std::vector<Field> &get_table_element_params() {
        return get_results_resource_element().get_table_element_params();
    }
    const std::vector<Field> &get_table_element_params() const {
        return get_results_resource_element().get_table_element_params();
    }

    std::vector<Field> &get_table_element_fields() {
        return get_results_resource_element().get_table_element_fields();
    }
    const std::vector<Field> &get_table_element_fields() const {
        return get_results_resource_element().get_table_element_fields();
    }

    std::vector<uint8_t> &get_data() {
        return get_results_resource_element().get_data();
    }

    const std::vector<uint8_t> &get_data() const {
        return get_results_resource_element().get_data();
    }


    Table_Element &get_main_table_element() {
        return get_results_resource_element().get_main_table_element();
    }


    const Table_Element &get_main_table_element() const {
        return get_results_resource_element().get_main_table_element();
    }


    //===========================================================

    // setters for Optional elements

    void set_attributes(
            const std::initializer_list<std::pair<const std::string, std::string>>
                    &attributes) {
        options_.set_attributes(attributes);
    }

    void add_attributes(const ATTRIBUTES &attributes) {
        options_.add_attributes(attributes);
    }

    void add_attribute(const std::string &name, const std::string &val) {
        options_.add_attribute(std::make_pair(name, val));
    }

    void set_description(const std::string &description) {
        options_.set_description(description);
    }

    void add_comments(const std::vector<std::string> &comments) {
        options_.add_comments(comments);
    }
    void add_comment(const std::string &comment) { options_.add_comment(comment); }

    void set_params(const std::vector<Field> &params) { options_.set_params(params); }

    void add_params(const std::vector<Field> &params) { options_.add_params(params); }

    void add_param(const Field &param) { options_.add_param(param); }

    void set_labeled_properties(const Labeled_Properties &labeled_props) {
        options_.labeled_properties_ = labeled_props;
    }

    void add_labeled_properties(const Labeled_Properties &labeled_props) {
        options_.add_labeled_properties(labeled_props);
    }

    // Temporarily implemented so as to support backward compatibility; not just a
    // wrapper.
    void add_labeled_property(const Labeled_Property &label_and_prop);
    void add_labeled_property(const std::string &label, const Property &prop) {
        add_labeled_property(Labeled_Property(label, prop));
    }

    void add_group_elements(const std::vector<Group_Element> &ges) {
        options_.add_group_elements(ges);
    }
    void add_group_element(const Group_Element &ge) { options_.add_group_element(ge); }
    void add_trailing_info_list(const std::vector<Property> &trailing_info_list) {
        options_.add_trailing_info_list(trailing_info_list);
    }
    void add_trailing_info(const Property &prop) { options_.add_trailing_info(prop); }


    //=================================================
    // setters for non-Optional elements

    void set_resource_elements(const std::vector<Resource_Element> &resource_elements) {
        resource_elements_ = resource_elements;
        arrange_resources();
    }

    void add_resource_elements(const std::vector<Resource_Element> &resource_elements) {
        resource_elements_.insert(resource_elements_.end(), resource_elements.begin(),
                                  resource_elements.end());
        arrange_resources();
    }

    void add_resource_element(const Resource_Element &resource_element) {
        resource_elements_.emplace_back(resource_element);
        arrange_resources();
    }

    //=================================================
    // Setter-wrappers for sub-elements

    void set_resource_element_params(const std::vector<Field> &params) {
        get_results_resource_element().set_params(params);
    }

    void set_table_element_params(const std::vector<Field> &params) {
        get_results_resource_element().set_table_element_params(params);
    }

    void set_data(const std::vector<uint8_t> &d) {
        get_results_resource_element().set_data(d);
    }


    void add_resource_element_attribute(const std::string &name,
                                        const std::string &value) {
        get_results_resource_element().add_attribute(name, value);
    }

    void add_resource_element_attribute(const STRING_PAIR &attr_pair) {
        get_results_resource_element().add_attribute(attr_pair);
    }

    void add_resource_element_labeled_property(const Labeled_Property &label_and_prop) {
        get_results_resource_element().add_labeled_property(label_and_prop);
    }

    void add_resource_element_labeled_property(const std::string &label,
                                               const Property &prop) {
        add_resource_element_labeled_property(Labeled_Property(label, prop));
    }
    size_t get_results_resource_idx() const { return results_resource_idx_; }
    void set_results_resource_idx(size_t idx) { results_resource_idx_ = idx; }


private:
    std::vector<size_t> get_column_widths(const Command_Line_Options &options) const {
        return Ipac_Table_Writer::get_column_widths(*this, options);
    }

    void write_ipac_table_header(std::ostream &os) const {
        Ipac_Table_Writer::write_keywords_and_comments(*this, os);
    }

    void write_ipac_column_headers(std::ostream &os,
                                   const Command_Line_Options &options) const {
        Ipac_Table_Writer::write_column_headers(*this, os, options);
    }


    std::string to_ipac_string(const Data_Type &type) const {
        return Ipac_Table_Writer::to_ipac_string(type);
    }


    // helpers for writing

    void write_hdf5_to_H5File(H5::H5File &outfile) const;
    void write_hdf5_attributes(H5::DataSet &table) const;

    void write_sql_insert(
            std::ostream &os, const std::string &quoted_table_name,
            const size_t &row_idx, const bool &has_point,
            const std::pair<std::pair<size_t, Data_Type>, std::pair<size_t, Data_Type>>
                    &point_input,
            const std::vector<std::pair<std::pair<size_t, Data_Type>,
                                        std::pair<size_t, Data_Type>>> &polygon_input,
            const Command_Line_Options &options) const;

    void write_sql_insert(std::ostream &os, const std::string &quoted_table_name,
                          const size_t &row_idx,
                          const Command_Line_Options &options) const {
        write_sql_insert(
                os, quoted_table_name, row_idx, false,
                std::pair<std::pair<size_t, Data_Type>, std::pair<size_t, Data_Type>>(),
                std::vector<std::pair<std::pair<size_t, Data_Type>,
                                      std::pair<size_t, Data_Type>>>(),
                options);
    }


    void reserve_data(const size_t &new_num_rows) {
        tablator::reserve_data(get_data(), new_num_rows, get_row_size());
    }

    // helpers for reading

    // WARNING: The private append_column() routines do not increase
    // the size of the null column.  The expectation is that the
    // number of columns is known before adding columns.
    void append_column(const std::string &name, const Data_Type &type,
                       const size_t &size, const Field_Properties &field_properties,
                       bool dynamic_array_flag) {
        append_column(Column(name, type, size, field_properties, dynamic_array_flag));
    }

    void append_column(const std::string &name, const Data_Type &type,
                       const size_t &size, bool dynamic_array_flag) {
        append_column(Column(name, type, size, dynamic_array_flag));
    }

    void append_column(const std::string &name, const Data_Type &type,
                       const size_t &size, const Field_Properties &field_properties) {
        append_column(Column(name, type, size, field_properties));
    }

    void append_column(const std::string &name, const Data_Type &type,
                       const size_t &size) {
        append_column(name, type, size);
    }

    void append_column(const std::string &name, const Data_Type &type) {
        append_column(name, type);
    }

    void append_column(const Column &column) {
        tablator::append_column(get_columns(), get_offsets(), column);
    }

    size_t read_ipac_header(std::istream &ipac_file,
                            std::array<std::vector<std::string>, 4> &Columns,
                            std::vector<size_t> &ipac_table_offsets,
                            Labeled_Properties &labeled_resource_properties);

    void create_types_from_ipac_headers(
            std::array<std::vector<std::string>, 4> &Columns,
            const std::vector<size_t> &ipac_column_widths) {
        create_types_from_ipac_headers(get_columns(), get_offsets(), Columns,
                                       ipac_column_widths);
    }

    void append_ipac_data_member(const std::string &name, const std::string &data_type,
                                 const size_t &size) {
        append_ipac_data_member(get_columns(), get_offsets(), name, data_type, size);
    }

    void shrink_ipac_string_columns_to_fit(const std::vector<size_t> &column_widths) {
        shrink_ipac_string_columns_to_fit(get_columns(), get_offsets(), get_data(),
                                          column_widths);
    };


    static void append_ipac_data_member(std::vector<Column> &columns,
                                        std::vector<size_t> &offsets,
                                        const std::string &name,
                                        const std::string &data_type,
                                        const size_t &size);

    static void create_types_from_ipac_headers(
            std::vector<Column> &columns, std::vector<size_t> &offsets,
            const std::array<std::vector<std::string>, 4> &ipac_columns,
            const std::vector<size_t> &ipac_column_widths);

    static void shrink_ipac_string_columns_to_fit(
            std::vector<Column> &columns, std::vector<size_t> &offsets,
            std::vector<uint8_t> &data, const std::vector<size_t> &column_widths);


    // miscellaneous helpers for reading


    static std::vector<uint8_t> read_dsv_rows(
            std::vector<Column> &columns, std::vector<size_t> &offsets,
            const std::list<std::vector<std::string>> &dsv);


    // used only for read_dsv()?
    static void set_column_info(std::vector<Column> &columns,
                                std::vector<size_t> &offsets,
                                std::list<std::vector<std::string>> &dsv);


    // This function is not used internally.
    static std::vector<STRING_PAIR> flatten_properties(
            const Labeled_Properties &properties);

    Table(const std::vector<Resource_Element> &resource_elements,
          const Options &options)
            : resource_elements_(resource_elements), options_(options) {
        arrange_resources();
    }

    void arrange_resources() {
        if (resource_elements_.size() == 1) {
            results_resource_idx_ = 0;
            return;
        }
        stable_sort(resource_elements_.begin(), resource_elements_.end());
        const auto iter = std::find_if(
                resource_elements_.begin(), resource_elements_.end(),
                [&](const Resource_Element &elt) { return elt.is_results_resource(); });
        if (iter == resource_elements_.end()) {
            // Assume not VOTABLE and only 1 resource. (Shouldn't happen.)
            results_resource_idx_ = 0;
        } else {
            results_resource_idx_ = distance(resource_elements_.begin(), iter);
        }
    }

    std::vector<Data_Type> get_original_datatypes() const {
        std::vector<Data_Type> orig_datatypes;
        const auto &columns = get_columns();
        for (size_t col = 0; col <= columns.size(); ++col) {
            orig_datatypes.emplace_back(columns[col].get_type());
        }
        return orig_datatypes;
    }

    void write_fits(std::ostream &os,
                    const std::vector<Data_Type> &datatypes_for_writing) const;
    void write_fits(const boost::filesystem::path &filename,
                    const std::vector<Data_Type> &datatypes_for_writing) const;
    void write_fits(fitsfile *fits_file,
                    const std::vector<Data_Type> &datatypes_for_writing) const;

    void write_html(std::ostream &os,
                    const std::vector<Data_Type> &datatypes_for_writing,
                    const Command_Line_Options &options) const;


    void splice_tabledata_and_write(std::ostream &os, std::stringstream &ss,
                                    Format::Enums enum_format, uint num_spaces_left,
                                    uint num_spaces_right,
                                    const Command_Line_Options &options) const;


    boost::property_tree::ptree generate_property_tree(
            const std::vector<Data_Type> &datatypes_for_writing, bool json_prep) const;

    void distribute_metadata(
            tablator::Labeled_Properties &resource_element_labeled_properties,
            std::vector<tablator::Property> &resource_element_trailing_infos,
            tablator::ATTRIBUTES &resource_element_attributes,
            std::vector<tablator::Property> &table_element_trailing_infos,
            tablator::ATTRIBUTES &table_element_attributes,
            const tablator::Labeled_Properties &label_prop_pairs);


    const Labeled_Properties combine_trailing_info_lists_all_levels() const;

    const Labeled_Properties combine_labeled_properties_all_levels() const;

    // Called by write_hdf5_attributes() and write_fits().
    const Labeled_Properties combine_attributes_all_levels(
            bool include_column_attributes_f) const;


    // JTODO terminology for table that has been constructed but not loaded.

    // In the middle of a read_XXX(), Table-level class members already exist but
    // Resource_Element- and Table_Element-level ones do not.  Components destined for
    // the lower-level classes must be stored in temporary vectors which will then be
    // sent as arguments to the relevant constructors.


    // labeled by element: Table, Resource_Element, Table_Element.
    bool stash_trailing_info_labeled_by_element(
            std::vector<Property> &resource_element_infos,
            std::vector<Property> &table_element_infos,
            const Labeled_Property &label_and_prop);

    bool stash_attributes_labeled_by_element(ATTRIBUTES &resource_element_attributes,
                                             ATTRIBUTES &table_element_attributes,
                                             const Labeled_Property &label_and_prop);


    void stash_resource_element_labeled_property(
            Labeled_Properties &resource_labeled_properties,
            const Labeled_Property &label_and_prop);


    void stash_resource_element_labeled_property(
            Labeled_Properties &resource_labeled_properties, const std::string &label,
            const Property &prop) {
        stash_resource_element_labeled_property(resource_labeled_properties,
                                                Labeled_Property(label, prop));
    }

    // private class members

    std::vector<Resource_Element> resource_elements_;
    Options options_;
    size_t results_resource_idx_;
};
}  // namespace tablator
