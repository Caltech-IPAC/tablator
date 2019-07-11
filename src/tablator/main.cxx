/// A simple converter program to test out the tablator library.

#include <json5_parser.h>
#include <CCfits/CCfits>
#include <boost/program_options.hpp>
#include <iostream>
#include <sstream>

#include "../Table.hxx"

std::string usage(const boost::program_options::options_description &visible_options) {
    std::stringstream ss;
    ss << "Usage: tablator [options] input_file output_file\n" << visible_options;
    return ss.str();
}

int main(int argc, char *argv[]) {
    bool stream_intermediate(false);
    tablator::Format input_format, output_format;
    size_t start_row = 0;
    size_t row_count = 0;
    std::vector<size_t> row_list;
    std::string row_string;
    bool call_static = false;
    std::string input_format_str, output_format_str, col_name;

    // Declare the supported options.
    boost::program_options::options_description visible_options("Options");
    visible_options.add_options()("help", "produce help message")(
            "stream-intermediate",
            boost::program_options::value<bool>(&stream_intermediate),
            "stream the intermediate file (for testing only)")(
            "input-format",
            boost::program_options::value<std::string>(&input_format_str),
            "Input file format (json,json5,votable,csv,tsv,fits,ipac_table,"
            "text,html,hdf5)")(
            "output-format",
            boost::program_options::value<std::string>(&output_format_str),
            "Output file format (json,json5,votable,csv,tsv,fits,ipac_table,"
            "text,html,hdf5)")("start-row",
                               boost::program_options::value<size_t>(&start_row),
                               "first row to write (output-format ipac_table only)")(
            "row-count", boost::program_options::value<size_t>(&row_count),
            "number of consecutive rows to write (output-format ipac_table only)")(
            "row-list", boost::program_options::value<std::string>(&row_string),
            "list of rows to write (output-format ipac_table only)")(
            "static", "call static function, not Table class member")(
            "col-name", boost::program_options::value<std::string>(&col_name),
            "name of column whose values to extract into vector");

    boost::program_options::options_description hidden_options("Hidden options");
    hidden_options.add_options()(
            "files", boost::program_options::value<std::vector<std::string> >(),
            "input and output file");

    boost::program_options::options_description options;
    options.add(visible_options).add(hidden_options);
    boost::program_options::positional_options_description positional_options;
    positional_options.add("files", -1);

    boost::program_options::variables_map option_variables;
    try {
        boost::program_options::store(
                boost::program_options::command_line_parser(argc, argv)
                        .options(options)
                        .positional(positional_options)
                        .run(),
                option_variables);
        boost::program_options::notify(option_variables);

        if (option_variables.count("help")) {
            std::cout << usage(visible_options) << "\n";
            return 1;
        }
        if (option_variables.count("input-format")) input_format.init(input_format_str);
        if (option_variables.count("output-format"))
            output_format.init(output_format_str);
        if (option_variables.count("start-row")) {
            if (!option_variables.count("row-count")) {
                row_count = 1;
            }
        }
        if (option_variables.count("row-list")) {
            std::stringstream row_stream(row_string);
            std::vector<size_t> temp_list((std::istream_iterator<size_t>(row_stream)),
                                          std::istream_iterator<size_t>());
            row_list.swap(temp_list);
        }

        // Validation
        boost::filesystem::path input_path, output_path;
        if (option_variables.count("files")) {
            std::vector<std::string> paths(
                    option_variables["files"].as<std::vector<std::string> >());
            if (paths.size() == 1) {
                std::cerr << "Missing an output file\n";
                return 1;
            }
            if (paths.size() != 2) {
                std::cerr << "Too many filenames\n";
                return 1;
            }
            input_path = paths.at(0);
            output_path = paths.at(1);
            if (input_format.is_unknown()) input_format.set_from_extension(input_path);
            if (output_format.is_unknown())
                output_format.set_from_extension(output_path);
        } else {
            std::cerr << "Missing an input and output file\n"
                      << usage(visible_options) << "\n";
            return 1;
        }

        bool do_subtable = ((option_variables.count("start-row") > 0) ||
                            (option_variables.count("row-list") > 0));

        if (do_subtable &&
            (output_format.enum_format != tablator::Format::Enums::IPAC_TABLE)) {
            std::cerr << "Subtable feature requested via 'start-row' and 'row-list' "
                         "options "
                      << "is supported only when output-format is ipac_table.\n";
            return 1;
        }

        if (do_subtable && !col_name.empty()) {
            std::cerr << "Subtable feature is incompatible with extract-column-values "
                         "feature.\n";
            return 1;
        }


        // Do it!
        if (!col_name.empty()) {
            boost::filesystem::ifstream input_stream(input_path);
            tablator::Table table(input_stream, input_format);
            std::vector<std::string> col_values =
                    table.extract_column_values_as_strings(col_name);
            boost::filesystem::ofstream output_stream(output_path);
            std::copy(col_values.begin(), col_values.end(),
                      std::ostream_iterator<std::string>(output_stream, " "));
        } else if (do_subtable) {
            boost::filesystem::ifstream input_stream(input_path);
            tablator::Table table(input_stream, input_format);
            boost::filesystem::ofstream output_stream(output_path);
            if (option_variables.count("start-row")) {
                if (call_static) {
                    tablator::Ipac_Table_Writer::write_ipac_subtable_by_row(
                            table, output_stream, start_row, row_count);
                } else {
                    table.write_ipac_subtable_by_row(output_stream, start_row,
                                                     row_count);
                }
            } else {
                if (call_static) {
                    tablator::Ipac_Table_Writer::write_ipac_subtable_by_row(
                            table, output_stream, row_list);
                } else {
                    table.write_ipac_subtable_by_row(output_stream, row_list);
                }
            }
        } else if (stream_intermediate) {
            boost::filesystem::ifstream input_stream(input_path);
            tablator::Table table(input_stream, input_format);
            boost::filesystem::ofstream output_stream(output_path);
            table.write(output_stream, output_path.stem().native(), output_format);
        } else {
            tablator::Table table(input_path, input_format);
            table.write(output_path, output_format);
        }
    } catch (boost::program_options::error &exception) {
        std::cerr << exception.what() << "\n" << usage(visible_options);
        std::cerr.flush();
        exit(1);
    } catch (CCfits::FitsException &exception) {
        std::cerr << exception.message() << "\n";
        std::cerr.flush();
        exit(1);
    } catch (H5::Exception &exception) {
        std::cerr << "In " << exception.getFuncName() << ": "
                  << exception.getDetailMsg() << "\n";
        std::cerr.flush();
        exit(1);
    } catch (json5_parser::Error_position &exception) {
        std::cerr << "On line " << exception.line_ << ", column " << exception.column_
                  << ": " << exception.reason_ << "\n";
        std::cerr.flush();
        exit(1);
    } catch (std::exception &exception) {
        std::cerr << exception.what() << "\n";
        std::cerr.flush();
        exit(1);
    }
}
