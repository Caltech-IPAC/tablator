#pragma once

#include "Format.hxx"
#include "Data_Type_to_Oracle.hxx"
#include "Data_Type_to_Postgres.hxx"
#include "Data_Type_to_SQLite.hxx"

namespace tablator
{
inline std::string Data_Type_to_SQL (const Data_Type &data_type,
                                     const size_t &array_size,
                                     const Format::Enums &output_type)
{
  switch (output_type)
    {
    case Format::Enums::ORACLE_SQL:
      return Data_Type_to_Oracle (data_type, array_size);
    case Format::Enums::POSTGRES_SQL:
      return Data_Type_to_Postgres (data_type, array_size);
    case Format::Enums::SQLITE_SQL:
      return Data_Type_to_SQLite (data_type);
    default:
      throw std::runtime_error (
          "Unknown Database_Type"
          + std::to_string (static_cast<int>(output_type)));
    }
}
}
