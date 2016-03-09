#include <stdexcept>

#include "../Row.hxx"

void tablator::Row::set_null (const H5::DataType &type, const size_t &column,
                              const size_t &offset, const size_t &offset_end)
{
  int byte = (column - 1) / 8;
  char mask = (1 << ((column - 1) % 8));
  data[byte] = data[byte] | mask;

  if (type == H5::PredType::STD_I8LE)
    {
      insert (std::numeric_limits<int8_t>::max (), offset);
    }
  else if (type == H5::PredType::STD_U8LE)
    {
      insert (std::numeric_limits<uint8_t>::max (), offset);
    }
  else if (type == H5::PredType::STD_I16LE)
    {
      insert (std::numeric_limits<int16_t>::max (), offset);
    }
  else if (type == H5::PredType::STD_U16LE)
    {
      insert (std::numeric_limits<uint16_t>::max (), offset);
    }
  else if (type == H5::PredType::STD_I32LE)
    {
      insert (std::numeric_limits<int32_t>::max (), offset);
    }
  else if (type == H5::PredType::STD_U32LE)
    {
      insert (std::numeric_limits<uint32_t>::max (), offset);
    }
  else if (type == H5::PredType::STD_I64LE)
    {
      insert (std::numeric_limits<int64_t>::max (), offset);
    }
  else if (type == H5::PredType::STD_U64LE)
    {
      insert (std::numeric_limits<uint64_t>::max (), offset);
    }
  else if (type == H5::PredType::IEEE_F32LE)
    {
      insert (std::numeric_limits<float>::quiet_NaN (), offset);
    }
  else if (type == H5::PredType::IEEE_F64LE)
    {
      insert (std::numeric_limits<double>::quiet_NaN (), offset);
    }
  else if (type.getClass () == H5T_STRING)
    {
      std::fill (data.data () + offset, data.data () + offset_end, 0);
    }
  else if (type.getClass () == H5T_ARRAY)
    {
      H5::DataType d = type.getSuper ();
      for (size_t o = offset; o < offset_end; o += d.getSize ())
        set_null (d, column, o, o + d.getSize ());
    }
  else
    {
      throw std::runtime_error (
          "Unexpected HDF5 data type in tablator::Row::set_null");
    }
}
