#pragma once

#include <string>
#include <map>
#include <utility>
#include <sstream>
#include <boost/algorithm/string.hpp>

class Format
{

public:
  enum class enum_format
  {
    VOTABLE,
    CSV,
    TSV,
    FITS,
    IPAC_TABLE,
    TEXT,
    HTML,
    HDF5,
    UNKNOWN
  };
  static std::map<enum_format, std::pair<std::string, std::string> > formats;
  enum_format index;

  std::string extension () const
  {
    if (formats[index].second.empty ())
      return "";
    return "." + formats[index].second;
  }

  Format () : index (enum_format::UNKNOWN) {}
  Format (const std::string &format)
  {
    index = enum_format::UNKNOWN;
    for (auto &f : formats)
      {
        if (boost::iequals (f.second.first, format))
          {
            index = f.first;
            break;
          }
      }

    if (index == enum_format::UNKNOWN)
      throw std::runtime_error ("Unknown format: " + format);
  }

  void set_from_extension (const std::string &extension)
  {
    index = enum_format::IPAC_TABLE;
    for (auto &f : formats)
      {
        if (boost::iequals (f.second.second, extension))
          {
            index = f.first;
            break;
          }
      }
  }

  std::string content_type () const
  {
    std::string result;

    switch (index)
      {
      case enum_format::CSV:
        result = "Content-type: text/csv\r\n\r\n";
        break;

      case enum_format::TSV:
        result = "Content-type: text/tab-separated-values\r\n\r\n";
        break;

      case enum_format::TEXT:
      case enum_format::IPAC_TABLE:
        result = "Content-type: text/plain\r\n\r\n";
        break;

      case enum_format::VOTABLE:
        result = "Content-type: application/x-votable+xml\r\n\r\n";
        break;

      case enum_format::FITS:
        result = "Content-type: application/fits\r\n\r\n";
        break;

      case enum_format::HTML:
        result = "Content-type: text/html\r\n\r\n";
        break;

      case enum_format::HDF5:
        result = "Content-type: application/x-hdf\r\n\r\n";
        break;

      case enum_format::UNKNOWN:
      default:
        {
          throw std::runtime_error ("INTERNAL ERROR: Unknown format when "
                                    "generating content type: "
                                    + std::to_string(static_cast<int>(index)));
        }
      }
    return result;
  }

  bool is_ipac_table () const { return index == enum_format::IPAC_TABLE; }
  bool is_votable () const { return index == enum_format::VOTABLE; }
  bool is_csv () const { return index == enum_format::CSV; }
  bool is_tsv () const { return index == enum_format::TSV; }
  bool is_text () const { return index == enum_format::TEXT; }
  bool is_fits () const { return index == enum_format::FITS; }
  bool is_html () const { return index == enum_format::HTML; }
  bool is_hdf5 () const { return index == enum_format::HDF5; }

  std::string string () const { return formats[index].first; }
};

inline std::ostream &operator<<(std::ostream &os, const Format &f)
{
  os << f.string ();
  return os;
}
