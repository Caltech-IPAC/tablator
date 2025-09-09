#include <string>

#include <boost/algorithm/string.hpp>
#include <boost/archive/iterators/base64_from_binary.hpp>
#include <boost/archive/iterators/insert_linebreaks.hpp>
#include <boost/archive/iterators/transform_width.hpp>

#include <fstream>
#include <iostream>
#include <sstream>

// https://stackoverflow.com/questions/7053538/how-do-i-encode-a-string-to-base64-using-only-boost

namespace tablator {

using namespace boost::archive::iterators;

const std::string encode_base64_stream(const uint8_t *bin2_ptr, size_t bin2_len) {
    typedef insert_linebreaks<base64_from_binary<transform_width<const char *, 6, 8> >,
                              64>
            it_base64_t;

    uint num_pad_chars = (3 - bin2_len % 3) % 3;
    std::string base64_str(it_base64_t(bin2_ptr), it_base64_t(bin2_ptr + bin2_len));
    base64_str.append(num_pad_chars, '=');
    return base64_str;
}
}  // namespace tablator
