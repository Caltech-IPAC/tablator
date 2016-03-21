#pragma once

#include <string>
#include <utility>
#include <set>
#include <vector>
#include <tuple>
#include <array>

#include <iostream>
#include <fstream>

#include <H5Cpp.h>
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <CCfits/CCfits>

#include "Property.hxx"
#include "Field_Properties.hxx"
#include "Format.hxx"

#include "Row.hxx"
#include "H5_to_Data_Type.hxx"

namespace tablator
{
class VOTable_Field;

class Table
{
public:
  std::vector<std::pair<std::string, Property> > properties;
  std::vector<char> data;
  std::vector<std::string> comments;
  std::vector<Field_Properties> fields_properties;
  H5::CompType compound_type;

  /// compound_type stores only a reference to the type, so we need a
  /// place to store string and array types.
  std::vector<Data_Type> data_types;
  std::vector<size_t> array_sizes;
  std::vector<H5::StrType> string_types;
  std::vector<H5::ArrayType> array_types;

  /// row_size and offsets are duplicative of information in
  /// compound_type.  We store them here to avoid the cost of dynamic
  /// lookups.
  size_t row_size = 0;
  std::vector<size_t> offsets = { 0 };

  static const std::string null_bitfield_flags_description;

  typedef std::pair<std::pair<H5::PredType, size_t>, Field_Properties>
  Column_Properties;
  typedef std::pair<std::string, Column_Properties> Column;

  Table (const std::vector<Column> &columns,
         const std::map<std::string, std::string> &property_map);

  Table (const std::vector<Column> &columns)
      : Table (columns, std::map<std::string, std::string>())
  {
  }

  Table (const boost::filesystem::path &input_path)
      : compound_type (size_t (1))
  {
    Format format (input_path);
    if (format.is_hdf5 ())
      {
        read_hdf5 (input_path);
      }
    else if (format.is_fits ())
      {
        read_fits (input_path);
      }
    else if (format.is_ipac_table ())
      {
        read_ipac_table (input_path);
      }
    else if (format.is_json5 ())
      {
        read_json5 (input_path);
      }
    else if (format.is_votable () || format.is_json ())
      {
        boost::property_tree::ptree tree;
        boost::filesystem::ifstream file (input_path);
        if (format.is_votable ())
          boost::property_tree::read_xml (file, tree);
        else
          boost::property_tree::read_json (file, tree);
        read_property_tree_as_votable (tree);
      }
    else
      {
        throw std::runtime_error ("Unsupported input format: "
                                  + input_path.string ());
      }
  }

  std::vector<Column> columns () const;

  size_t num_rows () const { return data.size () / row_size; }

  bool is_null (size_t row_offset, size_t column) const
  {
    return data[row_offset + (column - 1) / 8] & (1 << ((column - 1) % 8));
  }

  /// WARNING: append_column routines do not increase the size of the
  /// null column.  The expectation is that the number of columns is
  /// known before adding columns.
  void append_string_column (const std::string &name, const size_t &size)
  {
    auto new_string_types (string_types);
    new_string_types.emplace_back (0, size);

    append_column_internal (name, *new_string_types.rbegin (),1);
    using namespace std;
    swap (string_types, new_string_types);
  }

  void append_array_column (const std::string &name, const H5::PredType &type,
                            const size_t &size)
  {
    auto new_array_types (array_types);
    const hsize_t hsize (size);
    new_array_types.emplace_back (type, 1, &hsize);

    append_column_internal (name, *new_array_types.rbegin (), size);
    using namespace std;
    swap (array_types, new_array_types);
  }

  // FIXME: Lots of duplicate code
  void append_array_column (const std::string &name, const H5::ArrayType &type)
  {
    auto new_array_types (array_types);
    new_array_types.emplace_back (type);

    /// H5::ArrayType::getArrayDims is not const.
    int num_dims (const_cast<H5::ArrayType &>(type).getArrayNDims ());
    if (num_dims != 1)
      { throw std::runtime_error ("Invalid dimension of array type.  Only valid "
                                  "value is 1, but got: "
                                  + std::to_string (num_dims)); }
    hsize_t size;
    const_cast<H5::ArrayType &>(type).getArrayDims (&size);

    append_column_internal (name, *new_array_types.rbegin (), size);
    using namespace std;
    swap (array_types, new_array_types);
  }

  void append_column (const std::string &name, const H5::PredType &type)
  {
    append_column_internal (name, type, 1);
  }

  void append_column (const std::string &name, const H5::PredType &type,
                      const size_t &size)
  {
    if (type == H5::PredType::C_S1)
      {
        append_string_column (name, size);
      }
    else if (size != 1)
      {
        append_array_column (name, type, size);
      }
    else
      {
        append_column_internal (name, type, 1);
      }
  }
  
  void append_column_internal (const std::string &name,
                               const H5::DataType &type,
                               const size_t &array_size)
  {
    size_t new_row_size=row_size + type.getSize ();
    auto new_data_types (data_types);
    new_data_types.emplace_back (H5_to_Data_Type (type));

    auto new_array_sizes (array_sizes);
    new_array_sizes.push_back(array_size);

    auto new_offsets (offsets);
    new_offsets.push_back (new_row_size);

    auto new_fields_properties (fields_properties);
    new_fields_properties.push_back (Field_Properties ());

    auto new_compound_type (compound_type);
    new_compound_type.setSize (new_row_size);
    new_compound_type.insertMember (name, row_size, type);

    /// Copy and swap for exception safety.
    row_size=new_row_size;
    using namespace std;
    swap (data_types, new_data_types);
    swap (array_sizes, new_array_sizes);
    swap (offsets, new_offsets);
    swap (fields_properties, new_fields_properties);
    swap (compound_type, new_compound_type);
  }

  void append_row (const Row &row)
  {
    assert (row.data.size () == row_size);
    data.insert (data.end (), row.data.begin (), row.data.end ());
  }

  void unsafe_append_row (const char *row)
  {
    data.insert (data.end (), row, row + row_size);
  }

  void pop_row () { data.resize (data.size () - row_size); }

  void resize_rows (const size_t &new_num_rows)
  {
    data.resize (row_size * new_num_rows);
  }

  std::vector<std::pair<std::string, std::string> >
  flatten_properties () const;

  const int output_precision = std::numeric_limits<double>::max_digits10;
  void write_output (std::ostream &os, const Format &format);
  void write_output (const boost::filesystem::path &path,
                     const Format &format);
  void write_output (const boost::filesystem::path &path)
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
  void write_ipac_table_header (std::ostream &os,
                                const int &num_members) const;
  std::string to_ipac_string (const H5::DataType &type) const;

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
  void write_tabledata (std::ostream &os, const bool &is_json) const;
  void write_html (std::ostream &os) const;
  boost::property_tree::ptree
  generate_property_tree (const std::string &tabledata_string) const;

  size_t read_ipac_header (boost::filesystem::ifstream &ipac_file,
                           std::array<std::vector<std::string>, 4> &columns,
                           std::vector<size_t> &ipac_table_offsets);

  void create_types_from_ipac_headers (
      std::array<std::vector<std::string>, 4> &columns,
      const std::vector<size_t> &ipac_column_offsets,
      std::vector<size_t> &ipac_column_widths);

  void append_ipac_data_member (const std::string &name,
                                const std::string &data_type,
                                const size_t &size);

  void
  shrink_ipac_string_columns_to_fit (const std::vector<size_t> &column_widths);
};
}
