#include "../../Table.hxx"

void TAP::Table::write_element_type (std::ostream &os, const int &i) const
{
  switch (types[i])
    {
    case Type::BOOLEAN:
    case Type::SHORT:
    case Type::INT:
    case Type::LONG:
      os << "integer";
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
