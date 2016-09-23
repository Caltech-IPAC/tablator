#pragma once

#include "CSV_Document.hxx"
#include "Property.hxx"
#include "Field_Properties.hxx"
#include "Format.hxx"

#include "Row.hxx"
#include "Column.hxx"

#include <H5Cpp.h>
#include <CCfits/CCfits>
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/property_tree/json_parser.hpp>

#include <set>
#include <tuple>
#include <array>
#include <stdexcept>

#include <iostream>
#include <fstream>

namespace tablator
{
class VOTable_Field;

class Table
{
public:
  std::vector<std::pair<std::string, Property> > properties;
  std::vector<char> data;
  std::vector<std::string> comments;

  std::vector<Column> columns;
  std::vector<size_t> offsets = { 0 };

  static const std::string null_bitfield_flags_description;

  Table (const std::vector<Column> &Columns,
         const std::map<std::string, std::string> &property_map);

  Table (const std::vector<Column> &Columns)
      : Table (Columns, std::map<std::string, std::string>()) { }

  Table (const boost::filesystem::path &input_path);

  size_t row_size () const { return *offsets.rbegin (); }
  size_t num_rows () const { return data.size () / row_size (); }

  bool is_null (size_t row_offset, size_t column) const
  {
    return data[row_offset + (column - 1) / 8] & (1 << ((column - 1) % 8));
  }

  size_t column_offset (const std::string &name) const
  {
    auto column = find_column (name);
    if (column == columns.end ())
      {
        throw std::runtime_error ("Unable to find column '" + name
                                  + "' in table.");
      }
    return offsets[std::distance (columns.begin (), column)];
  }

  std::vector<Column>::const_iterator find_column (const std::string &name)
    const
  {
    return std::find_if (columns.begin (), columns.end (),
                         [&](const Column &c) { return c.name == name;});
  }

  /// WARNING: append_column routines do not increase the size of the
  /// null column.  The expectation is that the number of columns is
  /// known before adding columns.
  void append_column (const std::string &name, const Data_Type &type)
  {
    append_column (name, type, 1);
  }
  void append_column (const std::string &name, const Data_Type &type,
                      const size_t &size)
  {
    append_column (name, type, size, Field_Properties ());
  }
  
  void append_column (const std::string &name, const Data_Type &type,
                      const size_t &size,
                      const Field_Properties &field_properties)
  {
    append_column (Column (name, type, size, field_properties));
  }

  void append_column (const Column &column);

  void append_row (const Row &row)
  {
    assert (row.data.size () == row_size ());
    data.insert (data.end (), row.data.begin (), row.data.end ());
  }

  void unsafe_append_row (const char *row)
  {
    data.insert (data.end (), row, row + row_size ());
  }

  void pop_row () { data.resize (data.size () - row_size ()); }

  void resize_rows (const size_t &new_num_rows)
  {
    data.resize (row_size () * new_num_rows);
  }

  std::vector<std::pair<std::string, std::string> >
  flatten_properties () const;

  const int output_precision = std::numeric_limits<double>::max_digits10;
  void write_output (std::ostream &os, const Format &format) const;
  void write_output (const boost::filesystem::path &path,
                     const Format &format) const;
  void write_output (const boost::filesystem::path &path) const
  {
    write_output (path, Format (path));
  }
  void write_hdf5 (std::ostream &os) const;
  void write_hdf5 (const boost::filesystem::path &p) const;
  void write_hdf5_to_file (H5::H5File &outfile) const;
  void write_hdf5_attributes (H5::DataSet &table) const;

  void write_ipac_table (std::ostream &os) const;
  void write_ipac_table (const boost::filesystem::path &p)
  {
    boost::filesystem::ofstream outfile (p);
    write_ipac_table (outfile);
  }
  std::vector<size_t> get_column_width () const;
  void write_ipac_table_header (std::ostream &os) const;
  std::string to_ipac_string (const Data_Type &type) const;

  void write_csv_tsv (std::ostream &os, const char &separator) const;
  void write_fits (std::ostream &os) const;
  void write_fits (const boost::filesystem::path &filename) const;
  void write_fits (fitsfile *fits_file) const;
  void read_ipac_table (const boost::filesystem::path &path);
  void read_fits (const boost::filesystem::path &path);
  void read_hdf5 (const boost::filesystem::path &path);
  void read_json5 (const boost::filesystem::path &path);
  void read_property_tree_as_votable (const boost::property_tree::ptree &tree);
  void read_node_and_attributes (const std::string &node_name,
                                 const boost::property_tree::ptree &node);
  void read_node_and_attributes (
      const boost::property_tree::ptree::const_iterator &it)
  {
    read_node_and_attributes (it->first, it->second);
  }
  void read_resource (const boost::property_tree::ptree &resource);
  void read_table (const boost::property_tree::ptree &table);
  VOTable_Field read_field (const boost::property_tree::ptree &field);
  void read_data (const boost::property_tree::ptree &data,
                  const std::vector<VOTable_Field> &fields);
  void read_tabledata (const boost::property_tree::ptree &tabledata,
                       const std::vector<VOTable_Field> &fields);
  void read_csv (const boost::filesystem::path &path);
  void read_csv_rows (const CSV::CSV_Document &csv);
  void set_column_info (CSV::CSV_Document &csv);
  void write_tabledata (std::ostream &os, const bool &is_json) const;
  void write_html (std::ostream &os) const;
  boost::property_tree::ptree
  generate_property_tree (const std::string &tabledata_string) const;

  size_t read_ipac_header (boost::filesystem::ifstream &ipac_file,
                           std::array<std::vector<std::string>, 4> &Columns,
                           std::vector<size_t> &ipac_table_offsets);

  void create_types_from_ipac_headers (
      std::array<std::vector<std::string>, 4> &Columns,
      const std::vector<size_t> &ipac_column_offsets,
      std::vector<size_t> &ipac_column_widths);

  void append_ipac_data_member (const std::string &name,
                                const std::string &data_type,
                                const size_t &size);

  void
  shrink_ipac_string_columns_to_fit (const std::vector<size_t> &column_widths);
};
}
