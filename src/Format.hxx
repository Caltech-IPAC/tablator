#pragma once

#include <string>
#include <map>
#include <utility>
#include <sstream>
#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>

namespace tablator
{
class Format
{

public:
  enum class enum_format
  { JSON,
    VOTABLE,
    CSV,
    TSV,
    FITS,
    IPAC_TABLE,
    TEXT,
    HTML,
    HDF5,
    UNKNOWN };

  // FIXME: This should really be a static, but then I ran into
  // problems with order of static initialization, where something
  // static would get destroyed before this variable, causing crashes
  // on exit.  On the other hand, having this const breaks the default
  // operator=().
  const std::map<Format::enum_format,
                 std::pair<std::string, std::vector<std::string> > > formats{
    { Format::enum_format::JSON, { "json", { "js", "json" } } },
    { Format::enum_format::VOTABLE, { "votable", { "xml", "vot", "vo" } } },
    { Format::enum_format::CSV, { "csv", { "csv" } } },
    { Format::enum_format::TSV, { "tsv", { "tsv" } } },
    { Format::enum_format::FITS, { "fits", { "fits" } } },
    { Format::enum_format::IPAC_TABLE, { "ipac_table", { "tbl" } } },
    { Format::enum_format::TEXT, { "text", { "txt" } } },
    { Format::enum_format::HTML, { "html", { "html" } } },
    { Format::enum_format::HDF5, { "hdf5", { "h5", "hdf", "hdf5" } } },
    { Format::enum_format::UNKNOWN, { "", {} } }
  };

  std::map<Format::enum_format,
           std::pair<std::string, std::vector<std::string> > >::const_iterator
  index;

  std::string extension () const
  {
    if (index->second.second.empty ())
      return "";
    return "." + index->second.second.at (0);
  }

  Format () : index (formats.end ()) {}
  Format (const boost::filesystem::path &path) { set_from_extension (path); }

  void init (const std::string &format)
  {
    for (index = formats.begin (); index != formats.end (); ++index)
      {
        if (boost::iequals (index->second.first, format))
          break;
      }

    if (index == formats.end ())
      throw std::runtime_error ("Unknown format: " + format);
  }

  Format (const std::string &format) { init (format); }

  Format (const char *format) : Format (std::string (format)) {}

  void set_from_extension (const boost::filesystem::path &path);

  std::string content_type () const
  {
    if (index == formats.end ())
      throw std::runtime_error ("INTERNAL ERROR: Unknown format when "
                                "generating content type");

    std::string result;
    switch (index->first)
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

      default:
        throw std::runtime_error (
            "INTERNAL ERROR: Unknown format when "
            "generating content type: "
            + std::to_string (static_cast<int>(index->first)));
      }
    return result;
  }

  bool is_ipac_table () const
  {
    return index->first == enum_format::IPAC_TABLE;
  }
  bool is_votable () const { return index->first == enum_format::VOTABLE; }
  bool is_csv () const { return index->first == enum_format::CSV; }
  bool is_tsv () const { return index->first == enum_format::TSV; }
  bool is_text () const { return index->first == enum_format::TEXT; }
  bool is_fits () const { return index->first == enum_format::FITS; }
  bool is_html () const { return index->first == enum_format::HTML; }
  bool is_hdf5 () const { return index->first == enum_format::HDF5; }

  std::string string () const { return index->second.first; }
};
}

inline std::ostream &operator<<(std::ostream &os, const tablator::Format &f)
{
  os << f.string ();
  return os;
}
