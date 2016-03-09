#pragma once

class HDF5_Attribute
{
public:
  const char *name;
  const char *value;

  HDF5_Attribute (const char *Name, const char *Value)
      : name (Name), value (Value)
  {
  }
};
