#include "../Ipac_Table_Writer.hxx"

#include <iomanip>

#include "../Table.hxx"

static constexpr size_t KEYWORD_ALIGNMENT = 8;

/*******************************************************/
/* Helper functions */
/*******************************************************/

namespace {

void write_keyword_header_line(std::ostream &os, const std::string &name,
                               const std::string &value) {
    os << "\\" << std::setw(KEYWORD_ALIGNMENT)
       << tablator::Ipac_Table_Writer::convert_newlines(name) << " = "
       << "'" << tablator::Ipac_Table_Writer::convert_newlines(value) << "'\n";
}

/*******************************************************/

void store_json_comments(const std::string &value,
                         const std::vector<std::string> &comments,
                         std::vector<std::string> &json_comments) {
    std::string val(value);
    boost::algorithm::trim(val);
    std::vector<std::string> elements;
    boost::split(elements, val, boost::is_any_of("\n"));
    for (const std::string &elt : elements) {
        if (find(comments.begin(), comments.end(), elt) == comments.end()) {
            json_comments.emplace_back(elt);
        }
    }
}

/*******************************************************/

void write_comment_lines(std::ostream &os, const std::vector<std::string> &comments) {
    for (auto &c : comments) {
        std::stringstream ss(c);
        std::string line;
        while (std::getline(ss, line)) {
            os << "\\ " << line << "\n";
        }
    }
}

/*******************************************************/

void generate_and_write_default_comments(
        const tablator::Table &table, std::ostream &os,
        const std::vector<std::string> &json_comments) {
    auto columns = table.columns;
    auto comments = table.comments;
    for (size_t i = 1; i < columns.size(); ++i) {
        if (!columns[i].field_properties.attributes.empty() ||
            !columns[i].field_properties.description.empty()) {
            std::string col_comment(columns[i].name);
            auto unit = columns[i].field_properties.attributes.find("unit");
            if (unit != columns[i].field_properties.attributes.end() &&
                !unit->second.empty()) {
                col_comment.append(" (").append(unit->second).append(")");
            }

            if (find(comments.begin(), comments.end(), col_comment) != comments.end() ||
                find(json_comments.begin(), json_comments.end(), col_comment) !=
                        json_comments.end()) {
                continue;
            }

            os << "\\ " << col_comment << "\n";
            if (!columns[i].field_properties.description.empty())
                os << "\\ ___ " << columns[i].field_properties.description << "\n";
            // FIXME: Write out description attributes
        }
    }
}
}  // namespace

/*******************************************************/
/* Titular function */
/*******************************************************/

void tablator::Ipac_Table_Writer::write_ipac_table_header(const Table &table,
                                                          std::ostream &os,
                                                          int num_requested_rows) {
    static constexpr char const *FIXLEN_STRING = "fixlen = T";
    static constexpr char const *JSON_DESC_LABEL = "RESOURCE.TABLE.DESCRIPTION";


    os << "\\" << FIXLEN_STRING << "\n";
    os << std::left;

    size_t num_rows_to_report = (num_requested_rows == WRITE_ALL_ROWS)
                                        ? table.num_rows()
                                        : num_requested_rows;

    os << "\\" << tablator::Table::ROWS_RETRIEVED_KEYWORD << " = " << num_rows_to_report
       << "\n";

    auto &comments = table.comments;
    std::vector<std::string> json_comments;

    // Iterate through properties, distinguishing between keywords and json5-ified
    // comments. Write keywords here and save json_comments for a later loop. (Standard
    // refers to keywords rather than properties.)
    for (auto &name_and_property : table.properties) {
        auto &p = name_and_property.second;
        if (!p.value.empty()) {
            if (boost::equals(name_and_property.first, JSON_DESC_LABEL)) {
                // This is a comment (or concatenated comments) from the Json5 format.
                // Err on the side of respecting newlines rather than replacing with
                // spaces.
                store_json_comments(p.value, comments, json_comments);
            } else {
                write_keyword_header_line(os, name_and_property.first, p.value);
            }
        }
        auto &a = p.attributes;
        auto name(a.find("name")), value(a.find("value"));
        if (a.size() == 2 && name != a.end() && value != a.end()) {
            // We wrote these at the top of this function.
            if (boost::equals(name->second, tablator::Table::FIXLEN_KEYWORD) ||
                boost::equals(name->second, tablator::Table::ROWS_RETRIEVED_KEYWORD)) {
                continue;
            }
            write_keyword_header_line(os, name->second, value->second);
        } else {
            for (auto &attr : a) {
                write_keyword_header_line(
                        os, name_and_property.first + "." + attr.first, attr.second);
            }
        }
    }

    // Stream json_comments followed by comments.
    write_comment_lines(os, json_comments);
    write_comment_lines(os, comments);

    // Add default comments (column name with unit and description) if they aren't
    // present already.
    generate_and_write_default_comments(table, os, json_comments);
}
