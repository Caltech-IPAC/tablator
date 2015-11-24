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
  row_size = 0;
  offsets.push_back (0);
  for (size_t column = 0; column < table->column ().size (); ++column)
    {
      /// CCfits is 1 based, not 0 based.
      CCfits::Column &c = table->column (column+1);
      switch (c.type ())
        {
        case CCfits::Tlogical:
          append_member (c.name (), H5::PredType::STD_I8LE);
          types.push_back (H5::PredType::STD_I8LE);
          break;
        case CCfits::Tbyte:
          append_member (c.name (), H5::PredType::STD_U8LE);
          types.push_back (H5::PredType::STD_U8LE);
          break;
        case CCfits::Tshort:
          types.push_back (H5::PredType::STD_I16LE);
          append_member (c.name (), H5::PredType::STD_I16LE);
          break;
        case CCfits::Tushort:
          types.push_back (H5::PredType::STD_U16LE);
          append_member (c.name (), H5::PredType::STD_U16LE);
          break;
        case CCfits::Tint:
          types.push_back (H5::PredType::STD_I32LE);
          append_member (c.name (), H5::PredType::STD_I32LE);
          break;
        case CCfits::Tuint:
          types.push_back (H5::PredType::STD_U32LE);
          append_member (c.name (), H5::PredType::STD_U32LE);
          break;
        case CCfits::Tlong:
          /// Tlong and Tulong have indeterminate sizes.  We guess 32 bit.
          types.push_back (H5::PredType::STD_I32LE);
          append_member (c.name (), H5::PredType::STD_I32LE);
          break;
        case CCfits::Tulong:
          types.push_back (H5::PredType::STD_U32LE);
          append_member (c.name (), H5::PredType::STD_U32LE);
          break;
        case CCfits::Tlonglong:
          types.push_back (H5::PredType::STD_I64LE);
          append_member (c.name (), H5::PredType::STD_I64LE);
          break;
        case CCfits::Tfloat:
          types.push_back (H5::PredType::IEEE_F32LE);
          append_member (c.name (), H5::PredType::IEEE_F32LE);
          break;
        case CCfits::Tdouble:
          types.push_back (H5::PredType::IEEE_F64LE);
          append_member (c.name (), H5::PredType::IEEE_F64LE);
          break;
        case CCfits::Tstring:
          string_types.emplace_back (0, c.width ());
          append_member (c.name (), *string_types.rbegin ());
          types.push_back (H5::PredType::C_S1);
          break;
        default:
          throw std::runtime_error (
              "Unsupported data type in the fits file for "
              "column '" + c.name ()
              + "'" + std::to_string (static_cast<int>(c.type ())) + " blarg");
        }
    }

  // FIXME: table->rows () returns an int, so this is going to break
  // if we have more than 2^32 rows
  data.resize (table->rows () * row_size);

  for (size_t column = 0; column < table->column ().size (); ++column)
    {
      /// CCfits is 1 based, not 0 based.
      CCfits::Column &c = table->column (column+1);

      switch (c.type ())
        {
        case CCfits::Tlogical:
          {
            std::vector<int> v;
            c.read (v, 1, table->rows ());
            size_t offset = offsets[column];
            for (auto &element : v)
              {
                data[offset] = element;
                offset += row_size;
              }
          }
          break;
        case CCfits::Tbyte:
          read_column<uint8_t>(data.data () + offsets[column], c,
                               table->rows (), row_size);
          break;
        case CCfits::Tshort:
          read_column<int16_t>(data.data () + offsets[column], c,
                               table->rows (), row_size);
          break;
        case CCfits::Tushort:
          read_column<uint16_t>(data.data () + offsets[column], c,
                                table->rows (), row_size);
          break;
        case CCfits::Tuint:
        case CCfits::Tulong:
          read_column<uint32_t>(data.data () + offsets[column], c,
                                table->rows (), row_size);
          break;
        case CCfits::Tint:
        case CCfits::Tlong:
          read_column<int32_t>(data.data () + offsets[column], c,
                               table->rows (), row_size);
          break;
        case CCfits::Tlonglong:
          read_column<int64_t>(data.data () + offsets[column], c,
                               table->rows (), row_size);
          break;
        case CCfits::Tfloat:
          read_column<float>(data.data () + offsets[column], c,
                             table->rows (), row_size);
          break;
        case CCfits::Tdouble:
          read_column<double>(data.data () + offsets[column], c,
                              table->rows (), row_size);
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
                offset += row_size;
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
      if (c.unit ().empty())
        fields_properties.push_back (Field_Properties (std::string (""), { }));
      else
        fields_properties.push_back (Field_Properties
                                     (std::string (""),
                                      { { "unit", c.unit () } }));
    }
}
