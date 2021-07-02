#pragma once

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>

#include <string>

#include "Column.hxx"
#include "Common.hxx"
#include "ptree_readers.hxx"

namespace tablator {

class Group_Element {
private:
    struct Options {
        ATTRIBUTES attributes_;
        std::string description_;
        std::vector<Field> params_;
        std::vector<ATTRIBUTES> field_refs_;
        std::vector<ATTRIBUTES> param_refs_;

        void set_attributes(
                const std::initializer_list<std::pair<const std::string, std::string> >
                        attributes) {
            attributes_ = attributes;
        }

        void add_attributes(const ATTRIBUTES &attributes) {
            attributes_.insert(attributes.begin(), attributes.end());
        }

        void add_attribute(const STRING_PAIR attr_pair) {
            attributes_.emplace(attr_pair);
        }

        void set_description(const std::string &description) {
            description_ = description;
        }

        void add_params(const std::vector<Field> &params) {
            params_.insert(params_.end(), params.begin(), params.end());
        }

        void add_params(const std::string &params_xml) {
            tablator::ptree_readers::add_params_from_xml_string(params_, params_xml);
        }

        void add_param(const Field &param) { params_.emplace_back(param); }

        void add_field_refs(const std::vector<ATTRIBUTES> &field_refs) {
            field_refs_.insert(field_refs_.end(), field_refs.begin(), field_refs.end());
        }

        void add_field_ref(const ATTRIBUTES &field_ref) {
            field_refs_.emplace_back(field_ref);
        }

        void add_param_refs(const std::vector<ATTRIBUTES> &param_refs) {
            param_refs_.insert(param_refs_.end(), param_refs.begin(), param_refs.end());
        }

        void add_param_ref(const ATTRIBUTES &param_ref) {
            param_refs_.emplace_back(param_ref);
        }
    };


public:
    class Builder {
    public:
        Group_Element build() { return Group_Element(options_); }

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

        Builder &add_field_refs(const std::vector<ATTRIBUTES> &field_refs) {
            options_.add_field_refs(field_refs);
            return *this;
        }

        Builder &add_field_ref(const ATTRIBUTES &field_ref) {
            options_.add_field_ref(field_ref);
            return *this;
        }

        Builder &add_param_refs(const std::vector<ATTRIBUTES> &param_refs) {
            options_.add_param_refs(param_refs);
            return *this;
        }

        Builder &add_param_ref(const ATTRIBUTES &param_ref) {
            options_.add_param_ref(param_ref);
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

    void set_attributes(
            const std::initializer_list<std::pair<const std::string, std::string> >
                    attributes) {
        options_.set_attributes(attributes);
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

    void add_params(const std::vector<Field> &params) { options_.add_params(params); }

    void add_param(const Field &param) { options_.add_param(param); }

    void add_field_refs(const std::vector<ATTRIBUTES> &field_refs) {
        options_.add_field_refs(field_refs);
    }

    void add_field_ref(const ATTRIBUTES &field_ref) {
        options_.add_field_ref(field_ref);
    }

    void add_param_refs(const std::vector<ATTRIBUTES> &param_refs) {
        options_.add_param_refs(param_refs);
    }

    void add_param_ref(const ATTRIBUTES &param_ref) {
        options_.add_param_ref(param_ref);
    }

private:
    Group_Element(Options &options) : options_(options) {}
    Options options_;
};

}  // namespace tablator
