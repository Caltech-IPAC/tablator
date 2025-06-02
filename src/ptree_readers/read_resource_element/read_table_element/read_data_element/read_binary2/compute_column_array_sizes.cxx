#include <algorithm>
#include <boost/spirit/include/qi.hpp>

#include "../../../../../data_size.hxx"
#include "../../../../../ptree_readers.hxx"
#include "is_null_MSb.hxx"


namespace tablator {
void compute_column_array_sizes(
        const std::vector<uint8_t> &stream,
        const std::vector<ptree_readers::Field_And_Flag> &field_flag_pairs,
        std::vector<size_t> &column_array_sizes, size_t &num_rows) {
    num_rows = 0;
    if (field_flag_pairs.size() < 2) return;

    const size_t null_flags_size((field_flag_pairs.size() + 6) / 8);
    size_t position(0);
    while (position + null_flags_size < stream.size()) {
        size_t row_offset(position);
        position += null_flags_size;
        for (size_t field_idx = 1;
             field_idx < field_flag_pairs.size() && position <= stream.size();
             ++field_idx) {
            const auto &field = field_flag_pairs[field_idx].get_field();
            bool is_array_dynamic =
                    field_flag_pairs[field_idx].get_dynamic_array_flag();
            if (!is_array_dynamic) {
			  column_array_sizes[field_idx] = field.get_array_size();
                position += data_size(field.get_type()) * field.get_array_size();

            } else {
                if (is_null_MSb(stream, row_offset, field_idx)) {
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

                        // VOTable spec says that array size is a 32
                        // bit MSB integer

                        // This parsing can never fail, since any byte
                        // pattern is a valid 32 bit number
                        boost::spirit::qi::parse(
                                begin, end, boost::spirit::qi::big_dword, array_size);
                        column_array_sizes[field_idx] =
                                std::max(column_array_sizes[field_idx],
                                         static_cast<size_t>(array_size));
                        position += array_size * data_size(field.get_type());
                    }
                }
            }
        }  // end of loop through fields
        if (position <= stream.size()) {
            ++num_rows;
        }
    }
}
}  // namespace tablator
