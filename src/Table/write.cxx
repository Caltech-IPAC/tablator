#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>

#include "../Data_Type_Adjuster.hxx"
#include "../Table.hxx"

void tablator::Table::write(const boost::filesystem::path &path,
                            const Format &format) const {
    const bool use_stdout(path.string() == "-");
    if (format.is_fits()) {
        // List of cols whose types must be adjusted for writing due
        // to restrictions from <format>.
        std::vector<Data_Type> datatypes_for_writing =
                Data_Type_Adjuster(*this).get_datatypes_for_writing(format.enum_format);
        write_fits(path, datatypes_for_writing);
    } else if (format.is_hdf5()) {
        if (use_stdout) {
            write_hdf5(std::cout);
        } else {
            write_hdf5(path);
        }
    } else if (format.is_sqlite_db()) {
        write_sqlite_db(path);
    } else if (use_stdout) {
        write(std::cout, "stdout", format);
    } else {
        boost::filesystem::ofstream file_output;
        file_output.open(path);
        write(file_output, path.stem().native(), format);
    }
}

void tablator::Table::write(std::ostream &os, const std::string &table_name,
                            const Format &format) const {
    // List of cols whose types must be adjusted for writing due
    // to restrictions from <format>.
    std::vector<Data_Type> datatypes_for_writing;

    switch (format.enum_format) {
        case Format::Enums::FITS:
            datatypes_for_writing = Data_Type_Adjuster(*this).get_datatypes_for_writing(
                    format.enum_format);
            write_fits(os, datatypes_for_writing);
            break;
        case Format::Enums::HDF5:
            write_hdf5(os);
            break;
        case Format::Enums::JSON:
        case Format::Enums::JSON5:
        case Format::Enums::VOTABLE: {
            datatypes_for_writing = Data_Type_Adjuster(*this).get_datatypes_for_writing(
                    format.enum_format);
            std::string tabledata_string(
                    boost::uuids::to_string(boost::uuids::random_generator()()));
            boost::property_tree::ptree tree(
                    generate_property_tree(tabledata_string, datatypes_for_writing));
            std::stringstream ss;
            bool is_json(format.enum_format != Format::Enums::VOTABLE);
            if (is_json) {
                boost::property_tree::write_json(ss, tree, true);
            } else {
                boost::property_tree::write_xml(
                        ss, tree,
                        boost::property_tree::xml_writer_make_settings(' ', 2));
            }
            std::string s(ss.str());
            size_t tabledata_offset(s.find(tabledata_string));
            os << s.substr(0, tabledata_offset - (is_json ? 2 : 0));
            write_tabledata(os, format.enum_format);
            os << s.substr(tabledata_offset + tabledata_string.size() +
                           (is_json ? 2 : 0));
        } break;
        case Format::Enums::CSV:
            write_dsv(os, ',');
            break;
        case Format::Enums::TSV:
            write_dsv(os, '\t');
            break;
        case Format::Enums::IPAC_TABLE:
        case Format::Enums::TEXT:
            write_ipac_table(os);
            break;
        case Format::Enums::HTML:
            write_html(os);
            break;
        case Format::Enums::POSTGRES_SQL:
        case Format::Enums::ORACLE_SQL:
        case Format::Enums::SQLITE_SQL:
            write_sql(os, table_name, format.enum_format);
            break;
        case Format::Enums::SQLITE_DB:
            throw std::runtime_error("SQLITE_DB output to a stream not implemented");
            break;
        case Format::Enums::UNKNOWN:
        default:
            throw std::runtime_error("Unknown output type");
            break;
    }
}
