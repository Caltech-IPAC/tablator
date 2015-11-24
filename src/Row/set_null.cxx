#include <stdexcept>

#include "../Row.hxx"

void tablator::Row::set_null (size_t column, const H5::PredType &type,
                              const std::vector<size_t> &offsets)
{
  int byte=(column-1)/8;
  char mask= (1 << ((column-1)%8));
  data[byte]=data[byte] | mask;

  if (type==H5::PredType::STD_I8LE)
    {
      insert (std::numeric_limits<int8_t>::max (), offsets[column]);
    }
  else if (type==H5::PredType::STD_U8LE)
    {
      insert (std::numeric_limits<uint8_t>::max (), offsets[column]);
    }
  else if (type==H5::PredType::STD_I16LE)
    {
      insert (std::numeric_limits<int16_t>::max (), offsets[column]);
    }
  else if (type==H5::PredType::STD_U16LE)
    {
      insert (std::numeric_limits<uint16_t>::max (), offsets[column]);
    }
  else if (type==H5::PredType::STD_I32LE)
    {
      insert (std::numeric_limits<int32_t>::max (), offsets[column]);
    }
  else if (type==H5::PredType::STD_U32LE)
    {
      insert (std::numeric_limits<uint32_t>::max (), offsets[column]);
    }
  else if (type==H5::PredType::STD_I64LE)
    {
      insert (std::numeric_limits<int64_t>::max (), offsets[column]);
    }
  else if (type==H5::PredType::STD_U64LE)
    {
      insert (std::numeric_limits<uint64_t>::max (), offsets[column]);
    }
  else if (type==H5::PredType::IEEE_F32LE)
    {
      insert (std::numeric_limits<float>::quiet_NaN (), offsets[column]);
    }
  else if (type==H5::PredType::IEEE_F64LE)
    {
      insert (std::numeric_limits<double>::quiet_NaN (), offsets[column]);
    }
  else if (type==H5::PredType::C_S1)
    {
      std::fill (data.data() + offsets[column],
                 data.data() + offsets[column+1], 0);
    }
  else
    {
      throw std::runtime_error
        ("Unexpected HDF5 data type in tablator::Row::set_null");
    }
}
