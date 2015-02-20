#include <CCfits/CCfits>

#include "../Table.hxx"
#include "../fits_keyword_mapping.hxx"

namespace
{
template <typename T>
void read_column (char *position, CCfits::Column &c, const size_t &rows,
                  const size_t &row_size)
{
  std::vector<T> v;
  c.read (v, 1, rows);
  char *current = position;
  for (auto &element : v)
    {
      *reinterpret_cast<T *>(current) = element;
      current += row_size;
    }
}
}

// FIXME: This does not copy any keywords
void Tablator::Table::read_fits (const boost::filesystem::path &path)
{
  CCfits::FITS fits (path.string (), CCfits::Read, false);
  if (fits.extension ().empty ())
    throw std::runtime_error ("Could not find any extensions in this file: "
                              + path.string ());
  CCfits::ExtHDU &table_extension = *(fits.extension ().begin ()->second);
  CCfits::BinTable *table (dynamic_cast<CCfits::BinTable *>(&table_extension));

  std::vector<std::string> fits_ignored_keywords{ { "LONGSTRN" } };

  auto keyword_mapping = fits_keyword_mapping (false);
  table_extension.readAllKeys ();
  for (auto &k : table_extension.keyWord ())
    {
      std::string name (k.first), value;
      if (std::find (fits_ignored_keywords.begin (),
                     fits_ignored_keywords.end (), name)
          != fits_ignored_keywords.end ())
        continue;

      /// Annoyingly, CCfits does not have a way to just return the
      /// value.  You have to give it something to put it in.
      Property p (k.second->value (value));
      auto i = keyword_mapping.find (name);
      if (i != keyword_mapping.end ())
        {
          name = i->second;
          p.attributes.insert (std::make_pair ("ucd", name));
        }
      if (!k.second->comment ().empty ())
        p.attributes.insert (std::make_pair ("comment", k.second->comment ()));
      properties.insert (std::make_pair (name, p));
    }

  row_size = 0;
  for (size_t column = 1; column <= table->column ().size (); ++column)
    {
      CCfits::Column &c = table->column (column);
      switch (c.type ())
        {
        case CCfits::Tlogical:
          row_size += H5::PredType::NATIVE_UCHAR.getSize ();
          break;
        case CCfits::Tstring:
          row_size += H5::PredType::NATIVE_CHAR.getSize () * c.width ();
          nulls.push_back ("null");
          break;
        case CCfits::Tushort:
        case CCfits::Tshort:
          row_size += H5::PredType::NATIVE_INT16.getSize ();
          break;
        case CCfits::Tuint:
        case CCfits::Tint:
          row_size += H5::PredType::NATIVE_INT32.getSize ();
          break;
        case CCfits::Tulong:
        case CCfits::Tlong:
          row_size += H5::PredType::NATIVE_INT64.getSize ();
          break;
        case CCfits::Tfloat:
          row_size += H5::PredType::NATIVE_FLOAT.getSize ();
          break;
        case CCfits::Tdouble:
          row_size += H5::PredType::NATIVE_DOUBLE.getSize ();
          break;
        default:
          throw std::runtime_error (
              "Unsupported data type in the fits file for "
              "column " + c.name ());
        }
      if (c.type () != CCfits::Tstring)
        nulls.push_back ("nan");
    }
  compound_type = H5::CompType (row_size);

  // FIXME: This is going to break if we have more than 2^32 rows
  data.resize (table->rows () * row_size);

  size_t offset{ 0 };
  for (size_t column = 1; column <= table->column ().size (); ++column)
    {
      CCfits::Column &c = table->column (column);

      switch (c.type ())
        {
        case CCfits::Tlogical:
          compound_type.insertMember (c.name (), offset,
                                      H5::PredType::NATIVE_UCHAR);
          types.push_back (Type::BOOLEAN);
          {
            std::vector<int> v;
            c.read (v, 1, table->rows ());
            size_t total_offset = offset;
            for (auto &element : v)
              {
                data[total_offset] = element;
                total_offset += row_size;
              }
          }
          break;
        case CCfits::Tstring:
          string_types.emplace_back (H5::PredType::NATIVE_CHAR, c.width ());
          compound_type.insertMember (c.name (), offset,
                                      *string_types.rbegin ());
          types.push_back (Type::STRING);
          {
            std::vector<std::string> v;
            c.read (v, 1, table->rows ());

            size_t total_offset = offset;
            for (auto &element : v)
              {
                for (size_t i = 0; i < element.size (); ++i)
                  data[total_offset + i] = element[i];
                for (int i = element.size (); i < c.width (); ++i)
                  data[total_offset + i] = ' ';
                total_offset += row_size;
              }
          }
          break;
        case CCfits::Tushort:
        case CCfits::Tshort:
          compound_type.insertMember (c.name (), offset,
                                      H5::PredType::NATIVE_INT16);
          types.push_back (Type::SHORT);
          read_column<int16_t>(data.data () + offset, c, table->rows (),
                               row_size);
          break;
        case CCfits::Tuint:
        case CCfits::Tint:
          compound_type.insertMember (c.name (), offset,
                                      H5::PredType::NATIVE_INT32);
          types.push_back (Type::INT);
          read_column<int32_t>(data.data () + offset, c, table->rows (),
                               row_size);
          break;
        case CCfits::Tulong:
        case CCfits::Tlong:
          compound_type.insertMember (c.name (), offset,
                                      H5::PredType::NATIVE_INT64);
          types.push_back (Type::LONG);
          read_column<int64_t>(data.data () + offset, c, table->rows (),
                               row_size);
          break;
        case CCfits::Tfloat:
          compound_type.insertMember (c.name (), offset,
                                      H5::PredType::NATIVE_FLOAT);
          types.push_back (Type::FLOAT);
          read_column<float>(data.data () + offset, c, table->rows (),
                             row_size);
          break;
        case CCfits::Tdouble:
          compound_type.insertMember (c.name (), offset,
                                      H5::PredType::NATIVE_DOUBLE);
          types.push_back (Type::DOUBLE);
          read_column<double>(data.data () + offset, c, table->rows (),
                              row_size);
          break;
        default:
          throw std::runtime_error (
              "Unsupported data type in the fits file for "
              "column " + c.name ());
        }
      // FIXME: This should get the comment, but the comment()
      // function is protected???
      fields_properties.push_back (Field_Properties (
          std::string (""),
          { { "unit", c.unit () }, { "null", nulls[column - 1] } }));
      offsets.push_back (offset);
      offset += compound_type.getMemberDataType (compound_type.getNmembers ()
                                                 - 1).getSize ();
    }
  offsets.push_back (offset);
}
