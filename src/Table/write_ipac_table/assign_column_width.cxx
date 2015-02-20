#include "../../Table.hxx"

void Tablator::Table::assign_column_width ()
{
  std::string name;
  int width;

  const int num_members = compound_type.getNmembers ();
  for (int i = 0; i < num_members; ++i)
    {
      name = compound_type.getMemberName (i);
      switch (types[i])
        {
        case Type::STRING:
          width
              = (name.size () > compound_type.getMemberDataType (i).getSize ())
                    ? name.size ()
                    : compound_type.getMemberDataType (i).getSize ();
          break;
        default:
          width = 20;
        }
      ipac_column_widths.push_back (width);
    }
}
