#pragma once

#include <map>

#include "Common.hxx"
#include "Property.hxx"
#include "Values.hxx"

namespace tablator {
class Field_Properties {
public:
    static constexpr char const *FP_ATTRIBUTES = "attributes";
    static constexpr char const *FP_DESCRIPTION = "description";
    static constexpr char const *FP_LINKS = "links";
    static constexpr char const *FP_VALUES = "values";

private:
    struct Options {
        void set_attributes(
                const std::initializer_list<std::pair<const std::string, std::string>>
                        attributes) {
            attributes_ = attributes;
        }

        void set_attributes(const ATTRIBUTES &attributes) { attributes_ = attributes; }

        void add_attributes(const ATTRIBUTES &attributes) {
            attributes_.insert(attributes.begin(), attributes.end());
        }

        void add_attribute(const std::pair<std::string, std::string> &attr_pair) {
            attributes_.emplace(attr_pair);
        }

        void set_description(const std::string &description) {
            description_ = description;
        }

        void set_values(const Values &values) { values_ = values; }

        void add_hdf5_links(
                const std::vector<std::pair<std::string, std::string>> &hdf5_links) {
            hdf5_links_.insert(hdf5_links_.end(), hdf5_links.begin(), hdf5_links.end());
            ;
        }

        void set_links(const std::vector<Labeled_Property> &links) { links_ = links; }

        void add_links(const std::vector<Labeled_Property> &links) {
            links_.insert(links_.end(), links.begin(), links.end());
            ;
        }

        void add_link(const Labeled_Property &link) { links_.emplace_back(link); }


        ATTRIBUTES attributes_;
        std::string description_;
        Values values_;
        std::vector<std::pair<std::string, std::string>> hdf5_links_;
        std::vector<Labeled_Property> links_;
    };

public:
    class Builder {
    public:
        Field_Properties build() { return Field_Properties(options_); }

        Builder &add_attributes(const ATTRIBUTES &attributes) {
            options_.add_attributes(attributes);
            return *this;
        }

        Builder &add_attributes(
                const std::initializer_list<std::pair<const std::string, std::string>>
                        attributes) {
            options_.add_attributes(attributes);
            return *this;
        }

        Builder &add_attribute(const std::pair<std::string, std::string> &attr_pair) {
            options_.add_attribute(attr_pair);
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

        Builder &add_values(const Values &values) {
            options_.set_values(values);
            return *this;
        }

        Builder &add_hdf5_links(
                const std::vector<std::pair<std::string, std::string>> &hdf5_links) {
            options_.add_hdf5_links(hdf5_links);
            return *this;
        }

        Builder &add_links(const std::vector<Labeled_Property> &links) {
            options_.add_links(links);
            return *this;
        }

        Builder &add_link(const Labeled_Property &link) {
            options_.add_link(link);
            return *this;
        }

    private:
        Options options_;
    };

    // JTODO hide this?
    Field_Properties() = default;

    // JTODO query_server uses these constructors extensively.

    Field_Properties(const ATTRIBUTES &attributes) {
        Builder().add_attributes(attributes).build();
    }

    Field_Properties(
            const std::initializer_list<std::pair<const std::string, std::string>>
                    &attributes) {
        Builder().add_attributes(attributes).build();
    }

    Field_Properties(const std::string &description) {
        Builder().add_description(description).build();
    }

    Field_Properties(
            const std::string &description,
            const std::initializer_list<std::pair<const std::string, std::string>>
                    &attributes) {
        Builder().add_description(description).add_attributes(attributes).build();
    }

    Field_Properties(const std::string &description, const ATTRIBUTES &attributes) {
        Builder().add_description(description).add_attributes(attributes).build();
    }

    Field_Properties(const std::string &description, const ATTRIBUTES &attributes,
                     const Values &v, const std::vector<Labeled_Property> &links) {
        Builder()
                .add_description(description)
                .add_attributes(attributes)
                .add_values(v)
                .add_links(links)
                .build();
    }


    const ATTRIBUTES &get_attributes() const { return options_.attributes_; }
    ATTRIBUTES &get_attributes() { return options_.attributes_; }

    const std::string &get_description() const { return options_.description_; }

    const Values &get_values() const { return options_.values_; }
    Values &get_values() { return options_.values_; }

    const std::vector<std::pair<std::string, std::string>> &get_hdf5_links() const {
        return options_.hdf5_links_;
    }
    std::vector<std::pair<std::string, std::string>> &get_hdf5_links() {
        return options_.hdf5_links_;
    }
    const std::vector<Labeled_Property> &get_links() const { return options_.links_; }
    std::vector<Labeled_Property> &get_links() { return options_.links_; }


    void set_attributes(const ATTRIBUTES &attrs) { options_.set_attributes(attrs); }

    void add_attribute(const std::pair<std::string, std::string> &attr_pair) {
        options_.add_attribute(attr_pair);
    }

    void add_attribute(const std::string &name, const std::string &value) {
        add_attribute(std::make_pair(name, value));
    }

    void set_description(const std::string &desc) { options_.set_description(desc); }

    void set_values(const Values &values) { options_.set_values(values); }


    void add_hdf5_links(
            const std::vector<std::pair<std::string, std::string>> &hdf5_links) {
        options_.add_hdf5_links(hdf5_links);
    }

    void set_links(const std::vector<Labeled_Property> &links) {
        options_.set_links(links);
    }
    void add_links(const std::vector<Labeled_Property> &links) {
        options_.add_links(links);
    }

    void add_link(const Labeled_Property &link_pair) { options_.add_link(link_pair); }

private:
    Field_Properties(Options &options) : options_(options) {}
    Options options_;
};
}  // namespace tablator
