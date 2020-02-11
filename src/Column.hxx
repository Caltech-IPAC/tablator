#pragma once

#include "Field_Properties.hxx"
#include "data_size.hxx"

namespace tablator {
class Column {
public:
    std::string name;
    Data_Type type;
    size_t array_size;
    Field_Properties field_properties;

    Column(const std::string &Name, const Data_Type &Type, const size_t &Array_size)
            : Column(Name, Type, Array_size, Field_Properties()) {}

    Column(const std::string &Name, const Data_Type &Type, const size_t &Array_size,
           const Field_Properties &Field_properties)
            : name(Name),
              type(Type),
              array_size(Array_size),
              field_properties(Field_properties) {}

    size_t data_size() const { return tablator::data_size(type) * array_size; }

    // accessors
    inline const std::string &get_name() const { return name; }
    inline const Data_Type &get_type() const { return type; }
    inline size_t get_array_size() const { return array_size; }
    inline const Field_Properties &get_field_properties() const {
        return field_properties;
    }
    inline Field_Properties &get_field_properties() { return field_properties; }

    inline void set_name(const std::string &n) { name.assign(n); }
    inline void set_type(const Data_Type t) { type = t; }
    inline void set_array_size(size_t n) { array_size = n; }
    inline void set_field_properties(const Field_Properties &fp) {
        field_properties = fp;
    }
};
}  // namespace tablator
