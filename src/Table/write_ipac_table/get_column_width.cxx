#include "../../Table.hxx"

std::vector<size_t> tablator::Table::get_column_width () const
{
  std::vector<size_t> widths;
  std::string name;
  int width;

  const int num_members = compound_type.getNmembers ();
  for (int i = 0; i < num_members; ++i)
    {
      name = compound_type.getMemberName (i);
      if (compound_type.getMemberDataType (i).getClass ()==H5T_STRING)
        {
          /// The minimum of 4 is to accomodate the length of the
          /// literals 'char' and 'null'.
          width =
            std::max ((size_t)4,
                      std::max (name.size (),
                                compound_type.getMemberDataType (i).getSize ()));
        }
      else
        {
          width = output_precision+7;
        }
      widths.push_back (width);
    }
  return widths;
}
