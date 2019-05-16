#include "quote_sql_string.hxx"

#include <boost/regex.hpp>

#include <sstream>

namespace tablator {
std::string quote_sql_string(const std::string &input, const char &quote,
                             const Quote_SQL &quote_sql) {
    if (quote_sql == Quote_SQL::IF_NEEDED) {
        const boost::regex simple_characters("^[a-zA-Z0-9_]+$");
        if (boost::regex_match(input, simple_characters)) {
            return input;
        }
    }

    std::stringstream stream;
    stream << quote;
    for (auto &c : input) {
        stream << c;
        if (c == quote) {
            stream << quote;
        }
    }
    stream << quote;
    return stream.str();
}
}  // namespace tablator
