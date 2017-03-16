#include <iomanip>
#include <inttypes.h>
#include <limits>
#include <cmath>

#include "../../Table.hxx"
#include "../../write_type_as_ascii.hxx"

void tablator::Table::write_ipac_table (std::ostream &os) const
{
  std::vector<size_t> ipac_column_widths = get_column_width ();
  write_ipac_table_header (os);
  int total_record_width = 0;

  os << "|";
  os << std::right;
  auto ipac_column=std::next(ipac_column_widths.begin());
  for (auto column=std::next(columns.begin()); column!=columns.end();
       ++column, ++ipac_column)
    {
      if (column->type == Data_Type::CHAR || column->array_size==1)
        {
          total_record_width += (*ipac_column + 1);
          os << std::setw (*ipac_column) << column->name << "|";
        }
      else
        {
          for (size_t element=0; element<column->array_size; ++element)
            {
              total_record_width += (*ipac_column + 1);
              os << std::setw (*ipac_column)
                 << (column->name + "_" + std::to_string(element)) << "|";
            }
        }
    }
  os << "\n|";

  ipac_column=std::next(ipac_column_widths.begin());
  for (auto column=std::next(columns.begin()); column!=columns.end();
       ++column, ++ipac_column)
    {
      for (size_t element=0; element<column->array_size; ++element)
        {
          os << std::setw (*ipac_column) << to_ipac_string (column->type) << "|";
          if (column->type == Data_Type::CHAR)
            { break; }
        }
    }

  os << "\n|";

  ipac_column=std::next(ipac_column_widths.begin());
  for (auto column=std::next(columns.begin()); column!=columns.end();
       ++column, ++ipac_column)
    {
      auto unit = column->field_properties.attributes.find ("unit");
      if (unit == column->field_properties.attributes.end ())
        {
          for (size_t element=0; element<column->array_size; ++element)
            {
              os << std::setw (*ipac_column) << " " << "|";
              if (column->type == Data_Type::CHAR)
                { break; }
            }
        }
      else
        {
          for (size_t element=0; element<column->array_size; ++element)
            {
              os << std::setw (*ipac_column) << unit->second << "|";
              if (column->type == Data_Type::CHAR)
                { break; }
            }
        }
    }
  os << "\n|";

  ipac_column=std::next(ipac_column_widths.begin());
  for (auto column=std::next(columns.begin()); column!=columns.end();
       ++column, ++ipac_column)
    {
      auto null = column->field_properties.values.null;
      if (null.empty ())
        {
          for (size_t element=0; element<column->array_size; ++element)
            {
              os << std::setw (*ipac_column) << "null" << "|";
              if (column->type == Data_Type::CHAR)
                { break; }
            }
        }
      else
        {
          for (size_t element=0; element<column->array_size; ++element)
            {
              os << std::setw (*ipac_column) << null << "|";
              if (column->type == Data_Type::CHAR)
                { break; }
            }
        }
    }
  os << "\n";

  std::stringstream ss;
  for (size_t row_offset = 0; row_offset < data.size ();
       row_offset += row_size ())
    {
      /// Skip the null bitfield flag
      for (size_t i = 1; i < columns.size (); ++i)
        {
          for (size_t element=0; element<columns[i].array_size; ++element)
            {
              ss << " " << std::setw (ipac_column_widths[i]);
              auto data_type = columns[i].type;
              size_t offset = element*data_size(data_type) + offsets[i]
                + row_offset;
              if (is_null (row_offset, i))
                {
                  auto null_value = columns[i].field_properties.values.null;
                  if (null_value.empty ())
                    ss << "null";
                  else
                    ss << null_value;
                }
              else
                {
                  /// Silently convert unsigned 64 bit ints to signed 64
                  /// bit ints, since ipac tables do not support unsigned
                  /// 64 bit ints.
                  if (data_type == Data_Type::UINT64_LE)
                    { data_type = Data_Type::INT64_LE; }
                  /// Do some gymnastics because we want to write a byte
                  /// as an int
                  if (data_type == Data_Type::UINT8_LE)
                    {
                      ss << static_cast<const uint16_t>
                        (static_cast<uint8_t>(*(data.data () + offset)));
                    }
                  else
                    {
                      std::stringstream ss_temp;
                      write_type_as_ascii (ss_temp, data_type,
                                           (data_type == Data_Type::CHAR
                                            ? columns[i].array_size : 1),
                                           data.data () + offset,
                                           output_precision);
                      std::string s(ss_temp.str());
                      /// Turn newlines into spaces
                      auto newline_location (s.find('\n'));
                      while (newline_location != std::string::npos)
                        {
                          s[newline_location]=' ';
                          newline_location = s.find('\n',newline_location+1);
                        }
                      ss << s;
                    }
                }
              if (data_type == Data_Type::CHAR)
                { break; }
            }
        }
      ss << " \n";
      os << ss.str ();
      ss.str ("");
    }
}
