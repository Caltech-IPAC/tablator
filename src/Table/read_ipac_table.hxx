#pragma once

#include <stdexcept>
#include <exception>

#include <boost/filesystem/fstream.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/regex.hpp>

#include "../Table.hxx"

namespace Tablator
{
const boost::regex name_regex ("[a-zA-Z_]+[a-zA-Z0-9_]*");
const boost::regex type_regex ("^[icdflrICDFLR][a-ilnortuA-ILORTU]{2,}");

inline bool check_bar_position (std::vector<int> &off, std::string &str)
{
  for (size_t i = 0; i < off.size () - 1; i++)
    if (str.at (off[i]) != '|')
      return false;
  return true;
}

inline std::vector<int> get_ipac_column_widths (std::vector<int> &off)
{
  std::vector<int> ipac_column_widths;
  for (size_t i = 0; i < off.size () - 1; i++)
    ipac_column_widths.push_back (off[i + 1] - off[i] - 1);
  return ipac_column_widths;
}

inline std::vector<int> find_ipac_column_offset (std::string &str, char find)
{
  std::vector<int> ipac_column_offset;
  for (size_t i = 0; i < str.size (); i++)
    if (str[i] == find)
      ipac_column_offset.push_back (i);
  return ipac_column_offset;
}

inline bool validate_ipac_header (std::vector<std::string> &vec,
                                  const boost::regex &vec_regex)
{
  bool good_headerline = false;
  for (auto &v : vec)
    {
      v = boost::algorithm::trim_copy (v);
      if (!(good_headerline = (regex_match (v, vec_regex))))
        throw std::runtime_error ("Column name/datatype '" + v
                                  + "' contains an invalid character.");
    }
  return good_headerline;
}

inline std::vector<Tablator::Table::Type>
assign_data_type (std::vector<std::string> &vec)
{
  std::vector<Tablator::Table::Type> types;
  for (auto &t : vec)
    {
      if (boost::iequals (t, "double") || boost::iequals (t, "float")
          || boost::iequals (t, "real"))
        types.push_back (Tablator::Table::Type::DOUBLE);

      if (boost::iequals (t, "char"))
        types.push_back (Tablator::Table::Type::STRING);

      if (boost::iequals (t, "int") || boost::iequals (t, "integer"))
        types.push_back (Tablator::Table::Type::INT);

      if (boost::iequals (t, "long"))
        types.push_back (Tablator::Table::Type::LONG);
    }
  return types;
}

void trim (char *str)
{

  char *p = str;
  for (int l = (int)strlen (p) - 1; l >= 0; --l)
    {
      if (' ' == p[l] || '\t' == p[l])
        p[l] = '\0';
      else
        break;
    }
  for (int l = 0; l < (int)strlen (p); ++l)
    {
      if (' ' != p[l])
        {
          p += l;
          break;
        }
    }
  if (str != p)
    memmove (str, p, strlen (p) + 1);
}
}
