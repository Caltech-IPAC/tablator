#pragma once

#include <boost/algorithm/string.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>

#include "Column.hxx"
#include "Common.hxx"
#include "Group_Element.hxx"
#include "Property.hxx"
#include "Table_Element.hxx"

namespace tablator {

static constexpr size_t MAIN_TABLE_ELEMENT_IDX = 0;
static constexpr const char* RESULTS("results");
static constexpr const char* META("meta");
static constexpr const char* OTHER("other");
static constexpr const char* NONE("none");

enum class Resource_Type {
    META,
    RESULTS,
    OTHER,
    NONE,
};

namespace {

Resource_Type get_type_enum(const tablator::ATTRIBUTES &attributes) {
    std::string type_str(NONE);
    for (const auto &attr_pair : attributes) {
        if (boost::iequals(attr_pair.first, tablator::TYPE)) {
            type_str.assign(attr_pair.second);
            break;
        }
    }

    if (boost::equals(type_str, RESULTS)) {
        return Resource_Type::RESULTS;
    }
    if (boost::equals(type_str, META)) {
        return Resource_Type::META;
    }
    if (boost::equals(type_str, NONE)) {
        return Resource_Type::NONE;
    }
    return Resource_Type::OTHER;
}

std::string get_type_string(Resource_Type type) {
    switch (type) {
        case Resource_Type::RESULTS:
            return RESULTS;
        case Resource_Type::META:
            return META;
        case Resource_Type::NONE:
            return NONE;
        default:
            return OTHER;
    }
    return OTHER;
}

inline tablator::Resource_Type determine_resource_type(
        const std::vector<Table_Element> &table_elements,
        const tablator::ATTRIBUTES &attributes) {
    bool has_results_table_element = !table_elements.empty();
    tablator::Resource_Type attr_rtype = get_type_enum(attributes);

    if (!has_results_table_element && (attr_rtype == Resource_Type::RESULTS)) {
        throw std::runtime_error(
                "Resource_Element has type attribute 'results' but does not contain "
                "results.");
    }

    if (has_results_table_element && (attr_rtype != Resource_Type::RESULTS) &&
        (attr_rtype != Resource_Type::NONE)) {
        std::string msg(
                "Resource_Element contains results but has type attribute other than "
                "'results': ");
        msg.append(get_type_string(attr_rtype));
        throw std::runtime_error(msg);
    }

    if (!has_results_table_element) {
        return attr_rtype;
    }

    return Resource_Type::RESULTS;
}

}  // namespace


class Resource_Element {
    // JTODO check for required attributes.

private:
    struct Options {
        ATTRIBUTES attributes_;
        std::string description_;
        std::vector<Field> params_;
        std::vector<std::pair<std::string, Property>> labeled_properties_;
        std::vector<Group_Element> group_elements_;
        std::vector<Table_Element> table_elements_;
        std::vector<Property> trailing_info_list_;

        void set_attributes(
                const std::initializer_list<std::pair<const std::string, std::string>>
                        attributes) {
            attributes_ = attributes;
        }

        void add_attributes(const ATTRIBUTES &attributes) {
            attributes_.insert(attributes.begin(), attributes.end());
        }

        void add_attribute(const std::pair<std::string, std::string> att_pair) {
            attributes_.emplace(att_pair);
        }

        void set_description(const std::string &description) {
            description_ = description;
        }

        void set_params(const std::vector<Field> &params) { params_ = params; }

        void add_params(const std::vector<Field> &params) {
            params_.insert(params_.end(), params.begin(), params.end());
        }

        void add_params(const std::string &params_xml) {
            tablator::ptree_readers::add_params_from_xml_string(params_, params_xml);
        }

        void add_param(const Field &param) { params_.emplace_back(param); }

        void add_labeled_properties(const std::vector<std::pair<std::string, Property>>
                                            &labeled_properties) {
            labeled_properties_.insert(labeled_properties_.end(),
                                       labeled_properties.begin(),
                                       labeled_properties.end());
            ;
        }

        void add_labeled_property(
                const std::pair<std::string, Property> &labeled_property) {
            labeled_properties_.emplace_back(labeled_property);
        }

        void add_group_elements(const std::vector<Group_Element> &group_elements) {
            group_elements_.insert(group_elements_.end(), group_elements.begin(),
                                   group_elements.end());
        }

        void add_group_element(const Group_Element &group_element) {
            group_elements_.emplace_back(group_element);
        }

        void add_table_elements(const std::vector<Table_Element> &table_elements) {
            table_elements_.insert(table_elements_.end(), table_elements.begin(),
                                   table_elements.end());
        }

        void add_table_element(const Table_Element &table_element) {
            table_elements_.emplace_back(table_element);
        }

        void add_trailing_info_list(const std::vector<Property> &trailing_info_list) {
            trailing_info_list_.insert(trailing_info_list_.end(),
                                       trailing_info_list.begin(),
                                       trailing_info_list.end());
        }

        void add_trailing_info(const Property &trailing_info) {
            trailing_info_list_.emplace_back(trailing_info);
        }
    };

public:
    class Builder {
    public:
        Builder() {}

        Builder(const std::vector<Table_Element> &table_elements) {
            options_.table_elements_ = table_elements;
        }

        Builder(const Table_Element &table_element) {
            options_.table_elements_.emplace_back(table_element);
        }

        Resource_Element build() { return Resource_Element(options_); }

        Builder &set_attributes(
                const std::initializer_list<std::pair<const std::string, std::string>>
                        attributes) {
            options_.set_attributes(attributes);
            return *this;
        }

        Builder &add_attributes(const ATTRIBUTES &attributes) {
            options_.add_attributes(attributes);
            return *this;
        }

        Builder &add_attribute(const std::pair<std::string, std::string> &att_pair) {
            options_.add_attribute(att_pair);
            return *this;
        }

        Builder &add_attribute(const std::string &name, const std::string &value) {
            options_.add_attribute(std::make_pair(name, value));
            return *this;
        }

        Builder &add_description(const std::string &description) {
            options_.set_description(description);
            return *this;
        }

        Builder &add_params(const std::vector<Field> &params) {
            options_.add_params(params);
            return *this;
        }

        Builder &add_params(const std::string &params_xml) {
            options_.add_params(params_xml);
            return *this;
        }

        Builder &add_param(const Field &param) {
            options_.add_param(param);
            return *this;
        }

        Builder &add_labeled_properties(
                const std::vector<std::pair<std::string, Property>>
                        &labeled_properties) {
            options_.add_labeled_properties(labeled_properties);
            return *this;
        }

        Builder &add_labeled_property(
                const std::pair<std::string, Property> &labeled_property) {
            options_.add_labeled_property(labeled_property);
            return *this;
        }

        Builder &add_group_elements(const std::vector<Group_Element> &group_elements) {
            options_.add_group_elements(group_elements);
            return *this;
        }

        Builder &add_group_element(const Group_Element &group_element) {
            options_.add_group_element(group_element);
            return *this;
        }

        Builder &add_table_elements(const std::vector<Table_Element> &table_elements) {
            options_.add_table_elements(table_elements);
            return *this;
        }

        Builder &add_table_element(const Table_Element &table_element) {
            options_.add_table_element(table_element);
            return *this;
        }

        Builder &add_trailing_info_list(
                const std::vector<Property> &trailing_info_list) {
            options_.add_trailing_info_list(trailing_info_list);
            return *this;
        }

        Builder &add_trailing_info(const Property &trailing_info) {
            options_.add_trailing_info(trailing_info);
            return *this;
        }

    private:
        Options options_;
    };


    // As of 05Aug20, if a Resource_Element has a Table_Element, its type is RESULTS.
    Resource_Element(const Table_Element &table_element)
            : resource_type_(Resource_Type::RESULTS) {
        assert(!table_element.get_columns().empty());
        get_table_elements().emplace_back(table_element);
    }

    size_t num_rows() const { return get_main_table_element().num_rows(); }

    // accessors for Optional elements

    const ATTRIBUTES &get_attributes() const { return options_.attributes_; }
    const std::string &get_description() const { return options_.description_; }
    const std::vector<Field> &get_params() const { return options_.params_; }
    std::vector<std::pair<std::string, Property>> &get_labeled_properties() {
        return options_.labeled_properties_;
    }
    const std::vector<std::pair<std::string, Property>> &get_labeled_properties()
            const {
        return options_.labeled_properties_;
    }
    const std::vector<Group_Element> &get_group_elements() const {
        return options_.group_elements_;
    }
    const std::vector<Property> &get_trailing_info_list() const {
        return options_.trailing_info_list_;
    }


    void add_attributes(const ATTRIBUTES &attributes) {
        options_.add_attributes(attributes);
    }

    void add_attribute(const std::string &name, const std::string &val) {
        options_.add_attribute(std::make_pair(name, val));
    }

    void add_attribute(const std::pair<std::string, std::string> &att_pair) {
        options_.add_attribute(att_pair);
    }

    void set_description(const std::string &description) {
        options_.set_description(description);
    }

    void set_params(const std::vector<Field> &params) { options_.set_params(params); }

    void add_params(const std::vector<Field> &params) { options_.add_params(params); }

    void add_param(const Field &param) { options_.add_param(param); }

    void add_labeled_properties(
            const std::vector<std::pair<std::string, Property>> &labeled_properties) {
        options_.add_labeled_properties(labeled_properties);
    }

    void add_labeled_property(
            const std::pair<std::string, Property> &labeled_property) {
        options_.add_labeled_property(labeled_property);
    }

    void add_group_elements(const std::vector<Group_Element> &group_elements) {
        options_.add_group_elements(group_elements);
    }

    void add_group_element(const Group_Element &group_element) {
        options_.add_group_element(group_element);
    }

    void add_trailing_info_list(const std::vector<Property> &trailing_info_list) {
        options_.add_trailing_info_list(trailing_info_list);
    }

    void add_trailing_info(const Property &trailing_info) {
        options_.add_trailing_info(trailing_info);
    }


    const std::vector<Table_Element> &get_table_elements() const {
        return options_.table_elements_;
    }
    std::vector<Table_Element> &get_table_elements() {
        return options_.table_elements_;
    }

    //  These will fail if type != RESULTS.

    const Table_Element &get_main_table_element() const {
        assert(!get_table_elements().empty());  // JTODO
        return get_table_elements().at(MAIN_TABLE_ELEMENT_IDX);
    }
    Table_Element &get_main_table_element() {
        assert(!get_table_elements().empty());
        return get_table_elements().at(MAIN_TABLE_ELEMENT_IDX);
    }

    const std::vector<Column> &get_columns() const {
        return get_main_table_element().get_columns();
    }
    std::vector<Column> &get_columns() {
        return get_main_table_element().get_columns();
    }

    const std::vector<size_t> &get_offsets() const {
        return get_main_table_element().get_offsets();
    }
    std::vector<size_t> &get_offsets() {
        return get_main_table_element().get_offsets();
    }

    std::vector<Field> &get_table_element_params() {
        return get_main_table_element().get_params();
    }
    const std::vector<Field> &get_table_element_params() const {
        return get_main_table_element().get_params();
    }

    void set_table_element_params(const std::vector<Field> &params) {
        get_main_table_element().set_params(params);
    }

    void add_table_element_params(const std::vector<Field> &params) {
        get_main_table_element().add_params(params);
    }

    const std::vector<uint8_t> &get_data() const {
        return get_main_table_element().get_data();
    }

    std::vector<uint8_t> &get_data() { return get_main_table_element().get_data(); }

    void set_data(const std::vector<uint8_t> &d) {
        get_main_table_element().set_data(d);
    }

    Resource_Type get_resource_type() const { return resource_type_; };

    bool is_results_resource() const {
        return (resource_type_ == Resource_Type::RESULTS);
    }


private:
    Resource_Element(const Options &options)
            : options_(options),
              resource_type_(determine_resource_type(options_.table_elements_,
                                                     options_.attributes_)) {}
    Options options_;
    Resource_Type resource_type_;
};  // class Resource_Element

inline bool operator<(const Resource_Element &lhs, const Resource_Element &rhs) {
    return (lhs.get_resource_type() < rhs.get_resource_type());
}


}  // namespace tablator
