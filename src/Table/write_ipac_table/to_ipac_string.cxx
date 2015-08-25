#include "../../Table.hxx"

std::string tablator::Table::to_ipac_string (const tablator::Table::Type &type) const
{
  std::string result;
  switch (type)
    {
    case Type::BOOLEAN:
    case Type::SHORT:
    case Type::INT:
      result="int";
      break;

    case Type::LONG:
      result="long";
      break;

    case Type::FLOAT:
    case Type::DOUBLE:
      result="double";
      break;

    case Type::STRING:
      result="char";
      break;
    }
  return result;
}
