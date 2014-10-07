#include <iomanip>

#include <boost/property_tree/ptree.hpp>

#include "../Table.hxx"

void
TAP::Table::put_table_in_property_tree (boost::property_tree::ptree &table)
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
            case TAP::Table::Type::BOOLEAN:
              td << static_cast<int>(data[i + offsets[j]]);
              break;
            case TAP::Table::Type::SHORT:
              td << *reinterpret_cast<const int16_t *>(data.data () + i
                                                       + offsets[j]);
              break;
            case TAP::Table::Type::INT:
              td << *reinterpret_cast<const int32_t *>(data.data () + i
                                                       + offsets[j]);
              break;
            case TAP::Table::Type::LONG:
              td << *reinterpret_cast<const int64_t *>(data.data () + i
                                                       + offsets[j]);
              break;
            case TAP::Table::Type::FLOAT:
              td << std::setprecision (output_precision)
                 << *reinterpret_cast<const float *>(data.data () + i
                                                     + offsets[j]);
              break;
            case TAP::Table::Type::DOUBLE:
              td << std::setprecision (output_precision)
                 << *reinterpret_cast<const double *>(data.data () + i
                                                      + offsets[j]);
              break;
            case TAP::Table::Type::STRING:
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
