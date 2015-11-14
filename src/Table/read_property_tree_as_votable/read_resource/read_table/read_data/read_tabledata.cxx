#include <boost/lexical_cast.hpp>

#include "../../../../../Table.hxx"

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
      switch (types[c])
        {
        case Type::BOOLEAN:
          append_member(names.at(c), H5::PredType::NATIVE_UCHAR);
          break;
        case Type::SHORT:
          append_member(names.at(c), H5::PredType::NATIVE_INT16);
          break;
        case Type::INT:
          append_member(names.at(c), H5::PredType::NATIVE_INT32);
          break;
        case Type::LONG:
          append_member(names.at(c), H5::PredType::NATIVE_INT64);
          break;
        case Type::FLOAT:
          append_member(names.at(c), H5::PredType::NATIVE_FLOAT);
          break;
        case Type::DOUBLE:
          append_member(names.at(c), H5::PredType::NATIVE_DOUBLE);
          break;
        case Type::STRING:
          string_types.emplace_back (0, column_width[c]);
          append_member(names.at(c), *string_types.rbegin ());
          break;
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
            switch (types[column])
              {
              case Type::BOOLEAN:
                {
                  if (!boost::iequals(element, "true")
                      && !boost::iequals(element, "false")
                      && !element.empty ())
                    throw std::runtime_error
                      ("Invalid 'boolean' in row " + std::to_string(current_row + 1)
                        + ", field " + std::to_string(column)
                       + ".  Expected  'true', 'false', or empty, but found '"
                       + element + "'");
                  int8_t result=boost::iequals(element, "true");
                  row_string.insert (result, offsets[column]);
                }
                break;
              case Type::SHORT:
                try
                  {
                    int result=boost::lexical_cast<int> (element);
                    if (result > std::numeric_limits<int16_t>::max ()
                        || result < std::numeric_limits<int16_t>::lowest ())
                      throw std::exception ();
                    row_string.insert (static_cast<int16_t> (result),
                                       offsets[column]);
                  }
                catch (std::exception &error)
                  {
                    throw std::runtime_error ("Invalid 'short' in row "
                                              + std::to_string(current_row + 1)
                                              + ", field "
                                              + std::to_string(column)
                                              + ".  Found '"
                                              + element + "'");
                  }
                break;
              case Type::INT:
                try
                  {
                    long result=boost::lexical_cast<long> (element);
                    if (result > std::numeric_limits<int32_t>::max ()
                        || result < std::numeric_limits<int32_t>::lowest ())
                      throw std::exception ();
                    row_string.insert (static_cast<int32_t> (result),
                                       offsets[column]);
                  }
                catch (std::exception &error)
                  {
                    throw std::runtime_error ("Invalid 'int' in row "
                                              + std::to_string(current_row + 1)
                                              + ", field "
                                              + std::to_string(column)
                                              + ".  Found '"
                                              + element + "'");
                  }
                break;
              case Type::LONG:
                try
                  {
                    long long result=boost::lexical_cast<long long> (element);
                    if (result > std::numeric_limits<int64_t>::max ()
                        || result < std::numeric_limits<int64_t>::lowest ())
                      throw std::exception ();
                    row_string.insert (static_cast<int64_t> (result),
                                       offsets[column]);
                  }
                catch (std::exception &error)
                  {
                    throw std::runtime_error ("Invalid 'long' in row "
                                              + std::to_string(current_row + 1)
                                              + ", field "
                                              + std::to_string(column)
                                              + ".  Found '"
                                              + element + "'");
                  }
                break;
              case Type::FLOAT:
                try
                  {
                    float result=boost::lexical_cast<float> (element);
                    row_string.insert (result, offsets[column]);
                  }
                catch (std::exception &error)
                  {
                    throw std::runtime_error ("Invalid 'float' in row "
                                              + std::to_string(current_row + 1)
                                              + ", field "
                                              + std::to_string(column)
                                              + ".  Found '"
                                              + element + "'");
                  }
                break;
              case Type::DOUBLE:
                try
                  {
                    double result=boost::lexical_cast<double> (element);
                    row_string.insert (result, offsets[column]);
                  }
                catch (std::exception &error)
                  {
                    throw std::runtime_error ("Invalid 'double' in row "
                                              + std::to_string(current_row + 1)
                                              + ", field "
                                              + std::to_string(column)
                                              + ".  Found '"
                                              + element + "'");
                  }
                break;
              case Type::STRING:
                row_string.insert (element, offsets[column],
                                   offsets[column+1]);
                break;
              }
        }
      append_row (row_string);
    }
}
