#include <iomanip>
#include <limits>
#include <cmath>

#include <boost/property_tree/ptree.hpp>

#include "../Table.hxx"
#include "../to_string.hxx"
#include "write_type_as_ascii.hxx"

void tablator::Table::put_table_in_property_tree (
    boost::property_tree::ptree &table) const
{
  for (size_t row_offset = 0; row_offset < data.size (); row_offset += row_size)
    {
      boost::property_tree::ptree &tr = table.add ("TR", "");
      /// Skip the null bitfield flag
      for (size_t column = 1; column < types.size (); ++column)
        {
          std::stringstream td;
          if (!is_null(row_offset,column))
            {
              write_type_as_ascii
                (td, types[column], data.data () + row_offset + offsets[column],
                 compound_type.getMemberDataType (column).getSize (),
                 output_precision);
            }
          tr.add ("TD", td.str ());
        }
    }
}
