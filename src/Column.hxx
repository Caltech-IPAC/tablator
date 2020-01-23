#pragma once

#include "Field_Properties.hxx"
#include "data_size.hxx"

namespace tablator {
class Column {
public:
    Column(const std::string &Name, const Data_Type &Type, const size_t &Array_size)
            : Column(Name, Type, Array_size, Field_Properties()) {}

    Column(const std::string &Name, const Data_Type &Type, const size_t &Array_size,
           const Field_Properties &Field_properties)
            : name_(Name),
              type_(Type),
              array_size_(Array_size),
              field_properties_(Field_properties) {}

    inline size_t data_size() const { return tablator::data_size(type_) * array_size_; }

    // accessors
    inline const std::string &get_name() const { return name_; }
    inline const Data_Type &get_type() const { return type_; }
    inline size_t get_array_size() const { return array_size_; }
    inline const Field_Properties &get_field_properties() const {
        return field_properties_;
    }
    inline Field_Properties &get_field_properties() { return field_properties_; }

    inline void set_name(const std::string &name) { name_.assign(name); }
    inline void set_type(const Data_Type type) { type_ = type; }
    inline void set_array_size(size_t n) { array_size_ = n; }
    inline void set_field_properties(const Field_Properties &fp) {
        field_properties_ = fp;
    }

private:
    std::string name_;
    Data_Type type_;
    size_t array_size_;
    Field_Properties field_properties_;
};
}  // namespace tablator
