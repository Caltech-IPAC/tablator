#include <iomanip>
#include <limits>
#include <cmath>

#include <boost/property_tree/ptree.hpp>

#include "../Table.hxx"

void
Tablator::Table::put_table_in_property_tree (boost::property_tree::ptree &table)
    const
{
  for (size_t i = 0; i < data.size (); i += row_size)
    {
      boost::property_tree::ptree &tr = table.add ("TR", "");
      for (size_t j = 0; j < types.size (); ++j)
        {
          std::stringstream td;
          switch (types[j])
            {
            case Type::BOOLEAN:
                 /// FIXME: need to update Null value checking late
                 if (static_cast<int>(data[i + offsets[j]]) == -9)
                     td << nulls[j];
                 else
                     td << static_cast<int>(data[i + offsets[j]]);
            break;

            case Type::SHORT:
                 if (*reinterpret_cast<const int16_t *>
                     (data.data () + i + offsets[j]) == std::numeric_limits<int16_t>::has_quiet_NaN)
                     td << nulls[j];
                 else
                   {
                     td << *reinterpret_cast<const int16_t *>
                           (data.data () + i + offsets[j]);
                   }
            break;

            case Type::INT:
                 if (*reinterpret_cast<const int32_t *>
                     (data.data () + i + offsets[j]) == std::numeric_limits<int32_t>::has_quiet_NaN)
                     td << nulls[j];
                 else
                   {
                     td << *reinterpret_cast<const int32_t *>
                           (data.data () + i + offsets[j]);
                   }
            break;

            case Type::LONG:
                 if (*reinterpret_cast<const int64_t *>
                     (data.data () + i + offsets[j]) == std::numeric_limits<int64_t>::has_quiet_NaN)
                     td << nulls[j];
                 else
                   {
                     td << *reinterpret_cast<const int64_t *>
                          (data.data () + i + offsets[j]);
                   }
            break;

            case Type::FLOAT:
                 if (*reinterpret_cast<const float *>
                     (data.data () + i + offsets[j]) == std::numeric_limits<float>::has_quiet_NaN)
                     td << nulls[j];
                 else
                   {
                     td << std::setprecision (output_precision)
                        << *reinterpret_cast<const float *>
                           (data.data () + i + offsets[j]);
                   }
            break;

            case Type::DOUBLE:
                 if (*reinterpret_cast<const double *>
                     (data.data () + i + offsets[j]) == std::numeric_limits<double>::has_quiet_NaN)
                    td << nulls[j];
                 else
               {
                    td << std::setprecision (output_precision)
                       << *reinterpret_cast<const double *>(data.data () + i
                                                      + offsets[j]);
               }
              break;

            case Type::STRING:
              td << std::string (
                        data.data () + i + offsets[j],
                        compound_type.getMemberDataType (j).getSize ());
            break;

            default:
              throw std::runtime_error
                ("Unknown data type when writing data: "
                 + std::to_string (static_cast<int>(types[j])));
            }
          tr.add ("TD", td.str ());
        }
    }
}
