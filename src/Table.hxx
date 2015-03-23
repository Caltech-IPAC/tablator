#pragma once

#include <string>
#include <map>
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

#include "Property.hxx"
#include "Field_Properties.hxx"
#include "Format.hxx"

namespace Tablator
{
class Table
{
public:
  std::map<std::string, Property> properties;
  std::vector<char> data;
  std::vector<std::string> comment;
  std::vector<Field_Properties> fields_properties;
  H5::CompType compound_type;
  /// Type names are mostly lifted directly from the IVOA TAP spec.
  /// IVOA has fixed length char[] arrays.  We just use a string.
  enum class Type : char
  { BOOLEAN,
    SHORT,
    INT,
    LONG,
    FLOAT,
    DOUBLE,
    STRING };

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
    else
      {
        throw std::runtime_error ("Unsupported input format: "
                                  + input_path.string ());
      }
  }

  size_t size () const { return data.size () / row_size; }

  bool is_null (size_t row_offset, size_t column) const
  {
    return data[row_offset+(column-1)/8] & (1 << ((column-1)%8));
  }

  void clear_nulls (char row[]) const
  {
    std::fill (row,row+row_size,0);
  }

  void set_null (size_t column, char row[]);

  template <typename T>
  void copy_to_row (const T &element, const size_t &offset, char row[])
  {
    assert (offset + sizeof(T) <= row_size);
    // FIXME: I think this is undefined, because element+1 is not
    // guaranteed to be valid
    std::copy (reinterpret_cast<const char *>(&element),
               reinterpret_cast<const char *>(&element + 1), row + offset);
  }

  template <typename T>
  void copy_to_row (const T &begin, const T &end, const size_t &offset,
                    char row[])
  {
    assert (offset < row_size);
    std::copy (begin, end, row + offset);
  }

  void copy_to_row (const std::string &element, const size_t &offset_begin,
                    const size_t &offset_end, char row[])
  {
    std::string element_copy(element);
    element_copy.resize (offset_end-offset_begin,' ');
    std::copy (element_copy.begin (), element_copy.end (), row + offset_begin);
  }
  void insert_row (const char row[])
  {
    data.insert (data.end (), row, row + row_size);
  }

  void pop_row ()
  {
    data.resize (data.size () - row_size);
  }

  std::vector<std::pair<std::string, std::string> >
  flatten_properties () const;

  const int output_precision = 13;
  void read_ipac_table (const boost::filesystem::path &path);
  void write_output (const boost::filesystem::path &path,
                     const Format &format);
  void write_output (const boost::filesystem::path &path)
  {
    write_output (path, Format (path));
  }
  void write_hdf5 (std::ostream &os) const;
  void write_hdf5 (const boost::filesystem::path &p) const;
  void write_hdf5_to_file (H5::H5File &outfile) const;

  void write_ipac_table (std::ostream &os) const;
  void write_ipac_table (const boost::filesystem::path &p)
  {
    boost::filesystem::ofstream outfile (p);
    write_ipac_table (outfile);
  }
  std::vector<size_t> get_column_width () const;
  void write_ipac_table_header (std::ostream &os,
                                const int &num_members) const;
  void write_element_type (std::ostream &os, const int &i) const;

  void write_csv_tsv (std::ostream &os, const char &separator) const;
  void write_fits (const boost::filesystem::path &filename) const;
  void read_fits (const boost::filesystem::path &path);
  void read_hdf5 (const boost::filesystem::path &path);
  void put_table_in_property_tree (boost::property_tree::ptree &table) const;
  void write_html (std::ostream &os) const;
  void write_votable (std::ostream &os) const;

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
