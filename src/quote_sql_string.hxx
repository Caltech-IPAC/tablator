#include <string>

namespace tablator {
enum class Quote_SQL { ALWAYS, IF_NEEDED };

std::string quote_sql_string(const std::string &input, const char &quote,
                             const Quote_SQL &quote_sql);
inline std::string quote_sql_string(const std::string &input, const char &quote) {
    return quote_sql_string(input, quote, Quote_SQL::ALWAYS);
}
}  // namespace tablator
