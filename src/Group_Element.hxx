#pragma once

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>

#include <string>

#include "Column.hxx"
#include "Common.hxx"

namespace tablator {

class Group_Element {
private:
    struct Options {
        ATTRIBUTES attributes_;
        std::string description_;
        std::vector<Field> params_;
        std::vector<ATTRIBUTES> field_refs_;
        std::vector<ATTRIBUTES> param_refs_;
    };


public:
    class Builder {
    public:
        Group_Element build() { return Group_Element(options_); }

        Builder &add_attributes(const ATTRIBUTES &attributes) {
            options_.attributes_ = attributes;
            return *this;
        }

        Builder &add_attributes(
                const std::initializer_list<std::pair<const std::string, std::string> >
                        attributes) {
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
        Builder &add_params(const std::string &params_xml);
        Builder &add_field_refs(const std::vector<ATTRIBUTES> &field_refs) {
            options_.field_refs_ = field_refs;
            return *this;
        }

        Builder &add_param_refs(const std::vector<ATTRIBUTES> &param_refs) {
            options_.param_refs_ = param_refs;
            return *this;
        }

    private:
        Options options_;
    };


public:
    // accessors

    const ATTRIBUTES &get_attributes() const { return options_.attributes_; }
    const std::string &get_description() const { return options_.description_; }
    const std::vector<Field> &get_params() const { return options_.params_; }
    const std::vector<ATTRIBUTES> &get_field_refs() const {
        return options_.field_refs_;
    }

    void add_param(const Field &param) { options_.params_.emplace_back(param); }

    void add_field_ref(const ATTRIBUTES &field_ref) {
        options_.field_refs_.emplace_back(field_ref);
    }

    void add_param_ref(const ATTRIBUTES &param_ref) {
        options_.param_refs_.emplace_back(param_ref);
    }

private:
    Group_Element(Options &options) : options_(options) {}
    Options options_;
};

}  // namespace tablator
