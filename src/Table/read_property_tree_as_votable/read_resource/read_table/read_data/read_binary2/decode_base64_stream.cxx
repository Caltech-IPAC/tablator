#include <boost/archive/iterators/binary_from_base64.hpp>
#include <boost/archive/iterators/transform_width.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/iterator/filter_iterator.hpp>

#include <cctype>

namespace
{
struct is_not_space
{
  bool operator()(char x) { return std::isspace (x) == 0; }
};
}
namespace tablator
{
/// This does not trim nulls at the end, because there is no way of
/// distinguishing between nulls added for padding and real nulls in
/// the data.

std::vector<uint8_t> decode_base64_stream (const std::string &val)
{
  using namespace boost::archive::iterators;
  using It
      = transform_width<binary_from_base64<boost::
                                               filter_iterator<is_not_space,
                                                               std::string::
                                                                   const_iterator> >,
                        8, 6>;
  return std::vector<uint8_t>(It (std::begin (val)), It (std::end (val)));
}
}
