#include <iomanip>
#include <limits>
#include <cmath>

#include <boost/property_tree/ptree.hpp>

#include "../Table.hxx"
#include "../to_string.hxx"

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
              if (types[column]==H5::PredType::STD_I8LE)
                {
                  td << static_cast<int>(data[row_offset + offsets[column]]);
                }
              else if (types[column]==H5::PredType::STD_I16LE)
                {
                  td << *reinterpret_cast<const int16_t *>(data.data ()
                                                           + row_offset
                                                           + offsets[column]);
                }
              else if (types[column]==H5::PredType::STD_I32LE)
                {
                  td << *reinterpret_cast<const int32_t *>(data.data ()
                                                           + row_offset
                                                           + offsets[column]);
                }
              else if (types[column]==H5::PredType::STD_I64LE)
                {
                  td << *reinterpret_cast<const int64_t *>(data.data ()
                                                           + row_offset
                                                           + offsets[column]);
                }
              else if (types[column]==H5::PredType::IEEE_F32LE)
                {
                  td << std::setprecision (output_precision)
                     << *reinterpret_cast<const float *>(data.data ()
                                                         + row_offset
                                                         + offsets[column]);
                }
              else if (types[column]==H5::PredType::IEEE_F64LE)
                {
                  td << std::setprecision (output_precision)
                     << *reinterpret_cast<const double *>(data.data ()
                                                          + row_offset
                                                          + offsets[column]);
                }
              else if (types[column]==H5::PredType::C_S1)
                {
                  /// The characters in the type can be shorter than the
                  /// number of allowed bytes.  So add a .c_str() that
                  /// will terminate the string at the first null.
                  td << std::string
                    (data.data () + row_offset + offsets[column],
                     compound_type.getMemberDataType (column).getSize ()).c_str ();
                }
              // FIXME: Handle arrays
              else
                {
                  throw std::runtime_error
                    ("Unknown data type when writing data: "
                     + to_string (types[column]));
                }
            }
          tr.add ("TD", td.str ());
        }
    }
}
