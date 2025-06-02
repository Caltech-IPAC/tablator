#pragma once

#include "Field_Properties.hxx"
#include "data_size.hxx"

namespace tablator {

class Column {
public:
  // JTODO: is column.array_size the actual max, as opposed to the attribute which might be '*'?
    static constexpr char const *COL_ARRAY_SIZE = "array_size";
    static constexpr char const *COL_FIELD_PROPERTIES = "field_properties";
    static constexpr char const *COL_NAME = "name";
    static constexpr char const *COL_TYPE = "type";
    static constexpr char const *COL_DYNAMIC_ARRAY_FLAG = "dynamic_array_flag";

    Column(const std::string &Name, const Data_Type &Type, const size_t &Array_size)
            : Column(Name, Type, Array_size, Field_Properties(),
#if 0
                     Type == Data_Type::CHAR /* dynamic_array_flag */
#else
					 false /* JTODO */
#endif
) {}

    Column(const std::string &Name, const Data_Type &Type, const size_t &Array_size,
           bool dynamic_array_flag)
            : Column(Name, Type, Array_size, Field_Properties(), dynamic_array_flag) {}

    Column(const std::string &Name, const Data_Type &Type, const size_t &Array_size,
           const Field_Properties &Field_properties)
            : Column(Name, Type, Array_size, Field_properties,
#if 0
                     Type == Data_Type::CHAR /* dynamic_array_flag */
#else
					 false  // JTODO
#endif
					 ) {}

    Column(const std::string &Name, const Data_Type &Type, const size_t &Array_size,
           const Field_Properties &Field_properties, bool dynamic_array_flag)
            : name_(Name),
              type_(Type),
              array_size_(Array_size),
              field_properties_(Field_properties),
#if 0
              dynamic_array_flag_(dynamic_array_flag || Type == Data_Type::CHAR)
#else
              dynamic_array_flag_(dynamic_array_flag)
#endif
  {}

  inline size_t get_data_size() const {
	//	std::cout << "Column.data_size(), enter, data_size(type_): " << tablator::data_size(type_) << ", array_size: " << array_size_ << std::endl;
	return tablator::data_size(type_) * array_size_;
  }

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
    inline void add_field_property_attribute(const STRING_PAIR &attr_pair) {
        get_field_properties().add_attribute(attr_pair);
    }
    inline void add_field_property_attribute(const std::string &name,
                                             const std::string &value) {
        add_field_property_attribute(std::make_pair(name, value));
    }
    inline void set_field_property_attributes(const ATTRIBUTES &attributes) {
        get_field_properties().set_attributes(attributes);
    }
    inline void add_field_property_attributes(const ATTRIBUTES &attributes) {
        get_field_properties().add_attributes(attributes);
    }
    inline const ATTRIBUTES &get_field_property_attributes() const {
        return get_field_properties().get_attributes();
    }
    inline ATTRIBUTES &get_field_property_attributes() {
        return get_field_properties().get_attributes();
    }
    inline bool get_dynamic_array_flag() const { return dynamic_array_flag_; }
  inline void set_dynamic_array_flag(bool b)  {
	dynamic_array_flag_ = b;
  }

private:
    std::string name_;
    Data_Type type_;
    size_t array_size_;
    Field_Properties field_properties_;
    bool dynamic_array_flag_;
};


// JTODO If we were following IVOA terminology, this class would IMHO
// be called Field.  I'm not ready to rename the class and file
// altogether, though.
typedef Column Field;

}  // namespace tablator
