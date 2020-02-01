#pragma once

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>

#include "Column.hxx"
#include "Common.hxx"
#include "Group_Element.hxx"
#include "Property.hxx"
#include "Table_Element.hxx"

namespace tablator {

class Resource_Element {
    // JTODO check for required attributes.

private:
    struct Options {
        ATTRIBUTES attributes_;
        std::string description_;
        std::vector<std::pair<std::string, Property>> labeled_properties_;
        std::vector<Group_Element> group_elements_;
        std::vector<Field> params_;
        boost::property_tree::ptree params_ptree_;
        std::vector<Property> trailing_info_list_;
    };

public:
    class Builder {
    public:
        Builder(const std::vector<Table_Element> &table_elements)
                : table_elements_(table_elements) {}

        Builder(const Table_Element &table_element) {
            table_elements_.emplace_back(table_element);
        }


        Resource_Element build() { return Resource_Element(table_elements_, options_); }

        Builder &add_attributes(const ATTRIBUTES &attributes) {
            options_.attributes_ = attributes;
            return *this;
        }

        Builder &add_description(const std::string &description) {
            options_.description_ = description;
            return *this;
        }

        Builder &add_params(const std::vector<Field> &params) {
            options_.params_ = params;
            return *this;
        }

        Builder &add_params_tree(const boost::property_tree::ptree &params_ptree) {
            options_.params_ptree_ = params_ptree;
            return *this;
        }

        Builder &add_labeled_properties(
                const std::vector<std::pair<std::string, Property>>
                        &labeled_properties) {
            options_.labeled_properties_ = labeled_properties;
            return *this;
        }

        Builder &add_group_elements(const std::vector<Group_Element> &group_elements) {
            options_.group_elements_ = group_elements;
            return *this;
        }


        Builder &add_trailing_info_list(
                const std::vector<Property> &trailing_info_list) {
            options_.trailing_info_list_ = trailing_info_list;
            return *this;
        }

    private:
        std::vector<Table_Element> table_elements_;
        Options options_;
    };

    Resource_Element(const Table_Element &table_element) {
        table_elements_.emplace_back(table_element);
    }

    Resource_Element(const std::vector<Table_Element> &table_elements)
            : table_elements_(table_elements) {}


    inline size_t num_rows() const { return get_table_elements().at(0).num_rows(); }

    // accessors

    inline const ATTRIBUTES &get_attributes() const { return options_.attributes_; }
    inline const std::string &get_description() const { return options_.description_; }

    inline const std::vector<Field> &get_params() const { return options_.params_; }
    inline const boost::property_tree::ptree &get_params_ptree() const {
        return options_.params_ptree_;
    }

    inline std::vector<std::pair<std::string, Property>> &get_labeled_properties() {
        return options_.labeled_properties_;
    }

    inline const std::vector<std::pair<std::string, Property>> &get_labeled_properties()
            const {
        return options_.labeled_properties_;
    }

    inline const std::vector<Group_Element> &get_group_elements() const {
        return options_.group_elements_;
    }

    inline const std::vector<Property> &get_trailing_info_list() const {
        return options_.trailing_info_list_;
    }

    inline const std::vector<Table_Element> &get_table_elements() const {
        return table_elements_;
    }

    inline std::vector<Table_Element> &get_table_elements() { return table_elements_; }

    inline const std::vector<Column> &get_columns() const {
        return get_table_elements().at(0).get_columns();
    }
    inline std::vector<Column> &get_columns() {
        return get_table_elements().at(0).get_columns();
    }

    inline const std::vector<size_t> &get_offsets() const {
        return get_table_elements().at(0).get_offsets();
    }
    inline std::vector<size_t> &get_offsets() {
        return get_table_elements().at(0).get_offsets();
    }

    inline std::vector<Field> &get_table_element_params() {
        return get_table_elements().at(0).get_params();
    }

    inline const std::vector<Field> &get_table_element_params() const {
        return get_table_elements().at(0).get_params();
    }

    inline const std::vector<uint8_t> &get_data() const {
        return get_table_elements().at(0).get_data();
    }

    inline std::vector<uint8_t> &get_data() {
        return get_table_elements().at(0).get_data();
    }

    inline void set_params(const std::vector<Field> &params) {
        options_.params_ = params;
    }

    inline void set_table_element_params(const std::vector<Field> &params) {
        assert(!get_table_elements().empty());
        get_table_elements().at(0).set_params(params);
    }

    inline void add_param(const Field &param) {
        if (!options_.params_ptree_.empty()) {
            throw std::runtime_error(
                    "Cannot add <param> if params_ptree is non-empty.");
        }
        options_.params_.emplace_back(param);
    }

    inline void add_labeled_property(
            const std::pair<std::string, Property> &labeled_property) {
        options_.labeled_properties_.emplace_back(labeled_property);
    }

    inline void set_data(const std::vector<uint8_t> &d) {
        get_table_elements().at(0).set_data(d);
    }

private:
    Resource_Element(const std::vector<Table_Element> &table_elements,
                     const Options &options)
            : table_elements_(table_elements), options_(options) {}


    std::vector<Table_Element> table_elements_;
    Options options_;
};

}  // namespace tablator
