#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>

#include "../Data_Type_Adjuster.hxx"
#include "../Table.hxx"

void tablator::Table::write(const boost::filesystem::path &path, const Format &format,
                            const Command_Line_Options &options) const {
  // std::cout << "write(), path, enter" << std::endl;
    const bool use_stdout(path.string() == "-");
	// std::cout << "before format.is_fits()" << std::endl;
    if (format.is_fits()) {
	// std::cout << "format.is_fits()" << std::endl;
        // List of cols whose types must be adjusted for writing due
        // to restrictions from <format>.
        std::vector<Data_Type> datatypes_for_writing =
                Data_Type_Adjuster(*this).get_datatypes_for_writing(format.enum_format);
        write_fits(path, datatypes_for_writing);
    } else if (format.is_hdf5()) {
	// std::cout << "format.is_hdf5()" << std::endl;
        if (use_stdout) {
            write_hdf5(std::cout);
        } else {
	// std::cout << "format.is_hdf5(), path" << std::endl;
            write_hdf5(path);
        }
    } else if (format.is_sqlite_db()) {
        write_sqlite_db(path, options);
    } else if (use_stdout) {
        write(std::cout, "stdout", format, options);
    } else {
        boost::filesystem::ofstream file_output;
        file_output.open(path);
        write(file_output, path.stem().native(), format, options);
    }
}

void tablator::Table::write(std::ostream &os, const std::string &table_name,
                            const Format &format,
                            const Command_Line_Options &options) const {

#if 0
	std::ofstream debug_stream;
	std::string debug_file =  "/home/judith/repos/tablator/bin2/tablator/debug_write.txt";
	  debug_stream.open(debug_file.c_str());
	
	  debug_stream << "write(), enter" << std::endl << std::flush;
#endif


  // std::cout << "write(), enter, stream" << std::endl;
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
        case Format::Enums::VOTABLE:
        case Format::Enums::VOTABLE_BINARY2: {
		  // std::cout << "write(), before json/votable" << std::endl;
		  // debug_stream << "write(), before json/votable" << std::endl;
            datatypes_for_writing = Data_Type_Adjuster(*this).get_datatypes_for_writing(
                    format.enum_format);
            bool is_json = (format.enum_format == Format::Enums::JSON ||
                            format.enum_format == Format::Enums::JSON5);
            bool do_binary2 = (format.enum_format == Format::Enums::VOTABLE_BINARY2);
		  // debug_stream << "write(), before generate_property_tree()" << std::endl;
			// std::cout << "write(), before generate_property_tree()" << std::endl;
            boost::property_tree::ptree tree(
                    generate_property_tree(datatypes_for_writing, is_json, do_binary2));
		  // debug_stream << "write(), after generate_property_tree()" << std::endl;
			// std::cout << "write(), after generate_property_tree()" << std::endl;
            std::stringstream ss;

            if (is_json) {
                boost::property_tree::write_json(ss, tree, true);
            } else {
                boost::property_tree::write_xml(
                        ss, tree,
                        boost::property_tree::xml_writer_make_settings(' ', 2));
            }
            uint num_spaces = is_json ? 2 : 0;

            if (do_binary2) {
                // Command_Line_Options are not relevant in this case.
		  // debug_stream << "write(), before splice_binary2()" << std::endl;
                splice_binary2_and_write(os, ss, format.enum_format, num_spaces,
                                         num_spaces);
            } else {
		  // debug_stream << "write(), before splice_tabledata()" << std::endl;
			  // std::cout << "write(), before splice_tabledata()" << std::endl;
                splice_tabledata_and_write(os, ss, format.enum_format, num_spaces,
                                           num_spaces, options);
            }
		  // debug_stream << "write(), after json/votable" << std::endl;
			// std::cout << "write(), after json/votable" << std::endl;
        } break;
        case Format::Enums::CSV:
            write_dsv(os, ',', options);
            break;
        case Format::Enums::TSV:
            write_dsv(os, '\t', options);
            break;
        case Format::Enums::IPAC_TABLE:
        case Format::Enums::TEXT:
		  // std::cout << "write(), before write_ipac_table()" << std::endl;
		  // debug_stream << "write(), before write_ipac_table()" << std::endl;
            write_ipac_table(os, options);
		  // std::cout << "write(), after write_ipac_table()" << std::endl;
		 // debug_stream << "write(), after write_ipac_table()" << std::endl;
            break;
        case Format::Enums::HTML:
            write_html(os, options);
            break;
        case Format::Enums::POSTGRES_SQL:
        case Format::Enums::ORACLE_SQL:
        case Format::Enums::SQLITE_SQL:
            write_sql(os, table_name, format.enum_format, options);
            break;
        case Format::Enums::SQLITE_DB:
            throw std::runtime_error("SQLITE_DB output to a stream not implemented");
            break;
        case Format::Enums::UNKNOWN:
        default:
            throw std::runtime_error("Unknown output type");
            break;
    }
   // debug_stream << "write(), leaving" << std::endl << std::flush;
}
