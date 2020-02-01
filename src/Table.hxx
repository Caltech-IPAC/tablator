#pragma once

#include <H5Cpp.h>
#include <CCfits/CCfits>
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

#include "Column.hxx"
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
#include "Utils/Table_Utils.hxx"

namespace tablator {

class Table {
public:
    static constexpr char const *FIXLEN_KEYWORD = "fixlen";
    static constexpr char const *ROWS_RETRIEVED_KEYWORD = "RowsRetrieved";
    static constexpr const char *DEFAULT_NULL_VALUE = "null";


private:
    struct Options {
        ATTRIBUTES attributes_;
        std::string description_;
        std::vector<std::string> comments_;
        std::vector<Field> params_;
        boost::property_tree::ptree params_ptree_;
        std::vector<std::pair<std::string, Property>> labeled_properties_;
        std::vector<Group_Element> group_elements_;
        std::vector<Property> trailing_info_list_;
    };

public:
    class Builder {
    public:
        // JTODO check that resource_elements contains at least one Table_Element?
        Builder(std::vector<Resource_Element> &resource_elements)
                : resource_elements_(resource_elements) {}

        Table build() { return Table(resource_elements_, options_); }

        Builder &add_attributes(const ATTRIBUTES &attributes) {
            options_.attributes_ = attributes;
            return *this;
        }

        Builder &add_description(const std::string &description) {
            options_.description_ = description;
            return *this;
        }

        Builder &add_comments(const std::vector<std::string> &comments) {
            options_.comments_ = comments;
            return *this;
        }

        Builder &add_params(const std::vector<Field> &params) {
            options_.params_ = params;
            return *this;
        }

        Builder &add_params_tree(const boost::property_tree::ptree &params_ptree) {
            options_.params_ptree_ = params_ptree;
            return *this;
        }

        Builder &add_labeled_properties(
                const std::vector<std::pair<std::string, Property>>
                        &labeled_properties) {
            options_.labeled_properties_ = labeled_properties;
            return *this;
        }

        Builder &add_group_elements(const std::vector<Group_Element> &group_elements) {
            options_.group_elements_ = group_elements;
            return *this;
        }

        Builder &add_trailing_info_list(
                const std::vector<Property> &trailing_info_list) {
            options_.trailing_info_list_ = trailing_info_list;
            return *this;
        }


    private:
        std::vector<Resource_Element> resource_elements_;
        Options options_;
    };


    Table(const std::vector<Column> &Columns,
          const std::map<std::string, std::string> &property_map);

    Table(const std::vector<Column> &Columns,
          const std::vector<std::pair<std::string, tablator::Property>>
                  &property_pair_vec);

    Table(const std::vector<Column> &Columns)
            : Table(Columns, std::map<std::string, std::string>()) {}

    Table(const boost::filesystem::path &input_path, const Format &format);
    Table(const boost::filesystem::path &input_path) { read_unknown(input_path); }
    Table(std::istream &input_stream) { read_unknown(input_stream); }
    Table(std::istream &input_stream, const Format &format);

    // FIXME: Unfortunately, this is an opposite convention from
    // VOTable.  VOTable use the most significant bit for the
    // first column.  This uses the least significant bit.
    bool is_null(size_t row_offset, size_t column) const {
        auto pos = row_offset + (column - 1) / 8;
        if (pos >= get_data().size()) {
            throw std::runtime_error("invalid pos " + std::to_string(pos) +
                                     "; data size is " +
                                     std::to_string(get_data().size()));
        }
        return get_data().at(pos) & (128 >> ((column - 1) % 8));
    }

    std::vector<Column>::const_iterator find_column(const std::string &name) const {
        const auto &columns = get_columns();
        return std::find_if(columns.begin(), columns.end(),
                            [&](const Column &c) { return c.get_name() == name; });
    }

    size_t column_index(const std::string &name) const {
        const auto column_iter = find_column(name);
        const auto &columns = get_columns();
        if (column_iter == columns.end()) {
            throw std::runtime_error("Unable to find column '" + name + "' in table.");
        }
        return std::distance(columns.begin(), column_iter);
    }

    size_t column_offset(size_t col_id) const {
        const auto &columns = get_columns();
        if (col_id >= columns.size()) {
            throw std::runtime_error("Invalid column ID " + std::to_string(col_id) +
                                     " in table.");
        }
        return get_offsets().at(col_id);
    }

    size_t column_offset(const std::string &name) const {
        auto col_id = column_index(name);
        return get_offsets().at(col_id);
    }


    std::vector<size_t> find_column_ids(
            const std::vector<std::string> &col_names) const {
        std::vector<size_t> col_ids;

        for (const std::string &col_name : col_names) {
            size_t col_id = column_index(col_name);
            col_ids.emplace_back(col_id);
        }
        return col_ids;
    }

    std::vector<size_t> find_omitted_column_ids(
            const std::vector<std::string> &col_names) const {
        const auto &columns = get_columns();
        std::vector<size_t> col_ids = find_column_ids(col_names);
        sort(col_ids.begin(), col_ids.end());

        std::vector<size_t> all_col_ids(columns.size() - 1);
        std::iota(all_col_ids.begin(), all_col_ids.end(), 1);

        std::vector<size_t> diff_vec;
        diff_vec.reserve(columns.size());
        std::set_difference(all_col_ids.begin(), all_col_ids.end(), col_ids.begin(),
                            col_ids.end(), std::back_inserter(diff_vec));
        return diff_vec;
    }

    // WARNING: append_column routines do not increase the size of the
    // null column.  The expectation is that the number of columns is
    // known before adding columns.
    void append_column(const std::string &name, const Data_Type &type) {
        append_column(name, type, 1);
    }
    void append_column(const std::string &name, const Data_Type &type,
                       const size_t &size) {
        append_column(name, type, size, Field_Properties());
    }

    void append_column(const std::string &name, const Data_Type &type,
                       const size_t &size, const Field_Properties &field_properties) {
        append_column(Column(name, type, size, field_properties));
    }

    void append_column(const Column &column) {
        tablator::append_column(get_columns(), get_offsets(), column);
    }

    void append_row(const Row &row) {
        assert(row.data.size() == row_size());
        tablator::append_row(get_data(), row);
    }

    void unsafe_append_row(const char *row) {
        tablator::unsafe_append_row(get_data(), row, row_size());
    }

    void pop_row() { tablator::pop_row(get_data(), row_size()); }

    void resize_rows(const size_t &new_num_rows) {
        tablator::resize_rows(get_data(), new_num_rows, row_size());
    }

    // This function is not used internally.

    std::vector<std::pair<std::string, std::string>> flatten_properties() const {
        return flatten_properties(get_labeled_properties());
    }

    void write(std::ostream &os, const std::string &table_name,
               const Format &format) const;
    void write(const boost::filesystem::path &path, const Format &format) const;
    void write(const boost::filesystem::path &path) const { write(path, Format(path)); }
    void write_hdf5(std::ostream &os) const;
    void write_hdf5(const boost::filesystem::path &p) const;
    void write_hdf5_to_H5File(H5::H5File &outfile) const;
    void write_hdf5_attributes(H5::DataSet &table) const;

    void write_ipac_table(std::ostream &os) const {
        Ipac_Table_Writer::write(*this, os);
    }
    void write_ipac_table(const boost::filesystem::path &p) const {
        boost::filesystem::ofstream os(p);
        write_ipac_table(os);
    }
    void write_ipac_subtable_by_row(std::ostream &os,
                                    std::vector<size_t> requested_row_ids) const {
        Ipac_Table_Writer::write_subtable_by_row(*this, os, requested_row_ids);
    }

    void write_ipac_subtable_by_column_and_row(
            std::ostream &os, const std::vector<size_t> &column_ids,
            std::vector<size_t> requested_row_ids) const {
        Ipac_Table_Writer::write_subtable_by_column_and_row(*this, os, column_ids,
                                                            requested_row_ids);
    }

    void write_ipac_subtable_by_row(std::ostream &os, size_t start_row,
                                    size_t row_count) const {
        Ipac_Table_Writer::write_subtable_by_row(*this, os, start_row, row_count);
    }

    void write_ipac_subtable_by_column_and_row(std::ostream &os,
                                               const std::vector<size_t> &column_ids,
                                               size_t start_row,
                                               size_t row_count) const {
        Ipac_Table_Writer::write_subtable_by_column_and_row(*this, os, column_ids,
                                                            start_row, row_count);
    }

    void write_ipac_subtable_by_column_and_row(
            std::ostream &os, const std::vector<size_t> &column_ids) const {
        Ipac_Table_Writer::write_subtable_by_column_and_row(*this, os, column_ids, 0,
                                                            num_rows());
    }

    void write_single_ipac_record(std::ostream &os, size_t row_idx) const {
        Ipac_Table_Writer::write_single_record(*this, os, row_idx);
    }

    void write_single_ipac_record(std::ostream &os,
                                  const std::vector<size_t> &included_column_ids,
                                  size_t row_idx) const {
        Ipac_Table_Writer::write_single_record(*this, os, included_column_ids, row_idx);
    }

    void write_consecutive_ipac_records(std::ostream &os, size_t start_row,
                                        size_t row_count) const {
        Ipac_Table_Writer::write_consecutive_records(*this, os, start_row, row_count);
    }

    void write_consecutive_ipac_records(std::ostream &os,
                                        const std::vector<size_t> &included_column_ids,
                                        size_t start_row, size_t row_count) const {
        Ipac_Table_Writer::write_consecutive_records(*this, os, included_column_ids,
                                                     start_row, row_count);
    }

    void write_selected_ipac_records(
            std::ostream &os, std::vector<size_t> const &requested_row_ids) const {
        Ipac_Table_Writer::write_selected_records(*this, os, requested_row_ids);
    }

    void write_selected_ipac_records(
            std::ostream &os, const std::vector<size_t> &included_column_ids,
            std::vector<size_t> const &requested_row_ids) const {
        Ipac_Table_Writer::write_selected_records(*this, os, included_column_ids,
                                                  requested_row_ids);
    }

    std::vector<size_t> get_column_widths() const {
        return Ipac_Table_Writer::get_column_widths(*this);
    }
    // G2P calls this function, so can't simply rename it.  :-(
    [[deprecated]] std::vector<size_t> get_column_width() const {
        return get_column_widths();
    }

    void write_ipac_table_header(std::ostream &os) const {
        Ipac_Table_Writer::write_header(*this, os);
    }

    void write_ipac_column_headers(std::ostream &os) const {
        Ipac_Table_Writer::write_column_headers(*this, os);
    }

    std::string to_ipac_string(const Data_Type &type) const {
        return Ipac_Table_Writer::to_ipac_string(type);
    }

    void write_dsv(std::ostream &os, const char &separator) const;
    void write_sql_create_table(std::ostream &os, const std::string &table_name,
                                const Format::Enums &sql_type) const {
        using namespace std::string_literals;
        write_sql_create_table(os, table_name, sql_type, ""s, ""s);
    }
    void write_sql_create_table(std::ostream &os, const std::string &table_name,
                                const Format::Enums &sql_type,
                                const std::string &point_column_name,
                                const std::string &polygon_column_name) const;
    void write_sql_inserts(std::ostream &os, const std::string &table_name) const {
        write_sql_inserts(os, table_name, std::pair<std::string, std::string>(),
                          std::vector<std::pair<std::string, std::string>>());
    }
    void write_sql_inserts(std::ostream &os, const std::string &table_name,
                           const std::pair<std::string, std::string> &point_input_names,
                           const std::vector<std::pair<std::string, std::string>>
                                   &polygon_input_names) const;
    void write_sql_insert(std::ostream &os, const std::string &quoted_table_name,
                          const size_t &row_offset, const bool &has_point,
                          const std::pair<std::pair<size_t, Data_Type>,
                                          std::pair<size_t, Data_Type>> &point_input,
                          const std::vector<std::pair<std::pair<size_t, Data_Type>,
                                                      std::pair<size_t, Data_Type>>>
                                  &polygon_input) const;
    void write_sql_insert(std::ostream &os, const std::string &quoted_table_name,
                          const size_t &row_offset) const {
        write_sql_insert(
                os, quoted_table_name, row_offset, false,
                std::pair<std::pair<size_t, Data_Type>, std::pair<size_t, Data_Type>>(),
                std::vector<std::pair<std::pair<size_t, Data_Type>,
                                      std::pair<size_t, Data_Type>>>());
    }
    void write_sql(std::ostream &os, const std::string &table_name,
                   const Format::Enums &sql_type) const {
        write_sql_create_table(os, table_name, sql_type);
        os << ";\n";
        write_sql_inserts(os, table_name);
    }
    void write_sqlite_db(const boost::filesystem::path &path) const;

    void write_fits(std::ostream &os) const;

    void write_fits(const boost::filesystem::path &filename) const;

    void write_fits(fitsfile *fits_file) const;

    void splice_tabledata_and_write(std::ostream &os, std::stringstream &ss,
                                    Format::Enums enum_format, uint num_spaces_left,
                                    uint num_spaces_right) const;

    void write_tabledata(std::ostream &os, const Format::Enums &output_format) const;

    void write_html(std::ostream &os) const;

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

    size_t read_ipac_header(
            std::istream &ipac_file, std::array<std::vector<std::string>, 4> &Columns,
            std::vector<size_t> &ipac_table_offsets,
            std::vector<std::pair<std::string, Property>> &labeled_resource_properties);

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

    std::string extract_value_as_string(const std::string &col_name,
                                        size_t row_id) const;
    std::string extract_value_as_string(size_t col_id, size_t row_id) const;

    std::vector<std::string> extract_column_values_as_strings(
            const std::string &colname) const;

    template <typename T>
    std::vector<T> extract_value(const std::string &col_name, size_t row_id) {
        auto col_id = column_index(col_name);
        return extract_value<T>(col_id, row_id);
    }

    template <typename T>
    std::vector<T> extract_value(size_t col_id, size_t row_id) {
        std::vector<T> val_array;
        extract_value(val_array, col_id, row_id);
        return val_array;
    }

    template <typename T>
    void extract_value(std::vector<T> &val_array, size_t col_id, size_t row_id) {
        static_assert(!std::is_same<T, char>::value,
                      "extract_value() is not supported for columns of type char; "
                      "please use extract_values_as_string().");
        const auto &columns = get_columns();
        if (col_id >= columns.size()) {
            throw std::runtime_error("Invalid column index: " + std::to_string(col_id));
        }
        if (row_id >= num_rows()) {
            throw std::runtime_error("Invalid row index: " + std::to_string(row_id));
        }

        auto &column = columns[col_id];
        auto array_size = column.get_array_size();
        size_t row_offset = row_id * row_size();
        if (is_null(row_offset, col_id)) {
            for (size_t i = 0; i < array_size; ++i) {
                val_array.emplace_back(get_null<T>());
            }
        } else {
            // JTODO what if an element is null?  Assume already has get_null() value?
            size_t base_offset = row_offset + get_offsets().at(col_id);
            uint8_t const *curr_data = get_data().data() + base_offset;
            size_t element_size = data_size(column.get_type());

            for (size_t i = 0; i < array_size; ++i) {
                val_array.emplace_back(*(reinterpret_cast<const T *>(curr_data)));
                curr_data += element_size;
            }
        }
    }

    template <typename T>
    std::vector<T> extract_column(const std::string &col_name) {
        auto col_id = column_index(col_name);
        return extract_column<T>(col_id);
    }

    template <typename T>
    std::vector<T> extract_column(size_t col_id) {
        static_assert(!std::is_same<T, char>::value,
                      "extract_column() is not supported for columns of type char; "
                      "please use extract_column_values_as_strings().");
        const auto &columns = get_columns();
        if (col_id >= columns.size()) {
            throw std::runtime_error("Invalid column index: " + std::to_string(col_id));
        }
        auto &column = columns[col_id];

        size_t row_count = num_rows();
        std::vector<T> col_vec;
        col_vec.reserve(row_count * column.get_array_size());
        for (size_t curr_row_id = 0; curr_row_id < row_count; ++curr_row_id) {
            extract_value<T>(col_vec, col_id, curr_row_id);
        }
        return col_vec;
    }

    inline size_t row_size() const { return tablator::row_size(get_offsets()); }
    size_t num_rows() const { return get_data().size() / row_size(); }

    // static functions
    static std::vector<uint8_t> read_dsv_rows(
            std::vector<Column> &columns, std::vector<size_t> &offsets,
            const std::list<std::vector<std::string>> &dsv);

    static void set_column_info(std::vector<Column> &columns,
                                std::vector<size_t> &offsets,
                                std::list<std::vector<std::string>> &dsv);


    // WARNING: append_column routines do not increase the size of the
    // null column.  The expectation is that the number of columns is
    // known before adding columns.
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


    // This function is not used internally.
    static std::vector<std::pair<std::string, std::string>> flatten_properties(
            const std::vector<std::pair<std::string, Property>> &properties);


    // getters
    inline std::vector<Resource_Element> &get_resource_elements() {
        return resource_elements_;
    }

    inline const std::vector<Resource_Element> &get_resource_elements() const {
        return resource_elements_;
    }

    inline ATTRIBUTES &get_attributes() { return options_.attributes_; }
    inline const ATTRIBUTES &get_attributes() const { return options_.attributes_; }

    inline std::string &get_description() { return options_.description_; }
    inline const std::string &get_description() const { return options_.description_; }

    inline std::vector<std::string> &get_comments() { return options_.comments_; }
    inline const std::vector<std::string> &get_comments() const {
        return options_.comments_;
    }

    inline std::vector<Field> &get_params() { return options_.params_; }
    inline const std::vector<Field> &get_params() const { return options_.params_; }

    inline std::vector<std::pair<std::string, Property>> &get_labeled_properties() {
        return options_.labeled_properties_;
    }

    inline const std::vector<std::pair<std::string, Property>> &get_labeled_properties()
            const {
        return options_.labeled_properties_;
    }

    inline std::vector<Group_Element> &get_group_elements() {
        return options_.group_elements_;
    }

    inline const std::vector<Group_Element> &get_group_elements() const {
        return options_.group_elements_;
    }

    inline std::vector<Property> &get_trailing_info_list() {
        return options_.trailing_info_list_;
    }

    inline const std::vector<Property> &get_trailing_info_list() const {
        return options_.trailing_info_list_;
    }

    inline Resource_Element &get_main_resource_element() {
        if (get_resource_elements().empty()) {
            throw std::runtime_error("table is empty");
        }
        return get_resource_elements().at(0);
    }


    inline const Resource_Element &get_main_resource_element() const {
        if (get_resource_elements().empty()) {
            throw std::runtime_error("table is empty");
        }
        return get_resource_elements().at(0);
    }

    inline std::vector<Column> &get_columns() {
        return get_main_resource_element().get_columns();
    }
    inline const std::vector<Column> &get_columns() const {
        return get_main_resource_element().get_columns();
    }

    inline std::vector<size_t> &get_offsets() {
        return get_main_resource_element().get_offsets();
    }

    inline const std::vector<size_t> &get_offsets() const {
        return get_main_resource_element().get_offsets();
    }

    inline std::vector<std::pair<std::string, Property>>
            &get_resource_element_labeled_properties() {
        return get_main_resource_element().get_labeled_properties();
    }

    inline const std::vector<std::pair<std::string, Property>>
            &get_resource_element_labeled_properties() const {
        return get_main_resource_element().get_labeled_properties();
    }

    inline const std::vector<Field> &get_resource_element_params() const {
        return get_main_resource_element().get_params();
    }

    inline std::vector<Field> &get_table_element_params() {
        return get_main_resource_element().get_table_element_params();
    }
    inline const std::vector<Field> &get_table_element_params() const {
        return get_main_resource_element().get_table_element_params();
    }

    inline std::vector<uint8_t> &get_data() {
        return get_main_resource_element().get_data();
    }

    inline const std::vector<uint8_t> &get_data() const {
        return get_main_resource_element().get_data();
    }


    inline Table_Element &get_main_table_element() {
        if (get_resource_elements().empty()) {
            throw std::runtime_error("table is empty");
        }
        return get_resource_elements().at(0).get_table_elements().at(0);
    }


    inline const Table_Element &get_main_table_element() const {
        if (get_resource_elements().empty()) {
            throw std::runtime_error("table is empty");
        }
        return get_resource_elements().at(0).get_table_elements().at(0);
    }


    // mock getters
    const std::vector<std::pair<std::string, Property>>
    combine_trailing_info_lists_all_levels() const;

    const std::vector<std::pair<std::string, Property>>
    combine_labeled_properties_all_levels() const;

    const std::vector<std::pair<std::string, Property>> combine_attributes_all_levels()
            const;


    //===========================================================

    // setters

    inline void set_attributes(const ATTRIBUTES &attrs) {
        options_.attributes_ = attrs;
    }

    void set_description(const std::string &desc) {
        options_.description_.assign(desc);
    }
    inline void set_labeled_properties(
            const std::vector<std::pair<std::string, Property>> &labeled_props) {
        options_.labeled_properties_ = labeled_props;
    }

    inline void set_params(const std::vector<Field> &params) {
        options_.params_ = params;
    }
    inline void set_resource_element_params(const std::vector<Field> &params) {
        get_main_resource_element().set_params(params);
    }

    inline void set_table_element_params(const std::vector<Field> &params) {
        get_main_resource_element().set_table_element_params(params);
    }

    inline void set_data(const std::vector<uint8_t> &d) {
        get_main_resource_element().set_data(d);
    }


    inline void add_comment(const std::string &c) {
        options_.comments_.emplace_back(c);
    }

    inline void add_param(const Field &param) { get_params().emplace_back(param); }


    inline void add_trailing_info(const Property &prop) {
        get_trailing_info_list().emplace_back(prop);
    }

    // for backward compatibility
    void add_labeled_property(const std::pair<std::string, Property> &label_and_prop);

    inline void add_labeled_property(const std::string &label, const Property &prop) {
        add_labeled_property(std::make_pair(label, prop));
    }

    // JTODO terminology for table that has been constructed but not loaded.

    // In the middle of a read_XXX(), Table-level class members already exist but
    // Resource_Element- and Table_Element-level ones do not.  Components destined for
    // the lower-level classes must be stored in temporary vectors which will then be
    // sent as arguments to the relevant constructors.

    void add_labeled_property(
            std::vector<std::pair<std::string, Property>> &resource_labeled_properties,
            const std::pair<std::string, Property> &label_and_prop);


    inline void add_labeled_property(
            std::vector<std::pair<std::string, Property>> &resource_labeled_properties,
            const std::string &label, const Property &prop) {
        add_labeled_property(resource_labeled_properties, std::make_pair(label, prop));
    }

    inline bool add_labeled_trailing_info(
            std::vector<Property> &resource_element_infos,
            std::vector<Property> &table_element_infos,
            const std::pair<std::string, Property> &label_and_prop);

    inline bool add_labeled_attributes(
            ATTRIBUTES &resource_element_attributes,
            ATTRIBUTES &table_element_attributes,
            const std::pair<std::string, Property> &label_and_prop);


    inline void add_resource_element(const Resource_Element &resource_element) {
        get_resource_elements().emplace_back(resource_element);
    }

    inline void add_resource_element(const Table_Element &table_element) {
        get_resource_elements().emplace_back(Resource_Element(table_element));
    }

    inline void add_attribute(const std::string &name, const std::string &val) {
        options_.attributes_.insert(std::make_pair(name, val));
    }

    inline void add_attributes(const ATTRIBUTES &attrs) {
        get_attributes().insert(attrs.begin(), attrs.end());
    }

    inline void add_group_element(const Group_Element &ge) {
        get_group_elements().emplace_back(ge);
    }

    inline void add_resource_element_labeled_property(
            const std::pair<std::string, Property> &label_and_prop) {
        get_main_resource_element().add_labeled_property(label_and_prop);
    }

    inline void add_resource_element_labeled_property(const std::string &label,
                                                      const Property &prop) {
        add_resource_element_labeled_property(std::make_pair(label, prop));
    }


private:
    Table(std::vector<Resource_Element> &resource_elements, const Options &options)
            : resource_elements_(resource_elements), options_(options) {}

    std::vector<Resource_Element> resource_elements_;
    Options options_;

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
                    const std::vector<Data_Type> &datatypes_for_writing) const;

    boost::property_tree::ptree generate_property_tree(
            const std::vector<Data_Type> &datatypes_for_writing) const;

    void distribute_metadata(
            std::vector<std::pair<std::string, tablator::Property>>
                    &resource_element_labeled_properties,
            std::vector<tablator::Property> &resource_element_trailing_infos,
            tablator::ATTRIBUTES &resource_element_attributes,
            std::vector<tablator::Property> &table_element_trailing_infos,
            tablator::ATTRIBUTES &table_element_attributes,
            const std::vector<std::pair<std::string, tablator::Property>>
                    &label_prop_pairs);
};
}  // namespace tablator
