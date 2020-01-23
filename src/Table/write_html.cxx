#include <boost/property_tree/xml_parser.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>

#include "../Data_Type_Adjuster.hxx"
#include "../Table.hxx"


/**********************************************************/

void tablator::Table::write_html(std::ostream &os) const {
    boost::property_tree::ptree tree;
    boost::property_tree::ptree &html = tree.add("html", "");
    boost::property_tree::ptree &style =
            html.add("style",
                     "body {\n"
                     "font-family: Helvetica, Arial, sans-serif;\n"
                     "font-size: small;\n"
                     "color: black;\n"
                     "background-color: white;\n"
                     "}\n"
                     "table {\n"
                     "border-collapse:separate;\n"
                     "border-spacing: 0.2em;\n"
                     "margin: 1em 1em 1em 4em;\n"
                     "white-space: nowrap;\n"
                     "}\n"
                     "td {\n"
                     "margin: 0.25em;\n"
                     "padding: 0 0.5em 0 0.5em;\n"
                     "text-align: left;\n"
                     "vertical-align: middle;\n"
                     "border: 1px solid white;\n"
                     "}\n"
                     "td.num {\n"
                     "text-align: right;\n"
                     "}\n"
                     "th {\n"
                     "margin: 0.25em;\n"
                     "padding: 0.25em 0.5em 0.25em 0.5em;\n"
                     "font-weight: bold;\n"
                     "color: grey;\n"
                     "background-color: #DDD;\n"
                     "text-align: center;\n"
                     "vertical-align: middle;\n"
                     "border: 1px solid white;\n"
                     "}\n"
                     "th.meta {\n"
                     "color: black;\n"
                     "background-color: #AAA;\n"
                     "}\n"
                     "tr:hover {\n"
                     "color: #00F;\n"
                     "background: #EEF;\n"
                     "border: 1px solid blue;\n"
                     "}\n"
                     "td:hover {\n"
                     "border: 1px solid #DDF;\n"
                     "}\n");
    style.add("<xmlattr>.type", "text/css");
    boost::property_tree::ptree &table = html.add("body.table", "");

    boost::property_tree::ptree &heading_tr = table.add("TR", "");
    /// skip null_bitfield_flag
    const auto &columns = get_columns();
    for (size_t i = 1; i < columns.size(); ++i)
        heading_tr.add("TH", columns[i].get_name());

    std::string tabledata_string(
            boost::uuids::to_string(boost::uuids::random_generator()()));
    table.add("TR", tabledata_string);
    os << "<!DOCTYPE HTML>\n";
    std::stringstream ss;
    // FIXME: This uses the undocumented function write_xml_element
    // since write_xml always writes the <?xml...> header.
    boost::property_tree::xml_parser::write_xml_element(
            ss, std::string(), tree, -1,
            boost::property_tree::xml_writer_settings<char>(' ', 2));
    std::string s(ss.str());
    size_t tabledata_offset(s.find(tabledata_string));
    os << s.substr(0, tabledata_offset - 4);
    write_tabledata(os, Format::Enums::HTML);
    os << s.substr(tabledata_offset + tabledata_string.size() + 5);
}
