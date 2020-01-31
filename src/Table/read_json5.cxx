#include <json5_parser.h>
#include <boost/filesystem/fstream.hpp>

#include "../Table.hxx"
#include "../ptree_readers.hxx"

void tablator::Table::read_json5(std::istream &input_stream) {
    json5_parser::Value parse_tree;
    json5_parser::read_or_throw(input_stream, parse_tree);

    /// FIXME: This is rather inefficient.  It reads the file as json5,
    /// writes into a stringstream as json, then reads it again as json.
    std::stringstream ss;
    json5_parser::write(parse_tree, ss, json5_parser::none);
    boost::property_tree::ptree tree;
    boost::property_tree::read_json(ss, tree);
    ptree_readers::read_property_tree_as_votable(*this, tree);
}
