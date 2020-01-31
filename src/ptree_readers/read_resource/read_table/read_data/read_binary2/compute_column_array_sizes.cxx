#include <algorithm>
#include <boost/spirit/include/qi.hpp>

#include "../../../../../data_size.hxx"
#include "../../../VOTable_Field.hxx"
#include "is_null_MSb.hxx"


namespace tablator {
void compute_column_array_sizes(const std::vector<uint8_t> &stream,
                                const std::vector<VOTable_Field> &fields,
                                std::vector<size_t> &column_array_sizes,
                                size_t &num_rows) {
    num_rows = 0;
    if (fields.size() < 2) return;

    const size_t null_flags_size((fields.size() + 6) / 8);
    size_t position(0);
    while (position + null_flags_size < stream.size()) {
        size_t row_offset(position);
        position += null_flags_size;
        for (size_t field = 1; field < fields.size() && position <= stream.size();
             ++field) {
            if (!fields[field].get_is_array_dynamic()) {
                position += data_size(fields[field].get_type()) *
                            fields[field].get_array_size();
            } else {
                if (is_null_MSb(stream, row_offset, field)) {
                    position += sizeof(uint32_t);
                } else {
                    // FIXME: This feels like the hard way to do things.
                    // But I can not use plain old pointers, because I
                    // then run into problems with modifying const
                    // pointers.  So I need iterators to distinguish
                    // between "const iterator" and "const_iterator".
                    uint32_t array_size(0);
                    auto begin = stream.begin();
                    std::advance(begin, position);
                    auto end = stream.begin();
                    position += sizeof(array_size);
                    if (position <= stream.size()) {
                        std::advance(end, position);

                        /// VOTable spec says that array size is a 32
                        /// bit MSB integer

                        /// This parsing can never fail, since any byte
                        /// pattern is a valid 32 bit number
                        boost::spirit::qi::parse(
                                begin, end, boost::spirit::qi::big_dword, array_size);
                        column_array_sizes[field] =
                                std::max(column_array_sizes[field],
                                         static_cast<size_t>(array_size));
                        position += array_size * data_size(fields[field].get_type());
                    }
                }
            }
        }
        if (position <= stream.size()) ++num_rows;
    }
}
}  // namespace tablator
