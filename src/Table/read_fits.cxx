#include <CCfits/CCfits>

#include "../Table.hxx"
#include "../fits_keyword_mapping.hxx"

namespace
{
template <typename T>
void read_scalar_column (uint8_t *position, CCfits::Column &c, const size_t &rows,
                         const size_t &row_size)
{
  std::vector<T> v;
  c.read (v, 1, rows);
  uint8_t *current = position;
  for (auto &element : v)
    {
      *reinterpret_cast<T *>(current) = element;
      current += row_size;
    }
}

template <typename T>
void read_vector_column (fitsfile *fits_file, uint8_t *position, CCfits::Column &c,
                         const size_t &rows, const size_t &row_size)
{
  /// Use the C api because the C++ api (Column::readArrays) is
  /// horrendously slow.
  int status (0), anynul (0);
  std::vector<T> temp_array (rows * c.repeat ());
  fits_read_col (fits_file, c.type (), c.index (), 1, 1, c.repeat (), NULL,
                 temp_array.data (), &anynul, &status);

  uint8_t *current = position;
  for (size_t i=0; i < temp_array.size (); i+=c.repeat ())
    {
      uint8_t *element_start=current;
      for (size_t offset=0; offset < c.repeat (); ++offset)
        {
          *reinterpret_cast<T *>(current) = temp_array[i+offset];
          current += sizeof(T);
        }
      current = element_start + row_size;
    }
}

template <typename T>
void read_column (fitsfile *fits_file, uint8_t *position, CCfits::Column &c,
                  const bool &is_array, const size_t &rows,
                  const size_t &row_size)
{
  if (!is_array)
    read_scalar_column<T>(position, c, rows, row_size);
  else
    read_vector_column<T>(fits_file, position, c, rows, row_size);
}
}

// FIXME: This does not copy any keywords
void tablator::Table::read_fits (const boost::filesystem::path &path)
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
      properties.emplace_back (name, p);
    }

  // FIXME: This assumes that the first column is null_bitfield_flags
  // FIXME: This does not handle arrays
  for (size_t column = 0; column < table->column ().size (); ++column)
    {
      /// CCfits is 1 based, not 0 based.
      CCfits::Column &c = table->column (column + 1);
      size_t array_size = 1;
      if (std::isdigit (c.format ().at (0)))
        array_size = std::stoll (c.format ());
      switch (c.type ())
        {
        case CCfits::Tlogical:
          append_column (c.name (), Data_Type::INT8_LE, array_size);
          break;
        case CCfits::Tbyte:
          append_column (c.name (), Data_Type::UINT8_LE, array_size);
          break;
        case CCfits::Tshort:
          append_column (c.name (), Data_Type::INT16_LE, array_size);
          break;
        case CCfits::Tushort:
          append_column (c.name (), Data_Type::UINT16_LE, array_size);
          break;
        case CCfits::Tint:
          append_column (c.name (), Data_Type::INT32_LE, array_size);
          break;
        case CCfits::Tuint:
          append_column (c.name (), Data_Type::UINT32_LE, array_size);
          break;
        case CCfits::Tlong:
          /// Tlong and Tulong have indeterminate sizes.  We guess 32 bit.
          append_column (c.name (), Data_Type::INT32_LE, array_size);
          break;
        case CCfits::Tulong:
          append_column (c.name (), Data_Type::UINT32_LE, array_size);
          break;
        case CCfits::Tlonglong:
          append_column (c.name (), Data_Type::INT64_LE, array_size);
          break;
        case CCfits::Tfloat:
          append_column (c.name (), Data_Type::FLOAT32_LE, array_size);
          break;
        case CCfits::Tdouble:
          append_column (c.name (), Data_Type::FLOAT64_LE, array_size);
          break;
        case CCfits::Tstring:
          append_column (c.name (), Data_Type::CHAR, c.width ());
          break;
        default:
          throw std::runtime_error (
              "Unsupported data type in the fits file for "
              "column '" + c.name () + "'");
        }
    }

  // FIXME: table->rows () returns an int, so this is going to break
  // if we have more than 2^32 rows
  data.resize (table->rows () * row_size ());

  fitsfile *fits_pointer = fits.fitsPointer ();
  for (size_t column = 0; column < table->column ().size (); ++column)
    {
      /// CCfits is 1 based, not 0 based.
      CCfits::Column &c = table->column (column + 1);
      bool is_array (false);
      if (std::isdigit (c.format ().at (0)))
        is_array = (std::stoll (c.format ()) != 1);
      switch (c.type ())
        {
        case CCfits::Tlogical:
          {
            if (!is_array)
              {
                std::vector<int> v;
                c.read (v, 1, table->rows ());
                size_t offset = offsets[column];
                for (auto &element : v)
                  {
                    data[offset] = element;
                    offset += row_size ();
                  }
              }
            else
              {
                // FIXME: Use the C api because Column::readArrays is
                // horrendously slow.
                std::vector<std::valarray<int> > v;
                c.readArrays (v, 1, table->rows ());
                size_t offset = offsets[column];
                for (auto &array : v)
                  {
                    for (auto &element : array)
                      {
                        data[offset] = element;
                        ++offset;
                      }
                    offset += row_size ();
                  }
              }
          }
          break;
        case CCfits::Tbyte:
          read_column<uint8_t>(fits_pointer, data.data () + offsets[column], c,
                               is_array, table->rows (), row_size ());
          break;
        case CCfits::Tshort:
          read_column<int16_t>(fits_pointer, data.data () + offsets[column], c,
                               is_array, table->rows (), row_size ());
          break;
        case CCfits::Tushort:
          read_column<uint16_t>(fits_pointer, data.data () + offsets[column], c,
                                is_array, table->rows (), row_size ());
          break;
        case CCfits::Tuint:
        case CCfits::Tulong:
          read_column<uint32_t>(fits_pointer, data.data () + offsets[column], c,
                                is_array, table->rows (), row_size ());
          break;
        case CCfits::Tint:
        case CCfits::Tlong:
          read_column<int32_t>(fits_pointer, data.data () + offsets[column], c,
                               is_array, table->rows (), row_size ());
          break;
        case CCfits::Tlonglong:
          read_column<int64_t>(fits_pointer, data.data () + offsets[column], c,
                               is_array, table->rows (), row_size ());
          break;
        case CCfits::Tfloat:
          read_column<float>(fits_pointer, data.data () + offsets[column], c,
                             is_array, table->rows (), row_size ());
          break;
        case CCfits::Tdouble:
          read_column<double>(fits_pointer, data.data () + offsets[column], c,
                              is_array, table->rows (), row_size ());
          break;
        case CCfits::Tstring:
          {
            std::vector<std::string> v;
            c.read (v, 1, table->rows ());

            size_t offset = offsets[column];
            for (auto &element : v)
              {
                for (size_t i = 0; i < element.size (); ++i)
                  data[offset + i] = element[i];
                for (int i = element.size (); i < c.width (); ++i)
                  data[offset + i] = '\0';
                offset += row_size ();
              }
          }
          break;
        default:
          throw std::runtime_error (
              "Unsupported data type in the fits file for "
              "column " + c.name ());
        }
      // FIXME: This should get the comment, but the comment()
      // function is protected???
      if (!c.unit ().empty ())
        columns[column].field_properties.attributes = { { "unit", c.unit () } };
    }
}
