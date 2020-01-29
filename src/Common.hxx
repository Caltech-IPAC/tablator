#pragma once

#include <algorithm>
#include <map>
#include <string>
#include <vector>

namespace tablator {

static const std::string DESCRIPTION("DESCRIPTION");
static const std::string FIELD("FIELD");
static const std::string PARAM("PARAM");

static const std::string ATTR_NAME("name");
static const std::string ATTR_VALUE("value");

static const std::string XMLATTR("<xmlattr>");
static const std::string XMLATTR_DOT("<xmlattr>.");
static const std::string XMLCOMMENT("<xmlcomment>");

static const std::string VOTABLE_DOT("VOTABLE.");
static const std::string RESOURCE_DOT("RESOURCE.");

static const std::string VOTABLE_RESOURCE_DOT("VOTABLE.RESOURCE.");
static const std::string VOTABLE_RESOURCE_TABLE_DOT("VOTABLE.RESOURCE.TABLE.");


// For internal use with hdf5 (JTODO and eventually fits?)
static const std::string ATTR_MARKER("ATTR_MARKER");
static const std::string END_INFO_MARKER("END_INFO_MARKER");

// JTODO "arraysize" vs. "array_size" (Column, hdf5)
// lower case vs. upper case
static const std::string ARRAYSIZE("arraysize");

static const std::string BINARY("BINARY");
static const std::string BINARY2("BINARY2");
static const std::string DATA("DATA");
static const std::string DATATYPE("datatype");
static const std::string COOSYS("COOSYS");
static const std::string DEFINITIONS("DEFINITIONS");
static const std::string FIELDREF("FIELDref");
static const std::string FITS("FITS");
static const std::string GROUP("GROUP");
static const std::string ID("ID");
static const std::string INFO("INFO");
static const std::string LINK("LINK");
static const std::string MIN("MIN");
static const std::string MAX("MAX");
static const std::string NAME("name");
static const std::string OPTION("OPTION");
static const std::string QUERY_STATUS("QUERY_STATUS");
static const std::string REF("ref");
static const std::string RESOURCE("RESOURCE");
static const std::string TABLE("TABLE");
static const std::string TABLEDATA("TABLEDATA");
static const std::string TYPE("type");
static const std::string UNIT("unit");
static const std::string UTYPE("utype");
static const std::string VALUES("VALUES");
static const std::string VOTABLE("VOTABLE");

static const std::string RESOURCE_ELEMENT_DESCRIPTION("RESOURCE.DESCRIPTION");
static const std::string TABLE_ELEMENT_DESCRIPTION("RESOURCE.TABLE.DESCRIPTION");
static const std::string DATA_TABLEDATA("DATA.TABLEDATA");
static const std::string TABLEDATA_PLACEHOLDER("TaBlEdAtA_PlAcEhOlDeR");
static constexpr size_t PLACEHOLDER_LEFT_MARGIN = 4;
static constexpr size_t PLACEHOLDER_RIGHT_MARGIN = 5;
static constexpr size_t TABLE_RESOURCE_IDX = 0;

static std::vector<std::string> PROPERTY_STYLE_LABELS = {COOSYS, GROUP, PARAM, INFO};
inline bool is_property_style_label(const std::string &label) {
    return (std::find(PROPERTY_STYLE_LABELS.begin(), PROPERTY_STYLE_LABELS.end(),
                      label) != PROPERTY_STYLE_LABELS.end());
}

typedef std::map<std::string, std::string> ATTRIBUTES;
}  // namespace tablator
