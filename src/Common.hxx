#pragma once

#include <algorithm>
#include <map>
#include <string>
#include <vector>

namespace tablator {

// JTODO make these constexpr.

static const std::string DESCRIPTION("DESCRIPTION");
static const std::string FIELD("FIELD");
static const std::string PARAM("PARAM");

static const std::string ATTR_NAME("name");
static const std::string ATTR_IRSA_VALUE("irsa_value");
static const std::string ATTR_UCD("ucd");
static const std::string ATTR_VALUE("value");

static const std::string XMLATTR("<xmlattr>");
static const std::string XMLATTR_DOT("<xmlattr>.");

static const std::string XMLATTR_ARRAYSIZE("<xmlattr>.arraysize");
static const std::string XMLATTR_DATATYPE("<xmlattr>.datatype");
static const std::string XMLATTR_ENCODING("<xmlattr>.encoding");
static const std::string XMLATTR_ID("<xmlattr>.ID");
static const std::string XMLATTR_INCLUSIVE("<xmlattr>.inclusive");
static const std::string XMLATTR_NAME("<xmlattr>.name");
static const std::string XMLATTR_REF("<xmlattr>.ref");
static const std::string XMLATTR_TYPE("<xmlattr>.type");
static const std::string XMLATTR_VALUE("<xmlattr>.value");

static const std::string XMLCOMMENT("<xmlcomment>");

// For internal use with hdf5 (JTODO and eventually fits?)
static const std::string END_INFO_MARKER("END_INFO_MARKER");

static const std::string VOTABLE_KEYWORD_HEAD("VOKW");
static const std::string LABEL_END_MARKER("<VOEND>");

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
static const std::string PARAMREF("PARAMref");
static const std::string QUERY_STATUS("QUERY_STATUS");
static const std::string REF("ref");
static const std::string RESOURCE("RESOURCE");
static const std::string TABLE("TABLE");
static const std::string TABLEDATA("TABLEDATA");
static const std::string TIMESYS("TIMESYS");
static const std::string TYPE("type");
static const std::string UNIT("unit");
static const std::string UTYPE("utype");
static const std::string VALUE("VALUE");
static const std::string VALUES("VALUES");
static const std::string VOTABLE("VOTABLE");

static const std::string DOT(".");
static const std::string VOTABLE_DOT(VOTABLE + DOT);
static const std::string RESOURCE_DOT(RESOURCE + DOT);
static const std::string TABLE_DOT(TABLE + DOT);
static const std::string INFO_DOT(INFO + DOT);

static const std::string VOTABLE_RESOURCE(VOTABLE_DOT + RESOURCE);
static const std::string VOTABLE_RESOURCE_DOT(VOTABLE_DOT + RESOURCE_DOT);

static const std::string VOTABLE_RESOURCE_INFO(VOTABLE_RESOURCE_DOT + INFO);
static const std::string VOTABLE_RESOURCE_INFO_DOT(VOTABLE_RESOURCE_DOT + INFO_DOT);

static const std::string VOTABLE_RESOURCE_TABLE(VOTABLE_RESOURCE_DOT + TABLE);
static const std::string VOTABLE_RESOURCE_TABLE_DOT(VOTABLE_RESOURCE_DOT + TABLE_DOT);

static const std::string VOTABLE_RESOURCE_TABLE_INFO(VOTABLE_RESOURCE_TABLE_DOT + INFO);
static const std::string VOTABLE_RESOURCE_TABLE_INFO_DOT(VOTABLE_RESOURCE_TABLE_DOT +
                                                         INFO_DOT);

static const std::string VOTABLE_XMLATTR(VOTABLE_DOT + XMLATTR);
static const std::string VOTABLE_RESOURCE_XMLATTR(VOTABLE_RESOURCE_DOT + XMLATTR);
static const std::string VOTABLE_RESOURCE_INFO_XMLATTR(VOTABLE_RESOURCE_INFO_DOT +
                                                       XMLATTR);
static const std::string VOTABLE_RESOURCE_TABLE_XMLATTR(VOTABLE_RESOURCE_TABLE_DOT +
                                                        XMLATTR);
static const std::string VOTABLE_RESOURCE_TABLE_INFO_XMLATTR(
        VOTABLE_RESOURCE_TABLE_INFO_DOT + XMLATTR);

static const std::string VOTABLE_RESOURCE_XMLATTR_DOT(VOTABLE_RESOURCE_XMLATTR + DOT);


static const std::string VOTABLE_RESOURCE_TABLE_XMLATTR_DOT(
        VOTABLE_RESOURCE_TABLE_XMLATTR + DOT);

static const std::string VOTABLE_RESOURCE_INFO_XMLATTR_DOT(VOTABLE_RESOURCE_INFO_DOT +
                                                           XMLATTR_DOT);
static const std::string VOTABLE_RESOURCE_TABLE_INFO_XMLATTR_DOT(
        VOTABLE_RESOURCE_TABLE_INFO_DOT + XMLATTR_DOT);


static const std::string RESOURCE_ELEMENT_DESCRIPTION(RESOURCE_DOT + DESCRIPTION);
static const std::string TABLE_ELEMENT_DESCRIPTION(TABLE_DOT + DESCRIPTION);
static const std::string DATA_TABLEDATA(DATA + DOT + TABLEDATA);

static const std::string TABLEDATA_PLACEHOLDER("TaBlEdAtA_PlAcEhOlDeR");

static const std::string ARRAY_TAIL("_ARRAY");

static const std::string FIELD_ARRAY(FIELD + ARRAY_TAIL);
static const std::string FIELDREF_ARRAY(FIELDREF + ARRAY_TAIL);
static const std::string GROUP_ARRAY(GROUP + ARRAY_TAIL);
static const std::string INFO_ARRAY(INFO + ARRAY_TAIL);
static const std::string LINK_ARRAY(LINK + ARRAY_TAIL);
static const std::string PARAM_ARRAY(PARAM + ARRAY_TAIL);
static const std::string PARAMREF_ARRAY(PARAMREF + ARRAY_TAIL);
static const std::string RESOURCE_ARRAY(RESOURCE + ARRAY_TAIL);

static const std::string HDF5_LINK("hdf5.link.");

static constexpr size_t PLACEHOLDER_LEFT_MARGIN = 4;
static constexpr size_t PLACEHOLDER_RIGHT_MARGIN = 5;

// No complicated sub-elements, only attributes and/or string value.
static std::vector<std::string> PROPERTY_STYLE_LABELS = {COOSYS, INFO, TIMESYS, LINK};
inline bool is_property_style_label(const std::string &label) {
    return (std::find(PROPERTY_STYLE_LABELS.begin(), PROPERTY_STYLE_LABELS.end(),
                      label) != PROPERTY_STYLE_LABELS.end());
}

typedef std::map<std::string, std::string> ATTRIBUTES;
typedef std::pair<std::string, std::string> STRING_PAIR;
}  // namespace tablator
