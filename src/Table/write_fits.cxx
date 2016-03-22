#include <algorithm>
#include <longnam.h>
#include <CCfits/CCfits>

#include "../Table.hxx"
#include "../to_string.hxx"
#include "../fits_keyword_mapping.hxx"

/// We need different data_type and vector_type because vector<bool>
/// has a packed representation.
template <typename data_type, typename vector_type>
void write_column (fitsfile *fits_file, const int &fits_type,
                   const int &column, const char *data,
                   const hsize_t &array_size, const size_t &num_rows,
                   const size_t &row_size)
{
  std::vector<vector_type> temp_array (num_rows * array_size);
  size_t offset = 0;
  for (size_t j = 0; j < temp_array.size (); j += array_size)
    {
      for (size_t k = 0; k < array_size; ++k)
        temp_array[j + k]
            = *(reinterpret_cast<const vector_type *>(data + offset) + k);
      offset += row_size;
    }
  int status = 0;
  fits_write_col (fits_file, fits_type, column + 1, 1, 1, temp_array.size (),
                  reinterpret_cast<data_type *>(temp_array.data ()), &status);
  if (status != 0)
    throw CCfits::FitsError (status);
}

template <typename data_type>
void write_column (fitsfile *fits_file, const int &fits_type,
                   const int &column, const char *data,
                   const hsize_t &array_size, const size_t &num_rows,
                   const size_t &row_size)
{
  write_column<data_type, data_type>(fits_file, fits_type, column, data,
                                     array_size, num_rows, row_size);
}

void tablator::Table::write_fits (const boost::filesystem::path &filename)
    const
{
  /// Remove the file because cfitsio will fail if the file still
  /// exists.
  boost::filesystem::remove (filename);
  int status = 0;
  fitsfile *fits_file;
  fits_create_file (&fits_file, filename.c_str (), &status);
  if (status != 0)
    throw CCfits::FitsError (status);

  write_fits (fits_file);
  fits_close_file (fits_file, &status);
  if (status != 0)
    { throw CCfits::FitsError (status); }
}


void tablator::Table::write_fits (std::ostream &os) const
{
  size_t buffer_size (2880);
  void *buffer=malloc(buffer_size);
  try
    {
      fitsfile *fits_file, *reopen_file;
      int status = 0;
      fits_create_memfile (&fits_file, &buffer, &buffer_size, 0, std::realloc,
                           &status);
      if (status != 0)
        { throw CCfits::FitsError (status); }
      write_fits (fits_file);
  
      /// I have to reopen the file because otherwise fits_close_file
      /// will delete the memory
      fits_reopen_file (fits_file, &reopen_file, &status);
      if (status != 0)
        { throw CCfits::FitsError (status); }
        
      fits_close_file (fits_file, &status);
      if (status != 0)
        { throw CCfits::FitsError (status); }

      os.write (static_cast<const char*> (buffer), buffer_size);
      /// This also free's buffer.
      fits_close_file (reopen_file, &status);
      if (status != 0)
        { throw CCfits::FitsError (status); }
    }
  catch (...)
    {
      free (buffer);
      throw;
    }
}


/// We separate out the write_fits implementation so that we can
/// insert a read of the memory image so that we can write to a
/// stream.
void tablator::Table::write_fits (fitsfile *fits_file) const
{
  int status = 0;
  std::vector<string> fits_names;
  std::vector<string> fits_types;

  std::vector<string> fits_units;
  std::vector<const char *> ttype, tunit;
  for (auto &column: columns)
    {
      ttype.push_back (column.name.c_str ());
      std::string n (std::to_string (column.array_size));
      char fits_type;
      switch (column.type)
        {
        case Data_Type::INT8_LE:
          fits_type='L';
          break;
        case Data_Type::UINT8_LE:
          fits_type='B';
          break;
        case Data_Type::INT16_LE:
          fits_type='I';
          break;
        case Data_Type::UINT16_LE:
          fits_type='U';
          break;
        case Data_Type::INT32_LE:
          fits_type='J';
          break;
        case Data_Type::UINT32_LE:
          fits_type='V';
          break;
        case Data_Type::INT64_LE:
        case Data_Type::UINT64_LE:
          /// Fits does not know what an unsigned long is.  So we write it
          /// as a long and hope for the best.
          fits_type='K';
          break;
        case Data_Type::FLOAT32_LE:
          fits_type='E';
          break;
        case Data_Type::FLOAT64_LE:
          fits_type='D';
          break;
        case Data_Type::CHAR:
          fits_type='A';
          break;
        default:
          throw std::runtime_error (
              "In column '" + column.name
              + "': unknown data type when writing fits data: "
              + to_string (column.type));
          break;
        }
      fits_types.push_back (n+fits_type);
      
      auto unit = column.field_properties.attributes.find ("unit");
      if (unit == column.field_properties.attributes.end ())
        {
          tunit.push_back ("");
        }
      else
        {
          tunit.push_back (unit->second.c_str());
        }
    }

  /// We have to store the fits_type in a string and then set the
  /// c_str(), because otherwise the string will get deallocated.
  std::vector<const char *> tform;
  for (auto &t : fits_types)
    tform.push_back (t.c_str ());

  fits_create_tbl (fits_file, BINARY_TBL, 0, ttype.size (),
                   const_cast<char **>(ttype.data ()),
                   const_cast<char **>(tform.data ()),
                   const_cast<char **>(tunit.data ()), "Table", &status);
  if (status != 0)
    throw CCfits::FitsError (status);

  /// Write properties
  fits_write_key_longwarn (fits_file, &status);
  if (status != 0)
    throw CCfits::FitsError (status);
  for (auto &p : properties)
    {
      auto keyword_mapping = fits_keyword_mapping (true);
      std::string name = p.first;
      auto i = keyword_mapping.find (name);
      if (i != keyword_mapping.end ())
        name = i->second;

      std::string comment, value (p.second.value);
      for (auto &a : p.second.attributes)
        {
          if (a.first == "comment")
            comment = a.second;
          else if (a.first != "ucd")
            value += ", " + a.first + ": " + a.second;
        }
      fits_write_key_longstr (fits_file, name.c_str (), value.c_str (),
                              comment.c_str (), &status);
      if (status != 0)
        throw CCfits::FitsError (status);
    }

  for (size_t i = 0; i < columns.size (); ++i)
    {
      const char *offset_data = data.data () + offsets[i];
      switch (columns[i].type)
        {
        case Data_Type::INT8_LE:
          write_column<bool, char>(fits_file, TLOGICAL, i, offset_data,
                                   columns[i].array_size, num_rows (),
                                   row_size ());
          break;
        case Data_Type::UINT8_LE:
          write_column<uint8_t>(fits_file, TBYTE, i, offset_data,
                                columns[i].array_size, num_rows (),
                                row_size ());
          break;
        case Data_Type::INT16_LE:
          write_column<int16_t>(fits_file, TSHORT, i, offset_data,
                                columns[i].array_size, num_rows (),
                                row_size ());
          break;
        case Data_Type::UINT16_LE:
          write_column<uint16_t>(fits_file, TUSHORT, i, offset_data,
                                 columns[i].array_size, num_rows (),
                                 row_size ());
          break;
        case Data_Type::INT32_LE:
          write_column<int32_t>(fits_file, TINT, i, offset_data,
                                columns[i].array_size, num_rows (),
                                row_size ());
          break;
        case Data_Type::UINT32_LE:
          write_column<uint32_t>(fits_file, TUINT, i, offset_data,
                                 columns[i].array_size, num_rows (),
                                 row_size ());
          break;
        case Data_Type::INT64_LE:
        case Data_Type::UINT64_LE:
          /// Fits does not know what an unsigned long is.  So we write it
          /// as a long and hope for the best.
          write_column<int64_t>(fits_file, TLONGLONG, i, offset_data,
                                columns[i].array_size, num_rows (),
                                row_size ());
          break;
        case Data_Type::FLOAT32_LE:
          write_column<float>(fits_file, TFLOAT, i, offset_data,
                              columns[i].array_size, num_rows (),
                              row_size ());
          break;
        case Data_Type::FLOAT64_LE:
          write_column<double>(fits_file, TDOUBLE, i, offset_data,
                               columns[i].array_size, num_rows (),
                               row_size ());
          break;
        case Data_Type::CHAR:
          {
            // FIXME: This adds a space ' ' if the string is empty,
            // breaking the null_bitfield_flags column.
            std::vector<std::string> temp_strings (num_rows ());
            std::vector<char *> temp_chars (num_rows ());
            for (size_t j = 0; j < temp_strings.size (); ++j)
              {
                temp_strings[j]
                  = std::string (offset_data, offsets[i + 1] - offsets[i]);
                temp_chars[j] = const_cast<char *>(temp_strings[j].c_str ());
                offset_data += row_size ();
              }
            fits_write_col (fits_file, TSTRING, i + 1, 1, 1, num_rows (),
                            temp_chars.data (), &status);
            if (status != 0)
              throw CCfits::FitsError (status);
          }
          break;
        default:
          throw std::runtime_error (
            "Unknown data type when writing fits data: "
            + to_string (columns[i].type));
          break;
        }
    }
}

