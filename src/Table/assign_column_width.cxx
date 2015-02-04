#include "../../Table.hxx"

void Tablator::Table::assign_column_width ()
{
  std::string name;
  int  width;
 
  const int num_members = compound_type.getNmembers ();
  for (int i=0; i< num_members; ++i) 
    {
      name  = compound_type.getMemberName (i);
      switch (types[i])
        {
          case Type::BOOLEAN:
          case Type::SHORT:
          case Type::INT:
               width = (name.size() > 11) ? name.size() : 11;   
          break;

          case Type::LONG:
               width = (name.size() > 20) ? name.size() : 20;   
               break;

          case Type::FLOAT:
          case Type::DOUBLE:
               width = (name.size() > 16 ) ? name.size() : 16;   
               break;

          case Type::STRING:
               width = (name.size() > compound_type.getMemberDataType(i).getSize())
                     ?  name.size()
                     :  compound_type.getMemberDataType(i).getSize(); 
          break;
        }
        ipac_column_widths.push_back(width);
    }
}
