#pragma once

#include <string>
#include <map>
#include <set>
#include <vector>
#include <tuple>

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
  std::vector<std::string> comment, nulls;
  std::vector<Field_Properties> fields_properties;
  H5::CompType compound_type;
  std::vector<int> ipac_column_widths;
  /// Type names are mostly lifted directly from the IVOA TAP spec.
  /// IVOA has fixed length char[] arrays.  We just use a string.
  enum class Type : char
  {
    BOOLEAN,
    SHORT,
    INT,
    LONG,
    FLOAT,
    DOUBLE,
    STRING
  };

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

  typedef std::pair<std::pair<H5::PredType, size_t>, Field_Properties> Column_Properties;
  typedef std::pair<std::string, Column_Properties> Column;

  Table (const std::vector<Column> &columns,
         const std::map<std::string, std::string> &property_map);

  Table (const std::vector<Column> &columns)
      : Table (columns, std::map<std::string, std::string>())
  {
  }

  Table (const boost::filesystem::path &input_path)
  {
    Format format(input_path);
    if(format.is_hdf5())
      {
        read_hdf5(input_path);
      }
    else if(format.is_fits())
      {
        read_fits(input_path);
      }
    else if(format.is_ipac_table())
      {
        read_ipac_table(input_path);
      }
  }

  size_t size () const { return data.size () / row_size; }

  template <typename T>
  void copy_to_row (const T &element, const size_t &offset, char row[])
  {
    assert (offset < row_size);
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

  void insert_row (const char row[])
  {
    data.insert (data.end (), row, row + row_size);
  }

  std::vector<std::pair<std::string, std::string> >
  flatten_properties () const;

  const int output_precision = 13;
  void read_ipac_table (const boost::filesystem::path &path);
  void write_output (const boost::filesystem::path &path,
                     const Format &format);
  void write_output (const boost::filesystem::path &path)
  {
    write_output(path,Format(path));
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
  void assign_column_width ();
};
}
