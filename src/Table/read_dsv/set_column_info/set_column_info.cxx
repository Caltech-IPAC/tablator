#include "../../../Table.hxx"

namespace tablator {

Data_Type get_best_data_type(const Data_Type &current_type, const std::string &element);


// FIXME: A bit icky.  This modifies the dsv document (trims
// whitespace) while extracting metadata.
void Table::set_column_info(Field_Framework &field_framework,
                            std::list<std::vector<std::string> > &dsv) {
    auto row(dsv.begin());
    std::vector<std::string> names;
    for (auto &name : *row) {
        auto trimmed_name(boost::trim_copy(name));
        if (trimmed_name.find('\t') != std::string::npos) {
            throw std::runtime_error("Invalid character in the name for column '" +
                                     boost::trim_copy(trimmed_name) +
                                     "'.  Tabs are not allowed.");
        }
        names.push_back(trimmed_name);
    }
    append_column(field_framework, null_bitfield_flags_name, Data_Type::UINT8_LE,
                  (names.size() + 7) / 8,
                  Field_Properties(null_bitfield_flags_description));

    // Try to infer the types of the columns.  Supported are INT8_LE
    // (bool), INT64_LE, UINT64_LE, FLOAT64_LE, and CHAR.

    std::vector<Data_Type> types(names.size(), Data_Type::INT8_LE);
    std::vector<size_t> sizes(names.size(), 1);
    std::vector<std::vector<std::string> > strings;

    size_t line_number(1);
    ++row;

    for (; row != dsv.end(); ++row) {
        if (row->size() != names.size()) {
            throw std::runtime_error("In line " + std::to_string(line_number) +
                                     ", expected " + std::to_string(names.size()) +
                                     " elements, but found " +
                                     std::to_string(row->size()));
        }

        for (size_t elem = 0; elem < row->size(); ++elem) {
            std::string &element((*row)[elem]);
            boost::algorithm::trim(element);
            types[elem] = get_best_data_type(types[elem], element);
            sizes[elem] = std::max(sizes[elem], element.size());
        }
    }

    for (size_t elem = 0; elem < names.size(); ++elem) {
        append_column(field_framework, names[elem], types[elem],
                      types[elem] == Data_Type::CHAR ? sizes[elem] : 1);
    }
}

}  // namespace tablator
