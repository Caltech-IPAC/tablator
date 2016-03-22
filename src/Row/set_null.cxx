#include <stdexcept>

#include "../Row.hxx"
#include "../data_size.hxx"

void tablator::Row::set_null (const Data_Type &data_type,
                              const size_t &array_size, const size_t &column,
                              const size_t &offset, const size_t &offset_end)
{
  const int byte = (column - 1) / 8;
  const char mask = (1 << ((column - 1) % 8));
  data[byte] = data[byte] | mask;

  if (array_size != 1)
    {
      for (size_t o = offset; o < offset_end; o += data_size (data_type))
        { set_null (data_type, 1, column, o, o + data_size (data_type)); }
    }
  else
    {
      switch (data_type)
        {
        case Data_Type::INT8_LE:
          insert (std::numeric_limits<int8_t>::max (), offset);
          break;
        case Data_Type::UINT8_LE:
          insert (std::numeric_limits<uint8_t>::max (), offset);
          break;
        case Data_Type::INT16_LE:
          insert (std::numeric_limits<int16_t>::max (), offset);
          break;
        case Data_Type::UINT16_LE:
          insert (std::numeric_limits<uint16_t>::max (), offset);
          break;
        case Data_Type::INT32_LE:
          insert (std::numeric_limits<int32_t>::max (), offset);
          break;
        case Data_Type::UINT32_LE:
          insert (std::numeric_limits<uint32_t>::max (), offset);
          break;
        case Data_Type::INT64_LE:
          insert (std::numeric_limits<int64_t>::max (), offset);
          break;
        case Data_Type::UINT64_LE:
          insert (std::numeric_limits<uint64_t>::max (), offset);
          break;
        case Data_Type::FLOAT32_LE:
          insert (std::numeric_limits<float>::quiet_NaN (), offset);
          break;
        case Data_Type::FLOAT64_LE:
          insert (std::numeric_limits<double>::quiet_NaN (), offset);
          break;
        case Data_Type::CHAR:
          insert ('\0', offset);
          break;
        default:
          throw std::runtime_error (
            "Unexpected HDF5 data type in tablator::Row::set_null");
        }
    }
}
