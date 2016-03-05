#include <iomanip>
#include <limits>
#include <cmath>

#include <boost/property_tree/ptree.hpp>

#include "../Table.hxx"
#include "../to_string.hxx"
#include "write_type_as_ascii.hxx"

void tablator::Table::put_table_in_property_tree
(boost::property_tree::ptree &table,
 const bool &is_json) const
{
  const std::string tr_element (is_json ? "" : "TR"),
    td_element (is_json ? "" : "TD");
  for (size_t row_offset = 0; row_offset < data.size (); row_offset += row_size)
    {
      boost::property_tree::ptree &tr = table.add (tr_element, "");
      /// Skip the null bitfield flag
      for (int column = 1; column < compound_type.getNmembers (); ++column)
        {
          std::stringstream td;
          if (!is_null(row_offset,column))
            {
              H5::DataType datatype=compound_type.getMemberDataType (column);
              write_type_as_ascii
                (td, datatype, data.data () + row_offset + offsets[column],
                 datatype.getSize (), output_precision);
            }
          tr.add (td_element, td.str ());
        }
    }
}
