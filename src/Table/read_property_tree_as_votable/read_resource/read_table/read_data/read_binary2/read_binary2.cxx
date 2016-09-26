#include "../../../../../../Table.hxx"
#include "../../VOTable_Field.hxx"

namespace tablator
{
std::vector<uint8_t> decode_base64_stream(const std::string &val);
void compute_column_array_sizes (const std::vector<uint8_t> &stream,
                                 const std::vector<VOTable_Field> &fields,
                                 std::vector<size_t> &column_array_sizes);

void Table::read_binary2 (const boost::property_tree::ptree &binary2,
                          const std::vector<VOTable_Field> &fields)
{
  std::vector<size_t> column_array_sizes (fields.size (), 1);
  const size_t null_flags_size ((fields.size () + 6) / 8);
  column_array_sizes.at (0) = null_flags_size;
  std::vector<std::vector<uint8_t> > streams;
  for (auto &stream: binary2)
    {
      if (stream.first != "STREAM")
        throw std::runtime_error ("Unknown element in BINARY2.  Expected "
                                  "STREAM, but found: " + stream.first);
      std::string encoding;
      for (auto &stream_child : stream.second)
        {
          if (stream_child.first == "<xmlattr>")
            {
              for (auto &attribute : stream_child.second)
                {
                  if (attribute.first != "encoding")
                    throw std::runtime_error ("Unknown STREAM attribute.  "
                                              "Expected 'encoding', but found: "
                                              + attribute.first);
                  encoding = attribute.second.get_value<std::string>();
                }
            }
        }
      if (encoding.empty())
        throw std::runtime_error ("Could not find encoding attribute in STREAM");
      if (encoding != "base64")
        throw std::runtime_error ("Only base64 encoding is "
                                  "supported, but found: "
                                  + encoding);
      streams.emplace_back(decode_base64_stream
                           (stream.second.get_value<std::string>()));
    }

  for (auto &stream: streams)
    compute_column_array_sizes(stream,fields,column_array_sizes);
  
  for (std::size_t c = 0; c < fields.size (); ++c)
    append_column (fields.at (c).name, fields[c].type, column_array_sizes[c]);

  // for (auto &stream: streams)
  //   append_data_from_stream(stream,fields);
}
}
