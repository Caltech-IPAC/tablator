#include <iomanip>

#include "../../Table.hxx"

namespace {
std::string convert_newlines(const std::string &input) {
    std::string result(input);
    for (auto &c : result) {
        if (c == '\n') {
            c = ' ';
        }
    }
    return result;
}
}  // namespace

void tablator::Table::write_ipac_table_header(std::ostream &os) const {
    static constexpr size_t KEYWORD_ALIGNMENT = 8;

    static constexpr char const *FIXLEN_STRING = "fixlen = T";
    static constexpr char const *JSON_DESC_LABEL = "RESOURCE.TABLE.DESCRIPTION";

    os << "\\" << FIXLEN_STRING << "\n";
    os << std::left;

    os << "\\" << ROWS_RETRIEVED_KEYWORD << " = " << num_rows() << "\n";


    std::vector<std::string> json_comments;
    // Standard refers to keywords rather than properties.

    // Write keywords first, followed by comments.
    for (auto &name_and_property : properties) {
        auto &p = name_and_property.second;
        if (!p.value.empty()) {
            if (boost::equals(name_and_property.first, JSON_DESC_LABEL)) {
                // This is a comment (or concatenated comments) from the Json5 format.
                // Err on the side of respecting newlines rather than replacing with
                // spaces.
                std::string val(p.value);
                boost::algorithm::trim(val);
                std::vector<std::string> elements;
                boost::split(elements, val, boost::is_any_of("\n"));
                for (const std::string &elt : elements) {
                    if (find(comments.begin(), comments.end(), elt) == comments.end()) {
                        json_comments.emplace_back(elt);
                    }
                }
            } else {
                os << "\\" << std::setw(KEYWORD_ALIGNMENT)
                   << convert_newlines(name_and_property.first) << " = "
                   << "'" << convert_newlines(p.value) << "'\n";
            }
        }
        auto &a = p.attributes;
        auto name(a.find("name")), value(a.find("value"));
        if (a.size() == 2 && name != a.end() && value != a.end()) {
            // We wrote these at the top of this function.
            if (boost::equals(name->second, FIXLEN_KEYWORD) ||
                boost::equals(name->second, ROWS_RETRIEVED_KEYWORD)) {
                continue;
            }
            os << "\\" << std::setw(KEYWORD_ALIGNMENT) << convert_newlines(name->second)
               << " = "
               << "'" << convert_newlines(value->second) << "'\n";
        } else {
            for (auto &attr : a) {
                os << "\\" << std::setw(KEYWORD_ALIGNMENT)
                   << convert_newlines(name_and_property.first + "." + attr.first)
                   << " = "
                   << "'" << convert_newlines(attr.second) << "'\n";
            }
        }
    }

    // Stream the comments so far.
    for (auto &c : json_comments) {
        std::stringstream ss(c);
        std::string line;
        while (std::getline(ss, line)) {
            os << "\\ " << line << "\n";
        }
    }
    for (auto &c : comments) {
        std::stringstream ss(c);
        std::string line;
        while (std::getline(ss, line)) os << "\\ " << line << "\n";
    }

    // Add default comments (column name with unit and description) if they aren't
    // present already.
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
