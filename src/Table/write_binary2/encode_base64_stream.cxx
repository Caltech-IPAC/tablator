#include <string>

#include <boost/algorithm/string.hpp>
#include <boost/archive/iterators/base64_from_binary.hpp>
#include <boost/archive/iterators/transform_width.hpp>
#include <boost/archive/iterators/insert_linebreaks.hpp>

#include <fstream>
#include <iostream>
#include <sstream>




// https://stackoverflow.com/questions/7053538/how-do-i-encode-a-string-to-base64-using-only-boost

namespace tablator {

using namespace boost::archive::iterators;

const std::string encode_base64_stream(const char *bin2_ptr, size_t bin2_len) {
#if 1
	std::ofstream debug_stream;
	std::string debug_file =  "/home/judith/repos/tablator/bin2/tablator/debug_encode.txt";
	  debug_stream.open(debug_file.c_str());
	
	  // debug_stream << "encode_base64_stream(), enter" << std::endl << std::flush;
#endif
    typedef insert_linebreaks<base64_from_binary<transform_width<const char *, 6, 8> >,
                              72>
            it_base64_t;
    // Encode
    unsigned int num_pad_chars = (3 - bin2_len % 3) % 3;
	// debug_stream << "encode_base64_stream(), num_pad_chars: " << num_pad_chars << std::endl;
	std::string base64_str(it_base64_t(bin2_ptr), it_base64_t(bin2_ptr + bin2_len));
	// debug_stream << "encode_base64_stream(), after encode()" << std::endl;
	// debug_stream << "encode_base64_stream(), temp str: " << base64_str << std::endl;
    base64_str.append(num_pad_chars, '=');
	// debug_stream << "encode_base64_stream(), final str: " << base64_str << std::endl;
    return base64_str;
}
}
