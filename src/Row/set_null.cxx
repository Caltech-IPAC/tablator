#include "../Row.hxx"

void tablator::Row::set_null (size_t column, const Type &type,
                              const std::vector<size_t> offsets)
{
  int byte=(column-1)/8;
  char mask= (1 << ((column-1)%8));
  data[byte]=data[byte] | mask;

  switch (type)
    {
    case Type::BOOLEAN:
      insert (std::numeric_limits<int8_t>::max (), offsets[column]);
      break;
    case Type::SHORT:
      insert (std::numeric_limits<int16_t>::max (), offsets[column]);
      break;
    case Type::INT:
      insert (std::numeric_limits<int32_t>::max (), offsets[column]);
      break;
    case Type::LONG:
      insert (std::numeric_limits<int64_t>::max (), offsets[column]);
      break;
    case Type::FLOAT:
      insert (std::numeric_limits<float>::quiet_NaN (), offsets[column]);
      break;
    case Type::DOUBLE:
      insert (std::numeric_limits<double>::quiet_NaN (), offsets[column]);
      break;
    case Type::STRING:
      std::fill (data.data() + offsets[column],
                 data.data() + offsets[column+1], 0);
      break;
    }
}