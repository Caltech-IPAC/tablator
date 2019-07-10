#pragma once

#include <H5Cpp.h>
#include <CCfits/CCfits>
#include <array>
#include <fstream>
#include <iostream>
#include <set>
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
#include "Property.hxx"
#include "Row.hxx"

namespace tablator {
class VOTable_Field;

class Table {
public:
    static constexpr char const *FIXLEN_KEYWORD = "fixlen";
    static constexpr char const *ROWS_RETRIEVED_KEYWORD = "RowsRetrieved";

    std::vector<std::pair<std::string, Property>> properties;
    std::vector<uint8_t> data;
    std::vector<std::string> comments;

    std::vector<Column> resource_params, table_params;
    std::vector<Column> columns;
    std::vector<size_t> offsets = {0};

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

    size_t row_size() const { return *offsets.rbegin(); }
    size_t num_rows() const { return data.size() / row_size(); }

    // FIXME: Unfortunately, this is an opposite convention from
    // VOTable.  VOTable use the most significant bit for the
    // first column.  This uses the least significant bit.
    bool is_null(size_t row_offset, size_t column) const {
        return data[row_offset + (column - 1) / 8] & (128 >> ((column - 1) % 8));
    }

    size_t column_offset(const std::string &name) const {
        auto column = find_column(name);
        if (column == columns.end()) {
            throw std::runtime_error("Unable to find column '" + name + "' in table.");
        }
        return offsets[std::distance(columns.begin(), column)];
    }

    std::vector<Column>::const_iterator find_column(const std::string &name) const {
        return std::find_if(columns.begin(), columns.end(),
                            [&](const Column &c) { return c.name == name; });
    }

    /// WARNING: append_column routines do not increase the size of the
    /// null column.  The expectation is that the number of columns is
    /// known before adding columns.
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

    void append_column(const Column &column);

    void append_row(const Row &row) {
        assert(row.data.size() == row_size());
        data.insert(data.end(), row.data.begin(), row.data.end());
    }

    void unsafe_append_row(const char *row) {
        data.insert(data.end(), row, row + row_size());
    }

    void pop_row() { data.resize(data.size() - row_size()); }

    void resize_rows(const size_t &new_num_rows) {
        data.resize(row_size() * new_num_rows);
    }

    std::vector<std::pair<std::string, std::string>> flatten_properties() const;

    void write(std::ostream &os, const std::string &table_name,
               const Format &format) const;
    void write(const boost::filesystem::path &path, const Format &format) const;
    void write(const boost::filesystem::path &path) const { write(path, Format(path)); }
    void write_hdf5(std::ostream &os) const;
    void write_hdf5(const boost::filesystem::path &p) const;
    void write_hdf5_to_H5File(H5::H5File &outfile) const;
    void write_hdf5_attributes(H5::DataSet &table) const;

    void write_ipac_table(std::ostream &os) const;
    void write_ipac_table(const boost::filesystem::path &p) const {
        boost::filesystem::ofstream os(p);
        write_ipac_table(os);
    }

    std::vector<size_t> get_column_widths() const;
    // G2P calls this function, so can't simply rename it.  :-(
    [[deprecated]] std::vector<size_t> get_column_width() const {
        return get_column_widths();
    }

    void write_ipac_table_header(std::ostream &os) const;
    std::string to_ipac_string(const Data_Type &type) const;

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
    void read_property_tree_as_votable(const boost::property_tree::ptree &tree);
    void read_node_and_attributes(const std::string &node_name,
                                  const boost::property_tree::ptree &node);
    void read_node_and_attributes(
            const boost::property_tree::ptree::const_iterator &it) {
        read_node_and_attributes(it->first, it->second);
    }
    void read_resource(const boost::property_tree::ptree &resource);
    void read_table(const boost::property_tree::ptree &table);
    VOTable_Field read_field(const boost::property_tree::ptree &field);
    void read_data(const boost::property_tree::ptree &data,
                   const std::vector<VOTable_Field> &fields);
    void read_tabledata(const boost::property_tree::ptree &tabledata,
                        const std::vector<VOTable_Field> &fields);
    void read_binary2(const boost::property_tree::ptree &tabledata,
                      const std::vector<VOTable_Field> &fields);
    void append_data_from_stream(const std::vector<uint8_t> &stream,
                                 const size_t &num_rows,
                                 const std::vector<VOTable_Field> &fields);
    void read_dsv(std::istream &input_stream, const Format &format);
    void read_dsv(const boost::filesystem::path &path, const Format &format) {
        if (path == "-") {
            read_dsv(std::cin, format);
        } else {
            boost::filesystem::ifstream input_stream(path);
            read_dsv(input_stream, format);
        }
    }
    void read_dsv_rows(const std::list<std::vector<std::string>> &dsv);
    void set_column_info(std::list<std::vector<std::string>> &dsv);

    size_t read_ipac_header(std::istream &ipac_file,
                            std::array<std::vector<std::string>, 4> &Columns,
                            std::vector<size_t> &ipac_table_offsets);

    void create_types_from_ipac_headers(
            std::array<std::vector<std::string>, 4> &Columns,
            const std::vector<size_t> &ipac_column_offsets,
            std::vector<size_t> &ipac_column_widths);

    void append_ipac_data_member(const std::string &name, const std::string &data_type,
                                 const size_t &size);

    void shrink_ipac_string_columns_to_fit(const std::vector<size_t> &column_widths);

private:
    std::vector<Data_Type> get_original_datatypes() const {
        std::vector<Data_Type> orig_datatypes;
        for (size_t col = 0; col <= columns.size(); ++col) {
            orig_datatypes.emplace_back(columns[col].type);
        }
        return orig_datatypes;
    }

    void write_ipac_table(std::ostream &os,
                          const std::vector<Data_Type> &datatypes) const;

    void write_fits(std::ostream &os,
                    const std::vector<Data_Type> &datatypes_for_writing) const;
    void write_fits(const boost::filesystem::path &filename,
                    const std::vector<Data_Type> &datatypes_for_writing) const;
    void write_fits(fitsfile *fits_file,
                    const std::vector<Data_Type> &datatypes_for_writing) const;

    void write_tabledata(std::ostream &os, const Format::Enums &output_format,
                         const std::vector<Data_Type> &datatypes) const;
    void write_html(std::ostream &os,
                    const std::vector<Data_Type> &datatypes_for_writing) const;

    boost::property_tree::ptree generate_property_tree(
            const std::string &tabledata_string,
            const std::vector<Data_Type> &datatypes_for_writing) const;
};
}  // namespace tablator
