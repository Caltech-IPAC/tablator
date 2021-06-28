#include "../Table.hxx"

std::vector<std::pair<std::string, std::string>> tablator::Table::flatten_properties(
        const std::vector<Labeled_Property> &label_and_property_list) {
    std::vector<std::pair<std::string, std::string>> result;
    for (auto &label_and_prop : label_and_property_list) {
        std::vector<std::pair<std::string, std::string>> flattened_prop(
                label_and_prop.second.flatten(label_and_prop.first));
        result.insert(result.end(), flattened_prop.begin(), flattened_prop.end());
    }
    return result;
}
