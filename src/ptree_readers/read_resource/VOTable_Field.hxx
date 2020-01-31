#pragma once

#include "../../Column.hxx"

namespace tablator {
/// This is almost the same as Column
class VOTable_Field : public Column {
public:
    VOTable_Field() : Column("", Data_Type::UINT8_LE, 1) {}
    VOTable_Field(const std::string &Name, const Data_Type &Type,
                  const bool &Array_size, const Field_Properties &Properties)
            : Column(Name, Type, Array_size, Properties) {}

    bool get_is_array_dynamic() const { return is_array_dynamic_; }
    void set_is_array_dynamic(bool b) { is_array_dynamic_ = b; }

private:
    bool is_array_dynamic_ = false;
};
}  // namespace tablator
