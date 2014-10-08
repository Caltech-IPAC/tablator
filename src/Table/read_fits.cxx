#include <CCfits/CCfits>

#include "../Table.hxx"

namespace
{
  template <typename T>
  void read_column(char *position, std::pair<const std::string,
                                             CCfits::Column *> &c,
                   const size_t &rows)
  {
    std::vector<T> v;
    c.second->read(v,1,rows);
    for(auto &element: v)
      *reinterpret_cast<T*>(position)=element;
  }
}


void Tablator::Table::read_fits(const std::string &input_file)
{
  CCfits::FITS fits_file (input_file, CCfits::Read, false);
  CCfits::ExtHDU &table_extension = fits_file.extension (1);
  CCfits::BinTable *table (dynamic_cast<CCfits::BinTable *>(&table_extension));

  row_size=0;
  for(auto &c: table->column ())
    {
      switch(c.second->type())
        {
        case CCfits::Tlogical:
          row_size+=H5::PredType::NATIVE_UCHAR.getSize();
          break;
        case CCfits::Tstring:
          row_size+=H5::PredType::NATIVE_CHAR.getSize()*c.second->width();
          break;
        case CCfits::Tushort:
        case CCfits::Tshort:
          row_size+=H5::PredType::NATIVE_INT16.getSize();
          break;
        case CCfits::Tuint:
        case CCfits::Tint:
          row_size+=H5::PredType::NATIVE_INT32.getSize();
          break;
        case CCfits::Tulong:
        case CCfits::Tlong:
          row_size+=H5::PredType::NATIVE_INT64.getSize();
          break;
        case CCfits::Tfloat:
          row_size+=H5::PredType::NATIVE_FLOAT.getSize();
          break;
        case CCfits::Tdouble:
          row_size+=H5::PredType::NATIVE_DOUBLE.getSize();
          break;
        default:
          throw std::runtime_error("Unsupported data type in the fits file for "
                                   "column " + c.first);
        }
    }
  compound_type=H5::CompType(row_size);

  // FIXME: This is going to break if we have more than 2^32 rows
  data.resize(table->rows()*row_size);

  size_t offset{ 0 };
  for(auto &c: table->column ())
    {
      std::cout << "offset: " << offset << " "
                << c.first << " "
                << "\n";
      std::cout.flush();

      char *current_row=data.data();
      switch (c.second->type())
        {
        case CCfits::Tlogical:
          std::cout << "Tlogical\n";
          std::cout.flush();

          compound_type.insertMember(c.first,offset,H5::PredType::NATIVE_UCHAR);
          types.push_back(Type::BOOLEAN);
          {
            std::vector<int> v;
            c.second->read(v,1,table->rows());
            for(auto &element: v)
              current_row[offset]=element;
          }
          break;
        case CCfits::Tstring:
          std::cout << "Tstring\n";
          std::cout.flush();
          string_types.emplace_back(H5::PredType::NATIVE_CHAR,c.second->width());
          compound_type.insertMember(c.first,offset,*string_types.rbegin());
          types.push_back(Type::STRING);
          {
            std::vector<std::string> v;
            c.second->read(v,1,table->rows());
            for(auto &element: v)
              for(int i=0; i<c.second->width(); ++i)
                current_row[offset+i]=element[i];
          }
          break;
        case CCfits::Tushort:
        case CCfits::Tshort:
          std::cout << "Tshort\n";
          std::cout.flush();
          compound_type.insertMember(c.first,offset,H5::PredType::NATIVE_INT16);
          types.push_back(Type::SHORT);
          read_column<int16_t>(current_row+offset,c,table->rows());
          break;
        case CCfits::Tuint:
        case CCfits::Tint:
          std::cout << "Tint\n";
          std::cout.flush();
          compound_type.insertMember(c.first,offset,H5::PredType::NATIVE_INT32);
          types.push_back(Type::INT);
          read_column<int32_t>(current_row+offset,c,table->rows());
          break;
        case CCfits::Tulong:
        case CCfits::Tlong:
          std::cout << "Tlong\n";
          std::cout.flush();
          compound_type.insertMember(c.first,offset,H5::PredType::NATIVE_INT64);
          types.push_back(Type::LONG);
          read_column<int64_t>(current_row+offset,c,table->rows());
          break;
        case CCfits::Tfloat:
          std::cout << "Tfloat\n";
          std::cout.flush();
          compound_type.insertMember(c.first,offset,H5::PredType::NATIVE_FLOAT);
          types.push_back(Type::FLOAT);
          read_column<float>(current_row+offset,c,table->rows());
          break;
        case CCfits::Tdouble:
          std::cout << "Tdouble\n";
          std::cout.flush();
          compound_type.insertMember(c.first,offset,
                                     H5::PredType::NATIVE_DOUBLE);
          types.push_back(Type::DOUBLE);
          read_column<double>(current_row+offset,c,table->rows());
          break;
        default:
          throw std::runtime_error("Unsupported data type in the fits file for "
                                   "column " + c.first);
        }
      // FIXME: This should get the comment, but the comment()
      // function is protected???
      fields_properties.push_back(Field_Properties
                                  (std::string(""),
                                   {{ "unit", c.second->unit() }}));
      offsets.push_back(offset);
      offset += compound_type.getMemberDataType(compound_type.getNmembers()-1)
        .getSize();
        
      current_row+=row_size;
    }
  offsets.push_back(offset);
}

