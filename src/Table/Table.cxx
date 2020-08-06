#include "../Table.hxx"
#include "../ptree_readers.hxx"

namespace {
void append_info_list_with_label(
        std::vector<std::pair<std::string, tablator::Property>> &combined_list,
        const std::vector<tablator::Property> &info_list, const std::string &label) {
    std::transform(info_list.begin(), info_list.end(),
                   std::back_inserter(combined_list),
                   [&](const tablator::Property &prop)
                           -> std::pair<std::string, tablator::Property> {
                       return std::make_pair(label, prop);
                   });
}

void append_attributes_with_label(
        std::vector<std::pair<std::string, tablator::Property>> &combined_list,
        const tablator::ATTRIBUTES &attrs, const std::string &label) {
    combined_list.emplace_back(std::make_pair(label, tablator::Property(attrs)));
}

}  // namespace

namespace tablator {

Table_Element load_columns_and_offsets(const std::vector<Column> &columns) {
    if (columns.empty()) {
        throw std::runtime_error("This table has no columns");
    }

    std::vector<Column> tabledata_columns;
    std::vector<size_t> tabledata_offsets = {0};

    const size_t null_flags_size = bits_to_bytes(columns.size());
    tablator::append_column(tabledata_columns, tabledata_offsets,
                            null_bitfield_flags_name, Data_Type::UINT8_LE,
                            null_flags_size,
                            Field_Properties::Builder()
                                    .add_description(null_bitfield_flags_description)
                                    .build());

    for (auto &c : columns) {
        tablator::append_column(tabledata_columns, tabledata_offsets, c);
    }

    return Table_Element::Builder(tabledata_columns, tabledata_offsets,
                                  std::vector<uint8_t>() /*  data */)
            .build();
}


Table::Table(const std::vector<Column> &columns,
             const std::map<std::string, std::string> &property_map) {
    add_resource_element(load_columns_and_offsets(columns));

    for (auto &p : property_map) {
        add_labeled_property(p.first, Property(p.second));
    }
}

Table::Table(const std::vector<Column> &columns,
             const std::vector<std::pair<std::string, Property>> &property_pair_vec)
        : results_resource_idx_(0) {
    add_resource_element(load_columns_and_offsets(columns));

    for (const auto &label_and_prop : property_pair_vec) {
        if (boost::starts_with(label_and_prop.first, VOTABLE_RESOURCE_DOT)) {
            add_resource_element_labeled_property(
                    label_and_prop.first.substr(VOTABLE_RESOURCE_DOT.size()),
                    label_and_prop.second);
        } else {
            add_labeled_property(label_and_prop);
        }
    }
}

Table::Table(const boost::filesystem::path &input_path, const Format &format) {
    switch (format.enum_format) {
        case Format::Enums::HDF5:
            H5::Exception::dontPrint();
            read_hdf5(input_path);
            break;
        case Format::Enums::FITS:
            read_fits(input_path);
            break;
        case Format::Enums::IPAC_TABLE:
        case Format::Enums::TEXT:
            read_ipac_table(input_path);
            break;
        case Format::Enums::JSON5:
            read_json5(input_path);
            break;
        case Format::Enums::VOTABLE:
            read_votable(input_path);
            break;
        case Format::Enums::JSON:
            read_json(input_path);
            break;
        case Format::Enums::CSV:
        case Format::Enums::TSV:
            read_dsv(input_path, format);
            break;
        case Format::Enums::UNKNOWN:
            read_unknown(input_path);
            break;
        default:
            throw std::runtime_error("Unsupported input format: '" + format.string() +
                                     "' for input file: " + input_path.string());
            break;
    }
    if (get_columns().size() < 2) {
        throw std::runtime_error("This file has no columns: " + input_path.string());
    }
    arrange_resources();
}

Table::Table(std::istream &input_stream, const Format &format) {
    switch (format.enum_format) {
            // FIXME: Implement streaming for HDF5 and FITS
        // case Format::Enums::HDF5:
        //   H5::Exception::dontPrint ();
        //   read_hdf5 (input_stream);
        //   break;
        // case Format::Enums::FITS:
        //   read_fits (input_stream);
        //   break;
        case Format::Enums::IPAC_TABLE:
        case Format::Enums::TEXT:
            read_ipac_table(input_stream);
            break;
        case Format::Enums::JSON5:
            read_json5(input_stream);
            break;
        case Format::Enums::VOTABLE:
            read_votable(input_stream);
            break;
        case Format::Enums::JSON:
            read_json(input_stream);
            break;
        case Format::Enums::CSV:
        case Format::Enums::TSV:
            read_dsv(input_stream, format);
            break;
        case Format::Enums::UNKNOWN:
            read_unknown(input_stream);
            break;
        default:
            throw std::runtime_error("Unsupported input format for streaming: " +
                                     format.string());
            break;
    }
    if (get_columns().size() < 2) {
        throw std::runtime_error("This stream has no columns");
    }
    arrange_resources();
}

//===========================================================

void Table::read_votable(std::istream &input_stream) {
    boost::property_tree::ptree tree;
    using namespace boost::property_tree::xml_parser;
    boost::property_tree::read_xml(input_stream, tree, no_comments);
    ptree_readers::read_property_tree_as_votable(*this, tree);
}

void Table::read_json(std::istream &input_stream) {
    boost::property_tree::ptree tree;
    boost::property_tree::read_json(input_stream, tree);
    tablator::ptree_readers::read_property_tree_as_votable(*this, tree);
}


//===========================================================

const std::vector<std::pair<std::string, Property>>
Table::combine_trailing_info_lists_all_levels() const {
    std::vector<std::pair<std::string, Property>> combined_list;
    append_info_list_with_label(combined_list, get_trailing_info_list(),
                                VOTABLE_DOT + END_INFO_MARKER);
    append_info_list_with_label(combined_list,
                                get_results_resource_element().get_trailing_info_list(),
                                VOTABLE_RESOURCE_DOT + END_INFO_MARKER);
    append_info_list_with_label(combined_list,
                                get_main_table_element().get_trailing_info_list(),
                                VOTABLE_RESOURCE_TABLE_DOT + END_INFO_MARKER);
    return combined_list;
}


//===========================================================

const std::vector<std::pair<std::string, Property>>
Table::combine_attributes_all_levels() const {
    std::vector<std::pair<std::string, Property>> combined_list;
    append_attributes_with_label(combined_list, get_attributes(),
                                 VOTABLE_DOT + ATTR_MARKER);
    append_attributes_with_label(combined_list,
                                 get_results_resource_element().get_attributes(),
                                 VOTABLE_RESOURCE_DOT + ATTR_MARKER);
    append_attributes_with_label(combined_list,
                                 get_main_table_element().get_attributes(),
                                 VOTABLE_RESOURCE_TABLE_DOT + ATTR_MARKER);
    return combined_list;
}


//===========================================================

const std::vector<std::pair<std::string, Property>>
Table::combine_labeled_properties_all_levels() const {
    std::vector<std::pair<std::string, Property>> combined_labeled_props(
            get_labeled_properties());
    const auto &resource_labeled_props = get_resource_element_labeled_properties();
    std::transform(resource_labeled_props.begin(), resource_labeled_props.end(),
                   std::back_inserter(combined_labeled_props),
                   [](const std::pair<std::string, Property> &nppair)
                           -> std::pair<std::string, Property> {
                       return std::make_pair(VOTABLE_RESOURCE_DOT + nppair.first,
                                             nppair.second);
                   });
    return combined_labeled_props;
}
//===========================================================

// write_fits() and write_hdf5() store all levels of <labeled_properties>,
// <trailing_info_lists>, and <attributes> in the same big
// metadata pile.  This function sorts them out again.

// n.b. The class members by those names of <table> already exist by
// the time read_XXX() is called, but <table>'s Resource_Element and
// Table_Element class members haven't been constructed yet.  We store
// the eventual contents of their class members in the vector
// arguments to this function, which are then used as arguments to the
// relevant constructors.

void Table::distribute_metadata(
        std::vector<std::pair<std::string, Property>>
                &resource_element_labeled_properties,
        std::vector<Property> &resource_element_trailing_infos,
        ATTRIBUTES &resource_element_attributes,
        std::vector<Property> &table_element_trailing_infos,
        ATTRIBUTES &table_element_attributes,
        const std::vector<std::pair<std::string, Property>> &label_prop_pairs) {
    for (const auto &label_and_prop : label_prop_pairs) {
        const auto &label = label_and_prop.first;
        const auto &prop = label_and_prop.second;
        if (add_trailing_info_labeled_by_element(resource_element_trailing_infos,
                                                 table_element_trailing_infos,
                                                 label_and_prop)) {
            continue;
        }
        if (add_attributes_labeled_by_element(resource_element_attributes,
                                              table_element_attributes,
                                              label_and_prop)) {
            continue;
        }
        add_element_labeled_property(resource_element_labeled_properties,
                                     label_and_prop);
    }
}


//=============================================

bool Table::add_trailing_info_labeled_by_element(
        std::vector<Property> &resource_element_infos,
        std::vector<Property> &table_element_infos,
        const std::pair<std::string, Property> &label_and_prop) {
    const auto &label = label_and_prop.first;
    const auto &prop = label_and_prop.second;
    if (boost::equals(label, VOTABLE_RESOURCE_TABLE_DOT + END_INFO_MARKER)) {
        table_element_infos.emplace_back(prop);
        return true;
    }
    if (boost::equals(label, VOTABLE_RESOURCE_DOT + END_INFO_MARKER)) {
        resource_element_infos.emplace_back(prop);
        return true;
    }
    if (boost::ends_with(label, END_INFO_MARKER)) {
        add_trailing_info(prop);
        return true;
    }
    return false;
}

//=============================================

bool Table::add_attributes_labeled_by_element(
        ATTRIBUTES &resource_element_attributes, ATTRIBUTES &table_element_attributes,
        const std::pair<std::string, Property> &label_and_prop) {
    const auto &label = label_and_prop.first;
    const auto &prop = label_and_prop.second;
    const auto &prop_attrs = prop.get_attributes();
    if (boost::equals(label, VOTABLE_RESOURCE_TABLE_DOT + ATTR_MARKER)) {
        table_element_attributes.insert(prop_attrs.begin(), prop_attrs.end());
        return true;
    }
    if (boost::equals(label, VOTABLE_RESOURCE_DOT + ATTR_MARKER)) {
        resource_element_attributes.insert(prop_attrs.begin(), prop_attrs.end());
        return true;
    }
    if (boost::ends_with(label, ATTR_MARKER)) {
        add_attributes(prop_attrs);
        return true;
    }
    return false;
}

//===========================================================

// JTODO Call this version to add labeled_property to initialized(?) table
// (one with a non-empty <resource_elements_> member).
// Backward compatibility: called by query_server.
void Table::add_labeled_property(
        const std::pair<std::string, Property> &label_and_prop) {
    const auto &label = label_and_prop.first;
    const auto &prop = label_and_prop.second;
    if (boost::equals(label, COOSYS) || boost::equals(label, PARAM) ||
        boost::equals(label, INFO)) {
        get_labeled_properties().emplace_back(label_and_prop);
        return;
    }
    if (get_resource_elements().empty()) {
        throw std::runtime_error(
                "This function cannot be used to add resource-level properties to "
                "empty table.");
    }
    if (boost::starts_with(label, VOTABLE_RESOURCE_DOT)) {
        add_resource_element_labeled_property(label.substr(VOTABLE_RESOURCE_DOT.size()),
                                              prop);
    } else {
        add_resource_element_labeled_property(
                "INFO", Property({{ATTR_NAME, label}, {ATTR_VALUE, prop.get_value()}}));
        for (const auto &att : prop.get_attributes()) {
            add_resource_element_labeled_property(
                    "INFO",
                    Property({{ATTR_NAME, att.first}, {ATTR_VALUE, att.second}}));
        }
    }
}

//===========================================================

// This version is to be called e.g. by a read_XXX() function at a time when
// Table's <resource_elements_> vector is still empty.
void Table::add_element_labeled_property(
        std::vector<std::pair<std::string, Property>> &resource_labeled_properties,
        const std::pair<std::string, Property> &label_and_prop) {
    const auto &label = label_and_prop.first;
    const auto &prop = label_and_prop.second;
    if (boost::equals(label, COOSYS) || boost::equals(label, PARAM) ||
        boost::equals(label, INFO)) {
        get_labeled_properties().emplace_back(label_and_prop);
    } else if (boost::starts_with(
                       label,
                       VOTABLE_RESOURCE_DOT)) {  // Backward compatibility w/qs
        resource_labeled_properties.emplace_back(
                label.substr(VOTABLE_RESOURCE_DOT.size()), prop);
    } else if (boost::starts_with(label, RESOURCE_DOT)) {
        resource_labeled_properties.emplace_back(label.substr(RESOURCE_DOT.size()),
                                                 prop);
    } else {
        resource_labeled_properties.emplace_back(
                INFO, Property({{ATTR_NAME, label}, {ATTR_VALUE, prop.get_value()}}));
        for (const auto &att : prop.get_attributes()) {
            resource_labeled_properties.emplace_back(
                    INFO, Property({{ATTR_NAME, att.first}, {ATTR_VALUE, att.second}}));
        }
    }
}


};  // namespace tablator
