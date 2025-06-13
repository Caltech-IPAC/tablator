#include "../../../../../ptree_readers.hxx"

#include "../../../../../Data_Element.hxx"
#include "../../../../../Utils/Table_Utils.hxx"

namespace tablator {
std::vector<uint8_t> decode_base64_stream(const std::string &val);
void compute_column_array_sizes(
        const std::vector<uint8_t> &stream,
        const std::vector<ptree_readers::Field_And_Flag> &field_flag_pairs,
        std::vector<size_t> &column_array_sizes, size_t &num_rows);


//==============================================================

Data_Element ptree_readers::read_binary2(
        const boost::property_tree::ptree &binary2,
        const std::vector<ptree_readers::Field_And_Flag> &field_flag_pairs) {
    std::vector<size_t> column_array_sizes(field_flag_pairs.size(), 1);
    const size_t null_flags_size((field_flag_pairs.size() + 6) / 8);
    column_array_sizes.at(0) = null_flags_size;
    std::vector<std::vector<uint8_t> > streams;
    for (auto &stream : binary2) {
        if (stream.first == XMLCOMMENT) {
            continue;
        }
        if (stream.first != STREAM)
            throw std::runtime_error(
                    "Unknown element in BINARY2.  Expected " +
                    STREAM + ", but found: " +
                    stream.first);
        std::string encoding;
        for (auto &stream_child : stream.second) {
            if (stream_child.first == XMLATTR) {
                for (auto &attribute : stream_child.second) {
                    if (attribute.first != "encoding")
                        throw std::runtime_error(
                                "Unknown STREAM attribute.  "
                                "Expected 'encoding', but found: " +
                                attribute.first);
                    encoding = attribute.second.get_value<std::string>();
                }
            }
        }
        if (encoding.empty())
            throw std::runtime_error("Could not find encoding attribute in STREAM");
        if (encoding != "base64")
            throw std::runtime_error(
                    "Only base64 encoding is "
                    "supported, but found: " +
                    encoding);
        streams.emplace_back(
                decode_base64_stream(stream.second.get_value<std::string>()));
    }

    std::vector<size_t> rows_per_stream;
    for (auto &stream : streams) {
        size_t num_rows;
        compute_column_array_sizes(stream, field_flag_pairs, column_array_sizes,
                                   num_rows);
        rows_per_stream.push_back(num_rows);
    }

    std::vector<Column> columns;
    std::vector<size_t> offsets = {0};
    std::vector<uint8_t> data;

    for (std::size_t c = 0; c < field_flag_pairs.size(); ++c) {
        const auto &field = field_flag_pairs.at(c).get_field();
        append_column(columns, offsets, field.get_name(), field.get_type(),
                      column_array_sizes[c], field.get_field_properties(),
                      field_flag_pairs.at(c).get_dynamic_array_flag());
    }
    for (std::size_t stream = 0; stream < streams.size(); ++stream) {
        append_data_from_stream(data, columns, offsets, streams[stream],
                                field_flag_pairs, rows_per_stream[stream]);
    }
    return Data_Element(columns, offsets, data);
}
}  // namespace tablator
