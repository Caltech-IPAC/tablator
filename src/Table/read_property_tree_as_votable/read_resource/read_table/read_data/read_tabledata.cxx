#include <boost/lexical_cast.hpp>

#include "../../../../../Table.hxx"
#include "../../../../../to_string.hxx"
#include "../../../../insert_ascii_in_row.hxx"

void tablator::Table::read_tabledata (const boost::property_tree::ptree &tabledata,
                                      const std::vector<std::string> &names)
{
  std::vector<std::vector<std::string> > rows;
  std::vector<size_t> column_width (types.size ());
  const size_t null_flags_size((types.size () + 6)/8);
  column_width.at(0)=null_flags_size;
  for (auto &tr: tabledata)
    {
      if (tr.first == "TR")
        {
          rows.push_back ({});
          auto td=tr.second.begin ();
          if (td != tr.second.end () && td->first == "<xmlattr>.ID")
            ++td;
          for (std::size_t c=1; c<types.size (); ++c)
            {
              if (td == tr.second.end ())
                throw std::runtime_error
                  ("Not enough columns in row "
                   + std::to_string (rows.size ())
                   + ".  Expected "
                   + std::to_string (types.size () - 1)
                   + ", but only got " + std::to_string (c-1)
                   + ".");

              if (td->first == "TD")
                {
                  std::string temp=td->second.get_value<std::string> ();
                  column_width[c]=std::max (column_width[c], temp.size ());
                  rows.rbegin ()->emplace_back(temp);
                }
              else
                {
                  throw std::runtime_error
                    ("Expected TD inside RESOURCE.TABLE.DATA.TABLEDATA.TR, "
                     "but found: " + td->first);
                }
              // FIXME: Check encoding
              ++td;
            }
          if (td != tr.second.end ())
            throw std::runtime_error ("Too many elements in row "
                                      + std::to_string (rows.size ())
                                      + ".  Only expected "
                                      + std::to_string (types.size () - 1)
                                      + ".");
        }
      else if (tr.first != "<xmlattr>.encoding")
        {
          throw std::runtime_error
            ("Expected TR inside RESOURCE.TABLE.DATA.TABLEDATA, but found: "
             + tr.first);
        }
    }

  row_size = 0;
  offsets.push_back (0);
  for (std::size_t c=0; c<types.size (); ++c)
    {
      if (types[c]==H5::PredType::C_S1)
        {
          string_types.emplace_back (0, column_width[c]);
          append_member(names.at(c), *string_types.rbegin ());
        }
      else
        {
          append_member(names.at(c), types[c]);
        }
    }

  Row row_string (row_size);
  for (size_t current_row=0; current_row<rows.size (); ++current_row)
    {
      auto &row(rows[current_row]);
      row_string.clear_nulls ();
      for (size_t column=1; column<types.size (); ++column)
        {
          auto &element(row[column-1]);
          if (element.empty ())
            {
              row_string.set_null (column, types[column], offsets);
            }
          else
            try
              {
                insert_ascii_in_row (types[column], element, column, offsets,
                                     row_string);
              }
            catch (std::exception &error)
              {
                throw std::runtime_error ("Invalid " + to_string (types[column])
                                          + " in row "
                                          + std::to_string(current_row + 1)
                                          + ", field "
                                          + std::to_string(column)
                                          + ".  Found '"
                                          + element + "'");
              }
        }
      append_row (row_string);
    }
}
