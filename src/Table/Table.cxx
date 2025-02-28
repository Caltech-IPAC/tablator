#include "../Table.hxx"

#include <fstream>

#include "../ptree_readers.hxx"

namespace {
void append_info_list_with_label(tablator::Labeled_Properties &combined_list,
                                 const std::vector<tablator::Property> &info_list,
                                 const std::string &label) {
    std::transform(info_list.begin(), info_list.end(),
                   std::back_inserter(combined_list),
                   [&](const tablator::Property &prop) -> tablator::Labeled_Property {
                       return tablator::Labeled_Property(label, prop);
                   });
}

void append_attributes_with_label(tablator::Labeled_Properties &combined_list,
                                  const tablator::ATTRIBUTES &attrs,
                                  const std::string &label) {
    if (attrs.empty()) {
        return;
    }
    combined_list.emplace_back(label, attrs);
}

void append_column_attributes_with_label(
        tablator::Labeled_Properties &combined_list,
        const std::vector<tablator::Column> &column_list,
        const std::string &partial_label, bool is_param) {
    for (const auto &column : column_list) {
        auto attrs = column.get_field_properties().get_attributes();

        if (is_param) {
            std::ostringstream type_os;
            type_os << column.get_type();
            attrs.emplace(std::make_pair("datatype", type_os.str()));

            attrs.emplace(std::make_pair("arraysize",
                                         std::to_string(column.get_array_size())));
        }

        if (attrs.empty()) {
            continue;
        }

        combined_list.emplace_back(partial_label + column.get_name(), attrs);
    }
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
             const Labeled_Properties &property_pair_vec)
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

const Labeled_Properties Table::combine_trailing_info_lists_all_levels() const {
    Labeled_Properties combined_list;
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

// Called by write_fits().
const Labeled_Properties Table::combine_attributes_all_levels(
        bool include_column_attributes_f) const {
    Labeled_Properties combined_list;
    append_attributes_with_label(combined_list, get_attributes(), VOTABLE_XMLATTR);
    append_attributes_with_label(combined_list,
                                 get_results_resource_element().get_attributes(),
                                 VOTABLE_RESOURCE_XMLATTR);
    append_attributes_with_label(combined_list,
                                 get_main_table_element().get_attributes(),
                                 VOTABLE_RESOURCE_TABLE_XMLATTR);

    if (include_column_attributes_f) {
        append_column_attributes_with_label(combined_list, get_table_element_params(),
                                            VOTABLE_RESOURCE_TABLE_PARAM_DOT,
                                            true /* is_param */);

        append_column_attributes_with_label(combined_list, get_table_element_fields(),
                                            VOTABLE_RESOURCE_TABLE_FIELD_DOT,
                                            false /* is_param */);
    }

    return combined_list;
}


//===========================================================

const Labeled_Properties Table::combine_labeled_properties_all_levels() const {
    Labeled_Properties combined_labeled_props(get_labeled_properties());
    const auto &resource_labeled_props = get_resource_element_labeled_properties();
    std::transform(resource_labeled_props.begin(), resource_labeled_props.end(),
                   std::back_inserter(combined_labeled_props),
                   [](const Labeled_Property &label_and_prop) -> Labeled_Property {
                       return Labeled_Property(
                               VOTABLE_RESOURCE_DOT + label_and_prop.first,
                               label_and_prop.second);
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

void Table::distribute_metadata(Labeled_Properties &resource_element_labeled_properties,
                                std::vector<Property> &resource_element_trailing_infos,
                                ATTRIBUTES &resource_element_attributes,
                                std::vector<Property> &table_element_trailing_infos,
                                ATTRIBUTES &table_element_attributes,
                                const Labeled_Properties &label_prop_pairs) {
    for (const auto &label_and_prop : label_prop_pairs) {
        if (label_and_prop.second.empty()) {
            continue;
        }
        if (stash_trailing_info_labeled_by_element(resource_element_trailing_infos,
                                                   table_element_trailing_infos,
                                                   label_and_prop)) {
            continue;
        }
        if (stash_attributes_labeled_by_element(resource_element_attributes,
                                                table_element_attributes,
                                                label_and_prop)) {
            continue;
        }
        stash_resource_element_labeled_property(resource_element_labeled_properties,
                                                label_and_prop);
    }
}


//=============================================

bool Table::stash_trailing_info_labeled_by_element(
        std::vector<Property> &resource_element_infos,
        std::vector<Property> &table_element_infos,
        const Labeled_Property &label_and_prop) {
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

bool Table::stash_attributes_labeled_by_element(
        ATTRIBUTES &resource_element_attributes, ATTRIBUTES &table_element_attributes,
        const Labeled_Property &label_and_prop) {
    const auto &label = label_and_prop.first;
    const auto &prop = label_and_prop.second;
    const auto &prop_attrs = prop.get_attributes();
    if (boost::equals(label, VOTABLE_RESOURCE_TABLE_XMLATTR)) {
        table_element_attributes.insert(prop_attrs.begin(), prop_attrs.end());
        return true;
    }
    if (boost::equals(label, VOTABLE_RESOURCE_XMLATTR)) {
        resource_element_attributes.insert(prop_attrs.begin(), prop_attrs.end());
        return true;
    } else if (boost::starts_with(label, VOTABLE_RESOURCE_DOT)) {
        // e.g. FIELD or PARAM. Attributes of these elements are
        // handled in Column code.
        return false;
    }
    if (boost::ends_with(label, XMLATTR)) {
        add_attributes(prop_attrs);
        return true;
    }
    return false;
}

//===========================================================

// Call this version to add labeled_property to initialized(?) table
// (one with a non-empty <resource_elements_> member).
void Table::add_labeled_property(const Labeled_Property &label_and_prop) {
    const auto &label = label_and_prop.first;
    const auto &prop = label_and_prop.second;
    if (is_property_style_label(label)) {
        get_labeled_properties().emplace_back(label_and_prop);
        return;
    }
    if (get_resource_elements().empty()) {
        throw std::runtime_error(
                "This function cannot be used to add resource-level properties to "
                "embryonic table (one without resource_elements).");
    }

    if (boost::starts_with(label, VOTABLE_RESOURCE_DOT)) {
        add_resource_element_labeled_property(label.substr(VOTABLE_RESOURCE_DOT.size()),
                                              prop);
    } else {
        add_resource_element_labeled_property(label_and_prop);
    }
}

//===========================================================

// This version is to be called e.g. by a read_XXX() function at a time when
// Table's <resource_elements_> vector is still empty.
void Table::stash_resource_element_labeled_property(
        Labeled_Properties &resource_labeled_properties,
        const Labeled_Property &label_and_prop) {
    const auto &label = label_and_prop.first;
    const auto &prop = label_and_prop.second;

    if (is_property_style_label(label)) {
        get_labeled_properties().emplace_back(label_and_prop);
    } else if (boost::starts_with(label, VOTABLE_RESOURCE_DOT)) {
        resource_labeled_properties.emplace_back(
                label.substr(VOTABLE_RESOURCE_DOT.size()), prop);
    } else {
        resource_labeled_properties.emplace_back(label, prop);
    }
}


};  // namespace tablator
