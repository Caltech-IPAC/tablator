#include <json5_parser.h>
#include <boost/filesystem/fstream.hpp>

#include "../Table.hxx"

void tablator::Table::read_json5 (const boost::filesystem::path &path)
{
  json5_parser::Value parse_tree;
  {
    boost::filesystem::ifstream input_stream (path);
    json5_parser::read_or_throw (input_stream, parse_tree);
  }

  /// FIXME: This is rather inefficient.  It reads the file as json5,
  /// writes into a stringstream as json, then reads it again as json.
  std::stringstream ss;
  json5_parser::write (parse_tree, ss, json5_parser::none, output_precision);
  boost::property_tree::ptree tree;
  boost::property_tree::read_json (ss, tree);
  read_property_tree_as_votable (tree);
}
