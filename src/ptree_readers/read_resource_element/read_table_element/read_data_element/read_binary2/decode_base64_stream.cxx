#include <boost/algorithm/string.hpp>
#include <boost/archive/iterators/binary_from_base64.hpp>
#include <boost/archive/iterators/transform_width.hpp>
#include <boost/iterator/filter_iterator.hpp>

#include <cctype>
#include <vector>

namespace {
struct is_not_space {
    bool operator()(char x) { return std::isspace(x) == 0; }
};
}  // namespace
namespace tablator {

std::vector<uint8_t> decode_base64_stream(const std::string &base64_str) {
    using namespace boost::archive::iterators;

    using Iter = transform_width<binary_from_base64<boost::filter_iterator<
                                         is_not_space, std::string::const_iterator> >,
                                 8, 6>;

    unsigned int num_pad_chars = std::count(base64_str.begin(), base64_str.end(), '=');
    std::string base64_str_copy(base64_str);

	// JTODO work backwards?
	// replace '='s (which will all be at the end)  by the base64 encoding of '\0'.
    std::replace(base64_str_copy.begin(), base64_str_copy.end(), '=',
                 'A');

	// Decode!
    std::string result(Iter(std::begin(base64_str_copy)),
                       Iter(std::end(base64_str_copy)));

	// erase trailing '\0' characters.
    result.erase(result.end() - num_pad_chars,
                 result.end());

    return std::vector<uint8_t>(result.begin(), result.end());
}
}  // namespace tablator
