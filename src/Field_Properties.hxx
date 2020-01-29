#pragma once

#include <map>

#include "Common.hxx"
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
        Options() = default;

        // Add all these constructors to support old-style Field_Properties
        // constructors, which are called from query_server.
        Options(const ATTRIBUTES &attributes) : attributes_(attributes) {}
        Options(const std::initializer_list<std::pair<const std::string, std::string> >
                        &attributes)
                : attributes_(attributes) {}
        Options(const std::string &description) : description_(description) {}


        Options(const std::string &description, const ATTRIBUTES &attributes)
                : attributes_(attributes), description_(description) {}

        Options(const std::string &description,
                const std::initializer_list<std::pair<const std::string, std::string> >
                        &attributes)
                : attributes_(attributes), description_(description) {}


        Options(const std::string &description, const ATTRIBUTES &attributes,
                const Values &v,
                const std::vector<std::pair<std::string, std::string> > &links)
                : attributes_(attributes),
                  description_(description),
                  values_(v),
                  links_(links) {}


        ATTRIBUTES attributes_;
        std::string description_;
        Values values_;
        std::vector<std::pair<std::string, std::string> > links_;
    };

public:
    class Builder {
    public:
        Field_Properties build() { return Field_Properties(options_); }

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

        Builder &add_values(const Values &values) {
            options_.values_ = values;
            return *this;
        }

        Builder &add_links(
                const std::vector<std::pair<std::string, std::string> > &links) {
            options_.links_ = links;
            return *this;
        }

    private:
        Options options_;
    };

    // JTODO hide this?
    Field_Properties() = default;

    // JTODO query_server uses these constructors extensively.
    Field_Properties(const ATTRIBUTES &attributes) : options_(attributes) {}

    Field_Properties(
            const std::initializer_list<std::pair<const std::string, std::string> >
                    &attributes)
            : options_(attributes) {}


    Field_Properties(const std::string &description) : options_(description) {}

    Field_Properties(
            const std::string &description,
            const std::initializer_list<std::pair<const std::string, std::string> >
                    &attributes)
            : options_(description, attributes) {}

    Field_Properties(const std::string &description, const ATTRIBUTES &attributes) {
        Builder().add_description(description).add_attributes(attributes).build();
    }


    Field_Properties(const std::string &description, const ATTRIBUTES &attributes,
                     const Values &v,
                     const std::vector<std::pair<std::string, std::string> > &links) {
        set_description(description);
        set_attributes(attributes);
        set_values(v);
        set_links(links);
    }

    inline const std::string &get_description() const { return options_.description_; }

    inline const ATTRIBUTES &get_attributes() const { return options_.attributes_; }
    inline ATTRIBUTES &get_attributes() { return options_.attributes_; }

    inline const Values &get_values() const { return options_.values_; }
    inline Values &get_values() { return options_.values_; }

    inline const std::vector<std::pair<std::string, std::string> > &get_links() const {
        return options_.links_;
    }
    inline std::vector<std::pair<std::string, std::string> > &get_links() {
        return options_.links_;
    }

    inline void set_description(const std::string &desc) {
        options_.description_.assign(desc);
    }
    inline void set_attributes(const ATTRIBUTES &attrs) {
        options_.attributes_ = attrs;
    }
    inline void set_values(const Values &values) { options_.values_ = values; }
    inline void set_links(
            const std::vector<std::pair<std::string, std::string> > &links) {
        options_.links_ = links;
    }

    inline void add_attribute(const std::pair<std::string, std::string> &att_pair) {
        options_.attributes_.insert(att_pair);
    }

    inline void add_attribute(const std::string &name, const std::string &value) {
        add_attribute(std::make_pair(name, value));
    }

    inline void add_link(const std::pair<std::string, std::string> &link_pair) {
        options_.links_.emplace_back(link_pair);
    }

    inline void add_link(const std::string &name, const std::string &value) {
        add_link(std::make_pair(name, value));
    }


private:
    Field_Properties(Options &options) : options_(options) {}
    Options options_;
};
}  // namespace tablator
