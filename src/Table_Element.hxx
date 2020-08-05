#pragma once

#include <boost/algorithm/string/join.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>

#include "Column.hxx"
#include "Common.hxx"
#include "Data_Element.hxx"
#include "Group_Element.hxx"


namespace tablator {
static constexpr size_t DEFAULT_DATA_ELEMENT_IDX = 0;

// JTODO check for required attributes.
// JTODO Fields should match up with Columns.

class Table_Element {
private:
    struct Options {
        ATTRIBUTES attributes_;
        std::string description_;
        std::vector<Field> params_;
        std::vector<Group_Element> group_elements_;
        std::vector<Field> fields_;
        std::vector<Property> trailing_info_list_;

        void set_attributes(
                const std::initializer_list<std::pair<const std::string, std::string> >
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


        void add_group_elements(const std::vector<Group_Element> &group_elements) {
            group_elements_.insert(group_elements_.end(), group_elements.begin(),
                                   group_elements.end());
        }

        void add_group_element(const Group_Element &group_element) {
            group_elements_.emplace_back(group_element);
        }

        void add_fields(const std::vector<Field> &fields) {
            fields_.insert(fields_.end(), fields.begin(), fields.end());
        }

        void add_field(const Field &field) { fields_.emplace_back(field); }

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

        Builder(const Data_Element &data_element) {
            data_elements_.emplace_back(data_element);
        }

        Builder(const std::vector<Data_Element> &data_elements)
                : data_elements_(data_elements) {}

        Builder(const std::vector<Column> &columns, const std::vector<size_t> &offsets,
                const std::vector<uint8_t> &data) {
            data_elements_.emplace_back(Data_Element(columns, offsets, data));
        }

        Table_Element build() { return Table_Element(data_elements_, options_); }

        Builder &set_attributes(
                const std::initializer_list<std::pair<const std::string, std::string> >
                        attributes) {
            options_.set_attributes(attributes);
            return *this;
        }

        Builder &add_attributes(const ATTRIBUTES &attributes) {
            options_.add_attributes(attributes);
            return *this;
        }

        Builder &add_attribute(const std::string &name, const std::string &val) {
            options_.add_attribute(std::make_pair(name, val));
            return *this;
        }

        Builder &add_attribute(const std::pair<std::string, std::string> att_pair) {
            options_.add_attribute(att_pair);
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

        Builder &add_group_elements(const std::vector<Group_Element> &group_elements) {
            options_.add_group_elements(group_elements);
            return *this;
        }

        Builder &add_fields(const std::vector<Field> &fields) {
            options_.add_fields(fields);
            return *this;
        }

        Builder &add_field(const Field &field) {
            options_.add_field(field);
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
        std::vector<Data_Element> data_elements_;
        Options options_;
    };


    Table_Element() {}


    size_t row_size() const { return get_main_data_element().row_size(); }
    size_t num_rows() const { return get_main_data_element().num_rows(); }

    // accessors for Optional elements
    const ATTRIBUTES &get_attributes() const { return options_.attributes_; }
    const std::string &get_description() const { return options_.description_; }
    std::vector<Field> &get_params() { return options_.params_; }
    const std::vector<Field> &get_params() const { return options_.params_; }
    const std::vector<Group_Element> &get_group_elements() const {
        return options_.group_elements_;
    }
    const std::vector<Field> &get_fields() const { return options_.fields_; }
    const std::vector<Property> &get_trailing_info_list() const {
        return options_.trailing_info_list_;
    }

    void add_attributes(const ATTRIBUTES &attributes) {
        options_.add_attributes(attributes);
    }

    void add_attribute(const std::string &name, const std::string &val) {
        options_.add_attribute(std::make_pair(name, val));
    }

    void set_description(const std::string &description) {
        options_.set_description(description);
    }
    void add_description(const std::string &description) {
        set_description(description);
    }

    void set_params(const std::vector<Field> &params) { options_.set_params(params); }

    void add_params(const std::vector<Field> &params) { options_.add_params(params); }

    void add_params(const std::string &params_xml) { options_.add_params(params_xml); }
    void add_param(const Field &param) { options_.add_param(param); }

    void add_group_elements(const std::vector<Group_Element> &elements) {
        options_.add_group_elements(elements);
    }

    void add_group_element(const Group_Element &group_element) {
        options_.add_group_element(group_element);
    }

    void add_fields(const std::vector<Field> &fields) { options_.add_fields(fields); }
    void add_field(const Field &field) { options_.add_field(field); }

    void add_trailing_info_list(const std::vector<Property> &trailing_info_list) {
        options_.add_trailing_info_list(trailing_info_list);
    }

    void add_trailing_info(const Property &prop) { options_.add_trailing_info(prop); }


    //===========================================
    // accessors for non-Optional elements
    const std::vector<Data_Element> &get_data_elements() const {
        return data_elements_;
    }
    std::vector<Data_Element> &get_data_elements() { return data_elements_; }

    const Data_Element &get_main_data_element() const {
        assert(!get_data_elements().empty());
        return data_elements_.at(DEFAULT_DATA_ELEMENT_IDX);
    }
    Data_Element &get_main_data_element() {
        assert(!get_data_elements().empty());
        return data_elements_.at(DEFAULT_DATA_ELEMENT_IDX);
    }

    const std::vector<Column> &get_columns() const {
        return get_main_data_element().get_columns();
    }
    std::vector<Column> &get_columns() { return get_main_data_element().get_columns(); }

    const std::vector<size_t> &get_offsets() const {
        return get_main_data_element().get_offsets();
    }

    std::vector<size_t> &get_offsets() { return get_main_data_element().get_offsets(); }

    const std::vector<uint8_t> &get_data() const {
        return get_main_data_element().get_data();
    }
    std::vector<uint8_t> &get_data() { return get_main_data_element().get_data(); }

    void set_data(const std::vector<uint8_t> &d) {
        get_main_data_element().set_data(d);
    }

private:
    Table_Element(const std::vector<Data_Element> &data_elements,
                  const Options &options)
            : data_elements_(data_elements), options_(options) {
        // JTODO  What exactly is our restriction on Table_Elements?
        if (data_elements_.empty() || get_columns().empty()) {
            throw std::runtime_error("Table_Element must have non-empty Data_Element.");
        }
    }


    std::vector<Data_Element> data_elements_;
    Options options_;
};

}  // namespace tablator
