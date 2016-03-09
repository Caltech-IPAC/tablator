#include <boost/lexical_cast.hpp>

#include "../../../../../../Table.hxx"
#include "../../../../../../to_string.hxx"
#include "../../../../../insert_ascii_in_row.hxx"
#include "../../VOTable_Field.hxx"

namespace tablator
{
size_t count_elements (const std::string &entry, const H5::PredType &predtype);
}

void
tablator::Table::read_tabledata (const boost::property_tree::ptree &tabledata,
                                 const std::vector<VOTable_Field> &fields)
{
  std::vector<std::vector<std::string> > rows;
  /// Need to set the size to at least 1, because H5::StrType can not
  /// handle zero sized strings.
  std::vector<size_t> array_sizes (fields.size (), 1);
  const size_t null_flags_size ((fields.size () + 6) / 8);
  array_sizes.at (0) = null_flags_size;
  for (auto &tr : tabledata)
    {
      if (tr.first == "TR" || tr.first.empty ())
        {
          /// Add something for the null_bitfields_flag
          rows.push_back ({});
          auto td = tr.second.begin ();
          if (td != tr.second.end () && td->first == "<xmlattr>.ID")
            ++td;
          for (std::size_t c = 1; c < fields.size (); ++c)
            {
              if (td == tr.second.end ())
                throw std::runtime_error (
                    "Not enough columns in row "
                    + std::to_string (rows.size ()) + ".  Expected "
                    + std::to_string (fields.size () - 1) + ", but only got "
                    + std::to_string (c - 1) + ".");

              if (td->first == "TD" || td->first.empty ())
                {
                  std::string temp = td->second.get_value<std::string>();
                  if (fields.at (c).is_array)
                    array_sizes[c] = std::max (
                        array_sizes[c],
                        count_elements (temp, fields.at (c).predtype));
                  rows.rbegin ()->emplace_back (temp);
                }
              else
                {
                  throw std::runtime_error (
                      "Expected TD inside RESOURCE.TABLE.DATA.TABLEDATA.TR, "
                      "but found: " + td->first);
                }
              // FIXME: Check encoding
              ++td;
            }
          if (td != tr.second.end ())
            throw std::runtime_error (
                "Too many elements in row " + std::to_string (rows.size ())
                + ".  Only expected " + std::to_string (fields.size () - 1)
                + ".");
        }
      else if (tr.first != "<xmlattr>.encoding")
        {
          throw std::runtime_error (
              "Expected TR inside RESOURCE.TABLE.DATA.TABLEDATA, but found: "
              + tr.first);
        }
    }

  for (std::size_t c = 0; c < fields.size (); ++c)
    {
      if (fields[c].predtype == H5::PredType::C_S1)
        {
          append_string_member (fields.at (c).name, array_sizes[c]);
        }
      else if (fields[c].is_array)
        {
          append_array_member (fields.at (c).name, fields.at (c).predtype,
                               array_sizes[c]);
        }
      else
        {
          append_member (fields.at (c).name, fields.at (c).predtype);
        }
    }

  Row row_string (row_size);
  for (size_t current_row = 0; current_row < rows.size (); ++current_row)
    {
      auto &row (rows[current_row]);
      row_string.set_zero ();
      for (size_t column = 1; column < fields.size (); ++column)
        {
          auto &element (row[column - 1]);
          if (element.empty ())
            {
              row_string.set_null (compound_type.getMemberDataType (column),
                                   column, offsets[column],
                                   offsets[column + 1]);
            }
          else
            try
            {
              insert_ascii_in_row (compound_type.getMemberDataType (column),
                                   column, element, offsets[column],
                                   offsets[column + 1], row_string);
            }
          catch (std::exception &error)
          {
            throw std::runtime_error (
                "Invalid " + to_string (fields[column].predtype) + " in row "
                + std::to_string (current_row + 1) + ", field "
                + std::to_string (column) + ".  Found '" + element + "'");
          }
        }
      append_row (row_string);
    }
}
