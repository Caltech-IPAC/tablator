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

#include "Property.hxx"
#include "Field_Properties.hxx"
#include "Format.hxx"

#include "Row.hxx"

namespace tablator
{
class Table
{
public:
  std::vector<std::pair<std::string, Property> > properties;
  std::vector<char> data;
  std::vector<std::string> comments;
  std::vector<Field_Properties> fields_properties;
  H5::CompType compound_type;

  /// These members are redundant with information in compound_type.
  /// We precompute them so that we do not have to do dynamic lookups
  /// for each row.

  std::vector<Type> types;

  /// offsets has an extra element at the end so that we can get the
  /// size of any element.  This is needed for strings.
  std::vector<size_t> offsets;
  size_t row_size;

  /// Need to keep a copy of the string types around, since
  /// compound_type only has a reference to the type, not a copy of
  /// the type.
  std::vector<H5::StrType> string_types;

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
    : compound_type (size_t(1))
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
    else if (format.is_votable ()
             || format.is_json ())
      {
        boost::property_tree::ptree tree;
        boost::filesystem::ifstream file(input_path);
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

  size_t num_rows () const { return data.size () / row_size; }

  bool is_null (size_t row_offset, size_t column) const
  {
    return data[row_offset+(column-1)/8] & (1 << ((column-1)%8));
  }

  // FIXME: add_member feels a little magic.
  void append_member (const std::string &name, const H5::DataType &type)
  {
    size_t member_size=type.getSize ();
    size_t old_size=row_size;
    row_size+=member_size;
    compound_type.setSize (row_size);
    compound_type.insertMember (name, old_size, type);
    offsets.push_back (row_size);
  }
  
  void append_row (const Row &row)
  {
    data.insert (data.end (), row.data.begin (), row.data.end ());
  }

  void pop_row ()
  {
    data.resize (data.size () - row_size);
  }

  std::vector<std::pair<std::string, std::string> >
  flatten_properties () const;

  const int output_precision = 13;
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
  std::string to_ipac_string (const tablator::Type &type) const;

  void write_csv_tsv (std::ostream &os, const char &separator) const;
  void write_fits (const boost::filesystem::path &filename) const;
  void read_ipac_table (const boost::filesystem::path &path);
  void read_fits (const boost::filesystem::path &path);
  void read_hdf5 (const boost::filesystem::path &path);
  void read_json5 (const boost::filesystem::path &path);
  void read_property_tree_as_votable (const boost::property_tree::ptree &tree);
  void read_node_and_attributes (const std::string &node_name,
                                 const boost::property_tree::ptree &node);
  void read_node_and_attributes (const boost::property_tree::ptree::const_iterator &it)
  {
    read_node_and_attributes (it->first, it->second);
  }
  void read_resource (const boost::property_tree::ptree &resource);
  void read_table (const boost::property_tree::ptree &resource);
  std::string read_field (const boost::property_tree::ptree &resource);
  void read_data (const boost::property_tree::ptree &data,
                  const std::vector<std::string> &names);
  void read_tabledata (const boost::property_tree::ptree &tabledata,
                       const std::vector<std::string> &names);
  void put_table_in_property_tree (boost::property_tree::ptree &table) const;
  void write_html (std::ostream &os) const;
  boost::property_tree::ptree generate_property_tree () const;

  size_t read_ipac_header
  (boost::filesystem::ifstream &ipac_file,
   std::array<std::vector<std::string>,4> &columns,
   std::vector<size_t> &ipac_table_offsets);

  void create_types_from_ipac_headers
  (std::array<std::vector<std::string>,4> &columns,
   std::vector<size_t> &ipac_column_offsets,
   std::vector<size_t> &ipac_column_widths);
};
}
