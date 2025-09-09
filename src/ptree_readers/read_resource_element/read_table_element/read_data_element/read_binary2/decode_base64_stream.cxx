#include <boost/algorithm/string.hpp>
#include <boost/archive/iterators/binary_from_base64.hpp>
#include <boost/archive/iterators/remove_whitespace.hpp>
#include <boost/archive/iterators/transform_width.hpp>
#include <boost/iterator/filter_iterator.hpp>

#include <cctype>
#include <vector>

// FIXME: A more recent version of Boost might allow us to use a
// built-in filter rather than is_not_space.

// https://stackoverflow.com/questions/71932617/c-boost-base64-decoder-fails-when-newlines-are-present


namespace {
struct is_not_space {
    bool operator()(char x) { return std::isspace(x) == 0; }
};
}  // namespace
namespace tablator {
// This does not trim nulls at the end, because there is no way of
// distinguishing between nulls added for padding and real nulls in
// the data.

std::vector<uint8_t> decode_base64_stream(const std::string &val) {
    using namespace boost::archive::iterators;
    using It = transform_width<binary_from_base64<boost::filter_iterator<
                                       is_not_space, std::string::const_iterator> >,
                               8, 6>;
    return std::vector<uint8_t>(It(std::begin(val)), It(std::end(val)));
}
}  // namespace tablator
