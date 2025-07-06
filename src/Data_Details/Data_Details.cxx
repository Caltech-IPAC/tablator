#include "../Data_Details.hxx"

#include "../Field_Framework.hxx" 

namespace tablator {

  Data_Details::Data_Details(const Field_Framework &field_framework, size_t num_rows)
            : Data_Details(field_framework.get_row_size(), num_rows) {}

}
