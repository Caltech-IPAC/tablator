#include "../Table.hxx"

void tablator::Table::set_null (size_t column, char row[])
{
  int byte=(column-1)/8;
  char mask= (1 << ((column-1)%8));
  row[byte]=row[byte] | mask;

  switch (types[column])
    {
    case Type::BOOLEAN:
      copy_to_row (std::numeric_limits<int8_t>::max (), offsets[column], row);
      break;
    case Type::SHORT:
      copy_to_row (std::numeric_limits<int16_t>::max (), offsets[column], row);
      break;
    case Type::INT:
      copy_to_row (std::numeric_limits<int32_t>::max (), offsets[column], row);
      break;
    case Type::LONG:
      copy_to_row (std::numeric_limits<int64_t>::max (), offsets[column], row);
      break;
    case Type::FLOAT:
      copy_to_row (std::numeric_limits<float>::quiet_NaN (), offsets[column], row);
      break;
    case Type::DOUBLE:
      copy_to_row (std::numeric_limits<double>::quiet_NaN (), offsets[column], row);
      break;
    case Type::STRING:
      std::fill (row + offsets[column], row + offsets[column+1], 0);
      break;
    }
}
