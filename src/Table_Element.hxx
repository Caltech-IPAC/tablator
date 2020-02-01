#pragma once

#include <boost/algorithm/string/join.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>

#include "Column.hxx"
#include "Common.hxx"
#include "Data_Element.hxx"
#include "Group_Element.hxx"


namespace tablator {

// JTODO check for required attributes.

class Table_Element {
private:
    struct Options {
        ATTRIBUTES attributes_;
        std::string description_;
        std::vector<Field> params_;
        boost::property_tree::ptree params_ptree_;
        std::vector<Group_Element> group_elements_;
        std::vector<Field> fields_;
        std::vector<Property> trailing_info_list_;
    };


public:
    class Builder {
    public:
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

        Builder &add_group_elements(const std::vector<Group_Element> &group_elements) {
            options_.group_elements_ = group_elements;
            return *this;
        }

        Builder &add_fields(const std::vector<Field> &fields) {
            options_.fields_.insert(options_.fields_.end(), fields.begin(),
                                    fields.end());
            return *this;
        }

        Builder &add_trailing_info_list(
                const std::vector<Property> &trailing_info_list) {
            options_.trailing_info_list_ = trailing_info_list;
            return *this;
        }


    private:
        std::vector<Data_Element> data_elements_;
        Options options_;
    };


    Table_Element() {}

    // accessors

    inline size_t row_size() const { return get_data_elements().at(0).row_size(); }
    inline size_t num_rows() const { return get_data_elements().at(0).num_rows(); }

    inline const ATTRIBUTES &get_attributes() const { return options_.attributes_; }
    inline const std::string &get_description() const { return options_.description_; }
    inline const std::vector<Property> &get_trailing_info_list() const {
        return options_.trailing_info_list_;
    }
    inline const std::vector<Group_Element> &get_group_elements() const {
        return options_.group_elements_;
    }


    inline std::vector<Field> &get_params() { return options_.params_; }
    inline const std::vector<Field> &get_params() const { return options_.params_; }

    inline const boost::property_tree::ptree &get_params_ptree() const {
        return options_.params_ptree_;
    }
    inline const std::vector<Data_Element> &get_data_elements() const {
        return data_elements_;
    }
    inline std::vector<Data_Element> &get_data_elements() { return data_elements_; }

    inline const std::vector<Column> &get_columns() const {
        return get_data_elements().at(0).get_columns();
    }
    inline std::vector<Column> &get_columns() {
        return get_data_elements().at(0).get_columns();
    }

    inline const std::vector<size_t> &get_offsets() const {
        return get_data_elements().at(0).get_offsets();
    }
    inline std::vector<size_t> &get_offsets() {
        return get_data_elements().at(0).get_offsets();
    }

    inline const std::vector<uint8_t> &get_data() const {
        return get_data_elements().at(0).get_data();
    }
    inline std::vector<uint8_t> &get_data() {
        return get_data_elements().at(0).get_data();
    }

    inline void add_param(const Field &param) {
        if (!options_.params_ptree_.empty()) {
            throw std::runtime_error(
                    "Cannot add <param> if params_ptree is non-empty.");
        }
        options_.params_.emplace_back(param);
    }

    inline void set_params(const std::vector<Field> &params) {
        options_.params_ = params;
    }

    inline void set_data(const std::vector<uint8_t> &d) {
        get_data_elements().at(0).set_data(d);
    }

private:
    Table_Element(const std::vector<Data_Element> &data_elements,
                  const Options &options)
            : data_elements_(data_elements), options_(options) {}


    std::vector<Data_Element> data_elements_;
    Options options_;
};

}  // namespace tablator
