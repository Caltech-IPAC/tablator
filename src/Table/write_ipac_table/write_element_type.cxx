#include "../../Table.hxx"

void Tablator::Table::write_element_type (std::ostream &os, const int &i) const
{
  switch (types[i])
    {
    case Type::BOOLEAN:
    case Type::SHORT:
    case Type::INT:
      os << "integer";
      break;

    case Type::LONG:
      os << "long";
      break;

    case Type::FLOAT:
    case Type::DOUBLE:
      os << "double";
      break;

    case Type::STRING:
      os << "char";
      break;
    }
}
