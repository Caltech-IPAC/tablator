#include <longnam.h>
#include <CCfits/CCfits>

#include "../Table.hxx"
#include "fits_keyword_mapping.hxx"

/// We need different data_type and vector_type because vector<bool>
/// has a packed representation.
template<typename data_type, typename vector_type>
void write_column(fitsfile *fits_file, const int &fits_type,
                  const int &column,
                  const char *data, const size_t &size,
                  const size_t &row_size)
{
  std::vector<vector_type> temp_array(size);
  size_t offset=0;
  for(size_t j=0; j<size; ++j)
    {
      temp_array[j]=*(reinterpret_cast<const vector_type *>(data+offset));
      offset+=row_size;
    }
  int status=0;
  fits_write_col(fits_file,fits_type,column+1,1,1,size,
                 reinterpret_cast<data_type *>(temp_array.data()),
                 &status);
  if(status!=0)
    throw CCfits::FitsError(status);
}

template<typename data_type>
void write_column(fitsfile *fits_file, const int &fits_type,
                  const int &column,
                  const char *data, const size_t &size,
                  const size_t &row_size)
{
  write_column<data_type,data_type>(fits_file,fits_type,column,data,size,
                                    row_size);
}

void Tablator::Table::write_fits (const boost::filesystem::path &filename) const
{
  int status=0;
  fitsfile *fits_file;
  fits_create_file(&fits_file,filename.c_str(), &status);
  if(status!=0)
    throw CCfits::FitsError(status);

  std::vector<string> fits_names;
  std::vector<string> fits_types;

  const size_t num_columns = compound_type.getNmembers ();
  for (size_t i = 0; i < num_columns; ++i)
    {
      fits_names.push_back (compound_type.getMemberName (i));
      switch (types[i])
        {
        case Type::BOOLEAN:
          fits_types.push_back ("L");
          break;
        case Type::SHORT:
          fits_types.push_back ("I");
          break;
        case Type::INT:
          fits_types.push_back ("J");
          break;
        case Type::LONG:
          fits_types.push_back ("K");
          break;
        case Type::FLOAT:
          fits_types.push_back ("E");
          break;
        case Type::DOUBLE:
          fits_types.push_back ("D");
          break;
        case Type::STRING:
          fits_types.push_back (
              std::to_string (compound_type.getMemberDataType (i).getSize ())
              + "A");
          break;
        default:
          throw std::runtime_error
            ("Unknown data type when writing fits data: "
             + std::to_string (static_cast<int>(types[i])));
          break;
        }
    }

  std::vector<string> fits_units;
  for (auto &f : fields_properties)
    {
      auto unit = f.attributes.find ("unit");
      if (unit == f.attributes.end ())
        {
          fits_units.push_back ("");
        }
      else
        {
          fits_units.push_back (unit->second);
        }
    }

  std::vector<const char *> ttype, tform, tunit;
  for(auto &n: fits_names)
    ttype.push_back(n.c_str());

  for(auto &t: fits_types)
    tform.push_back(t.c_str());

  for(auto &u: fits_units)
    tunit.push_back(u.c_str());

  fits_create_tbl(fits_file,BINARY_TBL,0,ttype.size(),
                  const_cast<char **>(ttype.data()),
                  const_cast<char **>(tform.data()),
                  const_cast<char **>(tunit.data()),"Table",&status);
  if(status!=0)
    throw CCfits::FitsError(status);

  /// Write properties
  fits_write_key_longwarn(fits_file,&status);
  if(status!=0)
    throw CCfits::FitsError(status);
  for (auto &p : properties)
    {
      auto keyword_mapping=fits_keyword_mapping(true);
      std::string name=p.first;
      auto i=keyword_mapping.find(name);
      if(i!=keyword_mapping.end())
        name=i->second;

      std::string comment, value(p.second.value);
      for(auto &a: p.second.attributes)
        {
          if(a.first=="comment")
            comment=a.second;
          else if(a.first!="ucd")
            value+=", " + a.first + ": " + a.second;
        }
      fits_write_key_longstr(fits_file,name.c_str(),value.c_str(),
                             comment.c_str(),&status);
      if(status!=0)
        throw CCfits::FitsError(status);
    }

  for (size_t i = 0; i < num_columns; ++i)
    {
      const char *index=data.data () + offsets[i];
      switch (types[i])
        {
        case Type::BOOLEAN:
          write_column<bool,char>(fits_file,TLOGICAL,i,index,size(),row_size);
          break;
        case Type::SHORT:
          write_column<int16_t>(fits_file,TSHORT,i,index,size(),row_size);
          break;
        case Type::INT:
          write_column<int32_t>(fits_file,TINT,i,index,size(),row_size);
          break;
        case Type::LONG:
          write_column<int64_t>(fits_file,TLONG,i,index,size(),row_size);
          break;
        case Type::FLOAT:
          write_column<float>(fits_file,TFLOAT,i,index,size(),row_size);
          break;
        case Type::DOUBLE:
          write_column<double>(fits_file,TDOUBLE,i,index,size(),row_size);
          break;
        case Type::STRING:
          {
            std::vector<std::string> temp_strings(size());
            std::vector<char *> temp_chars(size());
            for(size_t j=0; j<temp_strings.size(); ++j)
              {
                temp_strings[j]=std::string(index,offsets[i + 1] - offsets[i]);
                temp_chars[j]=const_cast<char *>(temp_strings[j].c_str());
                index+=row_size;
              }
            fits_write_col(fits_file,TSTRING,i+1,1,1,size(),temp_chars.data(),
                           &status);
            if(status!=0)
              throw CCfits::FitsError(status);
          }
          break;
        default:
          throw std::runtime_error
            ("Unknown data type when writing fits data: "
             + std::to_string (static_cast<int>(types[i])));
        }
    }
  fits_close_file(fits_file, &status);
  if(status!=0)
    throw CCfits::FitsError(status);
}
