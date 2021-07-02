#include "../Table.hxx"

std::vector<tablator::STRING_PAIR> tablator::Table::flatten_properties(
        const std::vector<Labeled_Property> &label_and_property_list) {
    std::vector<tablator::STRING_PAIR> result;
    for (auto &label_and_prop : label_and_property_list) {
        std::vector<tablator::STRING_PAIR> flattened_prop(
                label_and_prop.second.flatten(label_and_prop.first));
        result.insert(result.end(), flattened_prop.begin(), flattened_prop.end());
    }
    return result;
}
