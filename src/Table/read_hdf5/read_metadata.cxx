#include "../../H5_to_Data_Type.hxx"
#include "../../Table.hxx"
#include "../HDF5_Attribute.hxx"
#include "../HDF5_Property.hxx"

namespace tablator {
std::vector<Labeled_Property> read_metadata(const H5::DataSet &dataset) {
    std::vector<Labeled_Property> result;
    if (dataset.attrExists("METADATA")) {
        auto metadata = dataset.openAttribute("METADATA");

        if (metadata.getTypeClass() == H5T_VLEN) {
            H5::VarLenType hdf5_properties_type = metadata.getVarLenType();
            H5::DataType hdf5_properties_super = hdf5_properties_type.getSuper();
            if (hdf5_properties_super.getClass() == H5T_COMPOUND) {
                H5::CompType hdf5_property_type(hdf5_properties_super.getId());
                if (hdf5_property_type.getNmembers() == 3 &&
                    hdf5_property_type.getMemberClass(0) == H5T_STRING &&
                    hdf5_property_type.getMemberClass(1) == H5T_STRING &&
                    hdf5_property_type.getMemberClass(2) == H5T_VLEN) {
                    H5::VarLenType attributes_type =
                            hdf5_property_type.getMemberVarLenType(2);
                    H5::DataType attributes_super = attributes_type.getSuper();
                    if (attributes_super.getClass() == H5T_COMPOUND) {
                        H5::CompType attribute_type(attributes_super.getId());
                        if (attribute_type.getNmembers() == 2 &&
                            attribute_type.getMemberClass(0) == H5T_STRING &&
                            attribute_type.getMemberClass(1) == H5T_STRING) {
                            hvl_t hdf5_props;
                            metadata.read(hdf5_properties_type, &hdf5_props);
                            for (size_t n = 0; n < hdf5_props.len; ++n) {
                                HDF5_Property &hdf5_property =
                                        reinterpret_cast<HDF5_Property *>(
                                                hdf5_props.p)[n];
                                Property prop(hdf5_property.value);
                                for (size_t i = 0; i < hdf5_property.attributes.len;
                                     ++i) {
                                    HDF5_Attribute &hdf_attribute =
                                            reinterpret_cast<HDF5_Attribute *>(
                                                    hdf5_property.attributes.p)[i];
                                    prop.add_attribute(
                                            std::string(hdf_attribute.name),
                                            std::string(hdf_attribute.value));
                                }
                                result.push_back(std::make_pair(
                                        std::string(hdf5_property.name), prop));
                            }
                        }
                    }
                }
            }
        }
    }
    return result;
}
}  // namespace tablator
