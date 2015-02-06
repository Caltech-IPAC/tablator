#include <errno.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cctype>

#include "read_ipac_table.hxx"

void Tablator::Table::read_ipac_table (const boost::filesystem::path &path)
{
    size_t ncols=0,file_size=0,ipac_row_size{0},ipac_row_number{0},ipac_data_offset=0;
    file_size = boost::filesystem::file_size(path);
    if (!exists(path))
        throw std::runtime_error("File " + path.string() + "does not exist.") ;

    boost::filesystem::ifstream ifs(path, std::ios::in);
    std::vector <std::string> keys, names, dtypes, units;
    std::vector <int> ipac_column_offset, ipac_row_offset;
    std::string str,key,val;

    bool fixlen_table = false, fixlen_key = false;
    int  header=0;

    while (std::getline(ifs, str)) 
      {
        ipac_data_offset += str.size()+1;
        if (str.find("\\ ") == 0)
            comment.push_back(str.erase(0,2));

        else if (str.find("\\") == 0)
          {
            int position_of_equal = str.find("=");
            key = boost::algorithm::trim_copy(str.substr(1,position_of_equal-1));
            val = boost::algorithm::trim_copy(str.substr(position_of_equal+1));

            if (boost::iequals(key,"fixlen"))
              {
                if (boost::iequals(val,"t")) 
                  {
                    fixlen_key   = true;
                    fixlen_table = true;
                  } 
              }
            else
              {
                std::size_t first = val.find_first_not_of("'");
                std::size_t last  = val.find_last_not_of("'");
                Property p(val.substr(first, last-first+1));
                properties.insert(std::make_pair(key,p)); 
              }
          }
 
        else if (str.find("|") == 0)
	  { 
            if (str.find("\t") != std::string::npos) 
                throw std::runtime_error( "Header '" + str +"' contains tabs.");
 
            header = (header < 4) ? header+1 
              : throw std::runtime_error("More than 4 headerlines started with '|' are found.");
    
            fixlen_table = fixlen_table & ((header == 1) ? 1 : ((ipac_row_size == str.size()) ? 1 : 0));
            if (header == 1) 
              {
                ipac_row_size = str.size();
                ipac_column_offset = find_ipac_column_offset(str,'|');
                boost::split(names,str.erase(str.find_last_of("|"),1).erase(0,1),boost::is_any_of("|"));
                validate_ipac_header (names, name_regex); 
              }
            else if (header == 2) 
              {
                if (!check_bar_position(ipac_column_offset, str))  
                    throw std::runtime_error( "Mis-locate the bar(|) in the datatype header.");
                boost::split(dtypes,str.erase(str.find_last_of("|"),1).erase(0,1),boost::is_any_of("|"));
                validate_ipac_header (dtypes, type_regex);
              }
            else if (header == 3) 
              {
                if (!check_bar_position(ipac_column_offset, str))  
                    throw std::runtime_error( "Mis-locate the bar(|) in the unit header.");
                boost::split(units,str.erase(str.find_last_of("|"),1).erase(0,1),boost::is_any_of("|"));
              }
            else if (header == 4) 
              {
                if (!check_bar_position(ipac_column_offset, str))  
                    throw std::runtime_error( "Mis-locate the bar(|) in the null header.");
                boost::split(nulls,str.erase(str.find_last_of("|"),1).erase(0,1),boost::is_any_of("|"));
                for (auto &n: nulls)
                    n = boost::algorithm::trim_copy(n);       
              }
          }
        else
          {
            ipac_data_offset -= str.size()+1;
            ++ ipac_row_size;
            break; 
          }
      }
      if (header < 2)
          throw std::runtime_error( "Missing column datatype.");

      ncols=names.size();
      fixlen_table = fixlen_table 
                   & (ncols == dtypes.size()) 
                   & (ncols == units.size()) 
                   & (ncols == nulls.size());

      ipac_column_widths = get_ipac_column_widths(ipac_column_offset);
      if (fixlen_key)
        {
          if (!fixlen_table) 
              throw std::runtime_error( "Not a fixlen table althogh 'fixlen = T' is defined.");
          if ((file_size - ipac_data_offset) % (ipac_row_size) > 0)
              throw std::runtime_error( "Bad formatted fixlen IPAC table.");
          ipac_row_number = (file_size - ipac_data_offset+1) / ipac_row_size;
          for (size_t row=0; row< ipac_row_number; ++row)
              ipac_row_offset.push_back(0);
        }
      else
        {
          ifs.seekg(ipac_data_offset, std::ios::beg);
          while (std::getline(ifs, str)) 
            {
              if (str.size() <= static_cast<size_t>(ipac_column_offset[ncols-1]+2))
                  throw std::runtime_error( "Bad formatted IPAC table.");
              ipac_row_offset.push_back(str.size()-ipac_column_offset[ncols]-1);
              ++ipac_row_number; 
            }
            ifs.clear();
        }
      types = assign_data_type(dtypes);
      row_size = 0;
      for (size_t i=0; i< ncols; ++i) 
        {
          offsets.push_back(row_size);
          switch(types[i])
            {
              case Type::DOUBLE:
              case Type::FLOAT:
                   row_size+=H5::PredType::NATIVE_DOUBLE.getSize();
              break;

              case Type::STRING:
                   string_types.emplace_back(H5::PredType::NATIVE_CHAR,ipac_column_widths[i]);
                   row_size+=H5::PredType::NATIVE_CHAR.getSize()*(ipac_column_widths[i]);
              break;

              case Type::INT:
              case Type::BOOLEAN:
              case Type::SHORT:
                   row_size+=H5::PredType::NATIVE_INT32.getSize();
              break;

              case Type::LONG:
                   row_size+=H5::PredType::NATIVE_INT64.getSize();
              break;
         
              default:
                   throw std::runtime_error("Unsupported data type for column " + names[i]);
            }
        }
      offsets.push_back(row_size);

      compound_type=H5::CompType(row_size);
      data.resize(ipac_row_number*row_size);
      auto string_type_pointer=string_types.begin();
      for (size_t i=0; i< ncols; ++i) 
        {
          switch(types[i])
            {
              case Type::DOUBLE:
              case Type::FLOAT:
                   compound_type.insertMember(names[i],offsets[i], H5::PredType::NATIVE_DOUBLE);
              break;

              case Type::STRING:
                   compound_type.insertMember(names[i],offsets[i],*string_type_pointer);
                   ++string_type_pointer;
              break;

              case Type::INT:
              case Type::BOOLEAN:
              case Type::SHORT:
                   compound_type.insertMember(names[i],offsets[i],H5::PredType::NATIVE_INT32);
              break;

              case Type::LONG:
                   compound_type.insertMember(names[i],offsets[i],H5::PredType::NATIVE_INT64);
              break;
         
              default:
                   throw std::runtime_error("Unsupported data type for column " + names[i]);
            }
        }

      for (int i=0; i< names.size(); ++i)
        {
          if (nulls.size() < i+1) nulls.push_back("");
          if (units.size() < i+1) units.push_back("");
          fields_properties.push_back(Field_Properties (std::string(""), 
              {{ "unit", boost::algorithm::trim_copy(units[i]) }, 
              { "null", boost::algorithm::trim_copy(nulls[i]) }}));
        }
      
      char    line[10240],  *block_ptr, *rd, *end_ptr;
      double  ipac_double;
      int64_t ipac_int64;
      int32_t ipac_int32;

      size_t total_ipac_data_offset = ipac_data_offset;
      for (size_t row = 0; row < ipac_row_number; ++row)
        {
          block_ptr = data.data() + row_size * row; 
          ifs.seekg(total_ipac_data_offset, std::ios::beg);
          for (size_t col = 0; col < ncols; ++col)
            {
              ifs.read(line, ipac_column_widths[col]+1);
              line[ipac_column_widths[col]+1]=0;

              if (line[0] != ' ')
                   throw std::runtime_error("Value is under the '|'");
 
              rd = (block_ptr + offsets[col]);
              switch (types[col])
                {
                  case Type::DOUBLE:
                  case Type::FLOAT:
                       ipac_double = strtod(line, &end_ptr);
                       if (end_ptr == line )
                         {
                           trim(line);
                           if (strcmp(line, (nulls[col]).c_str()) == 0)
                             ipac_double = -9999.999;
                           else
                             throw std::runtime_error("Invalid double value.");
                         }
                        *reinterpret_cast<double *> (rd) = ipac_double;
                  break;

                  case Type::STRING:
                       for (auto i = 0; i< ipac_column_widths[col]; ++i)
                          *(rd + i) = line[i+1];
                  break;

                  case Type::SHORT:
                  case Type::INT:
                       ipac_int32 =  strtol(line,&end_ptr,10);
                       if (end_ptr == line)
                         {
                           trim(line);
                           if (strcmp(line, (nulls[col]).c_str()) == 0)
                              ipac_int32= -9999;
                           else
                              throw std::runtime_error("Invalid long value.");
                         }
                       *reinterpret_cast<int32_t *> (rd) = ipac_int32;
                  break;

                  case Type::LONG:
                       ipac_int64 = strtoll(line, &end_ptr, 10);
                       if (end_ptr == line)
                         {
                           trim(line);
                           if (strcmp(line, (nulls[col]).c_str()) == 0)
                              ipac_int64= -9999;
                           else
                              throw std::runtime_error("Invalid long value.");
                         }
                       *reinterpret_cast<int64_t *> (rd) = ipac_int64;
                  break;
         
                  default:
                  break;
                }
            }
          total_ipac_data_offset  += ipac_row_size + ipac_row_offset[row];
        }

    for (size_t i=0; i<ipac_column_widths.size(); ++i)
      {
        if (types[i] == Type::DOUBLE)
          if (ipac_column_widths[i] < 6)
            ipac_column_widths[i] = 6;
        if (types[i] == Type::INT)
          if (ipac_column_widths[i] < 7)
            ipac_column_widths[i] = 7;
        if (types[i] == Type::LONG)
          if (ipac_column_widths[i] < 4)
            ipac_column_widths[i] = 4;
      }
}
