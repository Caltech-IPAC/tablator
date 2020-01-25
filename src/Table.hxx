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
#include "Field_Properties.hxx"
#include "Format.hxx"
#include "Ipac_Table_Writer.hxx"
#include "Property.hxx"
#include "Row.hxx"

namespace tablator {
class VOTable_Field;

class Table {
public:
    static constexpr char const *FIXLEN_KEYWORD = "fixlen";
    static constexpr char const *ROWS_RETRIEVED_KEYWORD = "RowsRetrieved";
    static constexpr const char *DEFAULT_NULL_VALUE = "null";

    static const std::string null_bitfield_flags_name;
    static const std::string null_bitfield_flags_description;

    Table(const std::vector<Column> &Columns,
          const std::map<std::string, std::string> &property_map);

    Table(const std::vector<Column> &Columns)
            : Table(Columns, std::map<std::string, std::string>()) {}

    Table(const boost::filesystem::path &input_path, const Format &format);
    Table(const boost::filesystem::path &input_path) { read_unknown(input_path); }
    Table(std::istream &input_stream) { read_unknown(input_stream); }
    Table(std::istream &input_stream, const Format &format);

    inline size_t row_size() const { return row_size(get_offsets()); }
    size_t num_rows() const { return get_data().size() / row_size(); }

    // FIXME: Unfortunately, this is an opposite convention from
    // VOTable.  VOTable use the most significant bit for the
    // first column.  This uses the least significant bit.
    bool is_null(size_t row_offset, size_t column) const {
        return get_data().at(row_offset + (column - 1) / 8) &
               (128 >> ((column - 1) % 8));
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
        append_column(get_columns(), get_offsets(), column);
    }

    void append_row(const Row &row) {
        assert(row.data.size() == row_size());
        append_row(get_data(), row);
    }

    void unsafe_append_row(const char *row) {
        unsafe_append_row(get_data(), row, row_size());
    }

    void pop_row() { pop_row(get_data(), row_size()); }

    void resize_rows(const size_t &new_num_rows) {
        resize_rows(get_data(), new_num_rows, row_size());
    }

    std::vector<std::pair<std::string, std::string>> flatten_properties() const {
        return flatten_properties(get_labeled_properties());
    };

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

    void write_tabledata(std::ostream &os, const Format::Enums &output_format) const;

    void write_html(std::ostream &os) const;


    boost::property_tree::ptree generate_property_tree(
            const std::string &tabledata_string) const;

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
    void read_json(std::istream &input_stream) {
        boost::property_tree::ptree tree;
        boost::property_tree::read_json(input_stream, tree);
        read_property_tree_as_votable(tree);
    }
    void read_json(const boost::filesystem::path &path) {
        boost::filesystem::ifstream input_stream(path);
        read_json(input_stream);
    }
    void read_votable(std::istream &input_stream) {
        boost::property_tree::ptree tree;
        boost::property_tree::read_xml(input_stream, tree);
        read_property_tree_as_votable(tree);
    }
    void read_votable(const boost::filesystem::path &path) {
        boost::filesystem::ifstream input_stream(path);
        read_votable(input_stream);
    }

    ATTRIBUTES extract_attributes(const boost::property_tree::ptree &node);
    void read_property_tree_as_votable(const boost::property_tree::ptree &tree);
    void read_resource(const boost::property_tree::ptree &resource);
    void read_table(const boost::property_tree::ptree &table);
    VOTable_Field read_field(const boost::property_tree::ptree &field);
    Property read_property(const boost::property_tree::ptree &prop);
    void read_data(const boost::property_tree::ptree &data,
                   const std::vector<VOTable_Field> &fields);
    void read_tabledata(const boost::property_tree::ptree &tabledata,
                        const std::vector<VOTable_Field> &fields);
    void read_binary2(const boost::property_tree::ptree &tabledata,
                      const std::vector<VOTable_Field> &fields);

    void append_data_from_stream(const std::vector<uint8_t> &stream,
                                 const std::vector<VOTable_Field> &fields,
                                 size_t num_rows) {
        append_data_from_stream(get_data(), get_columns(), get_offsets(), stream,
                                fields, num_rows);
    };

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

    size_t read_ipac_header(std::istream &ipac_file,
                            std::array<std::vector<std::string>, 4> &Columns,
                            std::vector<size_t> &ipac_table_offsets);

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

    // static functions
    // JTODO Move them out of this class?
    inline static size_t row_size(const std::vector<size_t> &offsets) {
        if (offsets.empty()) {
            throw std::runtime_error("<offsets> is empty");
        }
        return offsets.back();
    }

    static std::vector<uint8_t> read_dsv_rows(
            std::vector<Column> &columns, std::vector<size_t> &offsets,
            const std::list<std::vector<std::string>> &dsv);

    static void set_column_info(std::vector<Column> &columns,
                                std::vector<size_t> &offsets,
                                std::list<std::vector<std::string>> &dsv);


    // WARNING: append_column routines do not increase the size of the
    // null column.  The expectation is that the number of columns is
    // known before adding columns.

    static void append_column(std::vector<Column> &columns,
                              std::vector<size_t> &offsets, const Column &column);

    static void append_column(std::vector<Column> &columns,
                              std::vector<size_t> &offsets, const std::string &name,
                              const Data_Type &type, const size_t &size,
                              const Field_Properties &field_properties) {
        append_column(columns, offsets, Column(name, type, size, field_properties));
    }

    static void append_column(std::vector<Column> &columns,
                              std::vector<size_t> &offsets, const std::string &name,
                              const Data_Type &type, const size_t &size) {
        append_column(columns, offsets, Column(name, type, size, Field_Properties()));
    }

    static void append_column(std::vector<Column> &columns,
                              std::vector<size_t> &offsets, const std::string &name,
                              const Data_Type &type) {
        append_column(columns, offsets, name, type, 1, Field_Properties());
    }


    static void append_row(std::vector<uint8_t> &data, const Row &row) {
        data.insert(data.end(), row.data.begin(), row.data.end());
    }

    static void unsafe_append_row(std::vector<uint8_t> &data, const char *row,
                                  uint row_size) {
        data.insert(data.end(), row, row + row_size);
    }

    static void pop_row(std::vector<uint8_t> &data, uint row_size) {
        data.resize(data.size() - row_size);
    }

    static void resize_rows(std::vector<uint8_t> &data, const size_t &new_num_rows,
                            uint row_size) {
        data.resize(new_num_rows * row_size);
    }

    static void append_data_from_stream(std::vector<uint8_t> &data,
                                        const std::vector<Column> &columns,
                                        const std::vector<size_t> &offsets,
                                        const std::vector<uint8_t> &stream,
                                        const std::vector<VOTable_Field> &fields,
                                        size_t num_rows);


    static void append_ipac_data_member(std::vector<Column> &columns,
                                        std::vector<size_t> &offsets,
                                        const std::string &name,
                                        const std::string &data_type,
                                        const size_t &size);

    static void create_types_from_ipac_headers(
            std::vector<Column> &columns, std::vector<size_t> &offsets,
            const std::array<std::vector<std::string>, 4> &Columns,
            const std::vector<size_t> &ipac_column_widths);

    static void shrink_ipac_string_columns_to_fit(
            std::vector<Column> &columns, std::vector<size_t> &offsets,
            std::vector<uint8_t> &data, const std::vector<size_t> &column_widths);


    // This function is not used internally.
    static std::vector<std::pair<std::string, std::string>> flatten_properties(
            const std::vector<std::pair<std::string, Property>> &properties);


    // getters

    inline std::vector<std::pair<std::string, Property>> &get_labeled_properties() {
        return labeled_properties_;
    }
    inline const std::vector<std::pair<std::string, Property>> &get_labeled_properties()
            const {
        return labeled_properties_;
    }

    inline std::vector<uint8_t> &get_data() { return data_; }
    inline const std::vector<uint8_t> &get_data() const { return data_; }

    inline std::vector<std::string> &get_comments() { return comments_; }
    inline const std::vector<std::string> &get_comments() const { return comments_; }

    inline std::vector<Column> &get_resource_element_params() {
        return resource_element_params_;
    }
    inline const std::vector<Column> &get_resource_element_params() const {
        return resource_element_params_;
    }

    inline std::vector<Column> &get_table_element_params() {
        return table_element_params_;
    }
    inline const std::vector<Column> &get_table_element_params() const {
        return table_element_params_;
    }

    inline std::vector<Column> &get_columns() { return columns_; }
    inline const std::vector<Column> &get_columns() const { return columns_; }

    inline std::vector<size_t> &get_offsets() { return offsets_; }
    inline const std::vector<size_t> &get_offsets() const { return offsets_; }

    // setters

    inline void set_labeled_properties(
            const std::vector<std::pair<std::string, Property>> &props) {
        labeled_properties_ = props;
    }
    inline void set_data(const std::vector<uint8_t> &d) { data_ = d; }
    inline void set_comments(const std::vector<std::string> &comms) {
        comments_ = comms;
    }
    inline void set_resource_element_params(const std::vector<Column> &params) {
        resource_element_params_ = params;
    }
    inline void set_table_element_params(const std::vector<Column> &params) {
        table_element_params_ = params;
    }


    inline void set_columns(const std::vector<Column> &cols) { columns_ = cols; }
    inline void set_offsets(const std::vector<size_t> &offs) { offsets_ = offs; }


    inline void add_labeled_property(
            const std::pair<std::string, Property> &label_and_prop) {
        labeled_properties_.emplace_back(label_and_prop);
    }
    inline void add_labeled_property(const std::string &label, const Property &prop) {
        add_labeled_property(std::make_pair(label, prop));
    }


    void add_comment(const std::string &c) { comments_.emplace_back(c); }

    // This function does something more interesting in refactored tablator.
    // Adding it now eases the transition.
    inline void add_resource_element_labeled_property(
            const std::pair<std::string, Property> &label_and_prop) {
        add_labeled_property(label_and_prop);
    }

    inline void add_resource_element_param(const Column &param) {
        resource_element_params_.emplace_back(param);
    }
    inline void add_table_element_param(const Column &param) {
        table_element_params_.emplace_back(param);
    }

private:
    std::vector<std::pair<std::string, Property>> labeled_properties_;
    std::vector<uint8_t> data_;
    std::vector<std::string> comments_;

    std::vector<Column> resource_element_params_, table_element_params_;
    std::vector<Column> columns_;
    std::vector<size_t> offsets_ = {0};


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
            const std::string &tabledata_string,
            const std::vector<Data_Type> &datatypes_for_writing) const;
};
}  // namespace tablator
