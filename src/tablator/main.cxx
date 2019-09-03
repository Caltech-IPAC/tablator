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

// JNOTE These handle_XXX() functions are useful only for testing.

void handle_extract_single_value(const boost::filesystem::path &input_path,
                                 const tablator::Format &input_format,
                                 const boost::filesystem::path &output_path,
                                 const std::string &column_name,
                                 const std::string &type_str, size_t row_id) {
    boost::filesystem::ifstream input_stream(input_path);
    tablator::Table table(input_stream, input_format);
    boost::filesystem::ofstream output_stream(output_path);

    if (boost::iequals(type_str, "INT8_LE")) {
        auto val_array_vec = table.extract_value<int8_t>(column_name, row_id);
        std::copy(val_array_vec.begin(), val_array_vec.end(),
                  std::ostream_iterator<int8_t>(output_stream, " "));
    } else if (boost::iequals(type_str, "UINT8_LE")) {
        auto val_array_vec = table.extract_value<uint8_t>(column_name, row_id);
        std::copy(val_array_vec.begin(), val_array_vec.end(),
                  std::ostream_iterator<uint8_t>(output_stream, " "));
    } else if (boost::iequals(type_str, "INT16_LE")) {
        auto val_array_vec = table.extract_value<int16_t>(column_name, row_id);
        std::copy(val_array_vec.begin(), val_array_vec.end(),
                  std::ostream_iterator<int16_t>(output_stream, " "));
    } else if (boost::iequals(type_str, "UINT16_LE")) {
        auto val_array_vec = table.extract_value<uint16_t>(column_name, row_id);
        std::copy(val_array_vec.begin(), val_array_vec.end(),
                  std::ostream_iterator<uint16_t>(output_stream, " "));
    } else if (boost::iequals(type_str, "INT32_LE")) {
        auto val_array_vec = table.extract_value<int32_t>(column_name, row_id);
        std::copy(val_array_vec.begin(), val_array_vec.end(),
                  std::ostream_iterator<int32_t>(output_stream, " "));
    } else if (boost::iequals(type_str, "UINT32_LE")) {
        auto val_array_vec = table.extract_value<uint32_t>(column_name, row_id);
        std::copy(val_array_vec.begin(), val_array_vec.end(),
                  std::ostream_iterator<uint32_t>(output_stream, " "));
    } else if (boost::iequals(type_str, "INT64_LE")) {
        auto val_array_vec = table.extract_value<int64_t>(column_name, row_id);
        std::copy(val_array_vec.begin(), val_array_vec.end(),
                  std::ostream_iterator<int64_t>(output_stream, " "));
    } else if (boost::iequals(type_str, "UINT64_LE")) {
        auto val_array_vec = table.extract_value<uint64_t>(column_name, row_id);
        std::copy(val_array_vec.begin(), val_array_vec.end(),
                  std::ostream_iterator<uint64_t>(output_stream, " "));
    } else if (boost::iequals(type_str, "FLOAT32_LE")) {
        auto val_array_vec = table.extract_value<float>(column_name, row_id);
        std::copy(val_array_vec.begin(), val_array_vec.end(),
                  std::ostream_iterator<float>(output_stream, " "));
    } else if (boost::iequals(type_str, "FLOAT64_LE")) {
        auto val_array_vec = table.extract_value<double>(column_name, row_id);
        std::copy(val_array_vec.begin(), val_array_vec.end(),
                  std::ostream_iterator<double>(output_stream, " "));
    } else if (boost::iequals(type_str, "char")) {
        std::string msg("extract_value() is not supported for columns of type char; ");
        msg.append("please use extract_value_as_string().\n");
        throw(std::runtime_error(msg));
    }

    else {
        std::string msg("Invalid value of 'type' parameter: ");
        msg.append(type_str);
        throw(std::runtime_error(msg));
    }
}


template <typename T>
void dump_column_vector(boost::filesystem::ofstream &output_stream,
                        const std::vector<T> &val_array_vec, size_t num_rows,
                        size_t array_size) {
    auto vec_iter = val_array_vec.begin();
    auto next_iter = val_array_vec.begin();
    for (size_t i = 0; i < num_rows; ++i) {
        advance(next_iter, array_size);
        std::copy(vec_iter, next_iter, std::ostream_iterator<T>(output_stream, " "));
        output_stream << std::endl;
        vec_iter = next_iter;
    }
}

void handle_extract_column(const boost::filesystem::path &input_path,
                           const tablator::Format &input_format,
                           const boost::filesystem::path &output_path,
                           const std::string &column_name,
                           const std::string &type_str) {
    boost::filesystem::ifstream input_stream(input_path);
    tablator::Table table(input_stream, input_format);
    boost::filesystem::ofstream output_stream(output_path);
    auto column_id = table.column_index(column_name);
    auto array_size = table.columns[column_id].array_size;
    auto num_rows = table.num_rows();

    if (boost::iequals(type_str, "INT8_LE")) {
        auto val_array_vec = table.extract_column<int8_t>(column_id);
        dump_column_vector(output_stream, val_array_vec, num_rows, array_size);
    } else if (boost::iequals(type_str, "UINT8_LE")) {
        auto val_array_vec = table.extract_column<uint8_t>(column_name);
        dump_column_vector(output_stream, val_array_vec, num_rows, array_size);
    } else if (boost::iequals(type_str, "INT16_LE")) {
        auto val_array_vec = table.extract_column<int16_t>(column_name);
        dump_column_vector(output_stream, val_array_vec, num_rows, array_size);
    } else if (boost::iequals(type_str, "UINT16_LE")) {
        auto val_array_vec = table.extract_column<uint16_t>(column_name);
        dump_column_vector(output_stream, val_array_vec, num_rows, array_size);
    } else if (boost::iequals(type_str, "INT32_LE")) {
        auto val_array_vec = table.extract_column<int32_t>(column_name);
        dump_column_vector(output_stream, val_array_vec, num_rows, array_size);
    } else if (boost::iequals(type_str, "UINT32_LE")) {
        auto val_array_vec = table.extract_column<uint32_t>(column_name);
        dump_column_vector(output_stream, val_array_vec, num_rows, array_size);
    } else if (boost::iequals(type_str, "INT64_LE")) {
        auto val_array_vec = table.extract_column<int64_t>(column_name);
        dump_column_vector(output_stream, val_array_vec, num_rows, array_size);
    } else if (boost::iequals(type_str, "UINT64_LE")) {
        auto val_array_vec = table.extract_column<uint64_t>(column_name);
        dump_column_vector(output_stream, val_array_vec, num_rows, array_size);
    } else if (boost::iequals(type_str, "FLOAT32_LE")) {
        auto val_array_vec = table.extract_column<float>(column_name);
        dump_column_vector(output_stream, val_array_vec, num_rows, array_size);
    } else if (boost::iequals(type_str, "FLOAT64_LE")) {
        auto val_array_vec = table.extract_column<double>(column_name);
        dump_column_vector(output_stream, val_array_vec, num_rows, array_size);
    } else if (boost::iequals(type_str, "char")) {
        std::string msg("extract_column() is not supported for columns of type char; ");
        msg.append("please use extract_column_values_as_strings().\n");
        throw(std::runtime_error(msg));
    } else {
        std::string msg("Invalid value of 'type' parameter: ");
        msg.append(type_str);
        throw(std::runtime_error(msg));
    }
}

/////////////////////////////////////////////////////////////////////

int main(int argc, char *argv[]) {
    bool stream_intermediate(false);
    tablator::Format input_format, output_format;
    std::vector<size_t> column_list;
    std::string column_string;
    std::vector<std::string> column_names_list;
    std::string column_names_string;
    size_t row_id = 0;
    size_t start_row = 0;
    size_t row_count = 0;
    std::vector<size_t> row_list;
    std::string row_string;
    bool call_static_f = false;
    bool exclude_cols_f = false;
    bool as_string_f = false;
    std::string input_format_str, output_format_str, column_name;
    std::string type_str;

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
            "text,html,hdf5)")(
            "column-list", boost::program_options::value<std::string>(&column_string),
            "list of columns to write (output-format ipac_table only)")(
            "type", boost::program_options::value<std::string>(&type_str),
            "Extracted value type (int8_t, uint8_t, int16_t, uint16_t, int32_t, "
            "uint32_t, int64_t, uint64_t, float, double, char)")(
            "row-id", boost::program_options::value<size_t>(&row_id), "requested row")(
            "start-row", boost::program_options::value<size_t>(&start_row),
            "first row to write (output-format ipac_table only)")(
            "row-count", boost::program_options::value<size_t>(&row_count),
            "number of consecutive rows to write (output-format ipac_table only)")(
            "row-list", boost::program_options::value<std::string>(&row_string),
            "list of rows to write (output-format ipac_table only)")(
            "static", boost::program_options::value<bool>(&call_static_f),
            "call static function, not Table class member")(
            "as-string", boost::program_options::value<bool>(&as_string_f),
            "return values as strings")(
            "col-name", boost::program_options::value<std::string>(&column_name),
            "name of column whose values to extract into vector")(
            "lookup-col-names",
            boost::program_options::value<std::string>(&column_names_string),
            "names of columns to translate to IDs")(
            "exclude-cols", boost::program_options::value<bool>(&exclude_cols_f),
            "listed columns are to be excluded (flag is true) or included (false, "
            "default)");

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
        if (option_variables.count("start-row") &&
            !option_variables.count("row-count")) {
            row_count = 1;
        }
        if (option_variables.count("row-id") && (option_variables.count("start-row") ||
                                                 option_variables.count("row-count"))) {
            std::cerr << "The parameters 'start-row' `and 'row-count' are both "
                         "incompatible with 'row-id'.\n";
            return 1;
        }
        if (!option_variables.count("start-row") &&
            option_variables.count("row-count")) {
            std::cerr << "The parameter 'row-count' is valid only if 'start-row' is "
                         "present.\n";
            return 1;
        }
        if (option_variables.count("row-list")) {
            if (option_variables.count("start-row")) {
                std::cerr << "The parameters 'row-list' and 'start-row' are mutually "
                             "incompatible.\n";
                return 1;
            }
            std::stringstream row_stream(row_string);
            std::vector<size_t> temp_list((std::istream_iterator<size_t>(row_stream)),
                                          std::istream_iterator<size_t>());
            row_list.swap(temp_list);
        }

        if (!option_variables.count("col-name") &&
            option_variables.count("as-string")) {
            std::cerr << "The parameter 'as-string' is valid only if 'col-name' is "
                         "present.\n";
            return 1;
        }

        if (option_variables.count("type") && as_string_f) {
            std::cerr
                    << "The value of 'as-string' must be false if 'type' is present.\n";
            return 1;
        }

        if (option_variables.count("column-list")) {
            if (option_variables.count("column-name")) {
                std::cerr << "The parameters 'column-list' and 'column-name' are "
                             "mutually "
                             "incompatible.\n";
                return 1;
            }
            std::stringstream column_stream(column_string);
            std::vector<size_t> temp_list(
                    (std::istream_iterator<size_t>(column_stream)),
                    std::istream_iterator<size_t>());
            column_list.swap(temp_list);
        }

        if (option_variables.count("lookup-col-names")) {
            // JTODO incompatible with everything
            // JTODO complement
            std::stringstream column_names_stream(column_names_string);
            std::vector<std::string> temp_list(
                    (std::istream_iterator<std::string>(column_names_stream)),
                    std::istream_iterator<std::string>());

            column_names_list.swap(temp_list);
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

        bool extract_single_value = false;
        bool extract_single_value_as_string = false;
        bool extract_column = false;
        bool extract_column_as_string = false;

        if (option_variables.count("col-name")) {
            if (option_variables.count("row-id")) {
                if (as_string_f) {
                    extract_single_value_as_string = true;
                } else {
                    extract_single_value = true;
                }
            } else {
                if (as_string_f) {
                    extract_column_as_string = true;
                } else {
                    extract_column = true;
                }
            }
        }

        bool do_subtable = (option_variables.count("start-row") ||
                            option_variables.count("row-list"));

        bool do_subtable_by_column_and_row =
                do_subtable && (option_variables.count("column-list"));

        if (do_subtable &&
            (output_format.enum_format != tablator::Format::Enums::IPAC_TABLE)) {
            std::cerr
                    << "Subtable feature requested via 'start-row' or 'row-list' "
                       "options is supported only when output-format is ipac_table.\n";
            return 1;
        }

        if (do_subtable && !column_name.empty()) {
            std::cerr << "Subtable feature is incompatible with extract-column-values "
                         "feature.\n";
            return 1;
        }

        // Do it!
        if (extract_single_value) {
            handle_extract_single_value(input_path, input_format, output_path,
                                        column_name, type_str, row_id);
        } else if (extract_column) {
            handle_extract_column(input_path, input_format, output_path, column_name,
                                  type_str);
        } else if (extract_single_value_as_string) {
            boost::filesystem::ifstream input_stream(input_path);
            tablator::Table table(input_stream, input_format);
            std::string value = table.extract_value_as_string(column_name, row_id);
            boost::filesystem::ofstream output_stream(output_path);
            output_stream << value;
        } else if (extract_column_as_string) {
            boost::filesystem::ifstream input_stream(input_path);
            tablator::Table table(input_stream, input_format);
            std::vector<std::string> col_values =
                    table.extract_column_values_as_strings(column_name);
            boost::filesystem::ofstream output_stream(output_path);
            std::copy(col_values.begin(), col_values.end(),
                      std::ostream_iterator<std::string>(output_stream, "\n"));
        } else if (do_subtable_by_column_and_row) {
            boost::filesystem::ifstream input_stream(input_path);
            tablator::Table table(input_stream, input_format);
            boost::filesystem::ofstream output_stream(output_path);
            if (option_variables.count("start-row")) {
                if (call_static_f) {
                    tablator::Ipac_Table_Writer::write_subtable_by_column_and_row(
                            table, output_stream, column_list, start_row, row_count);
                } else {
                    table.write_ipac_subtable_by_column_and_row(
                            output_stream, column_list, start_row, row_count);
                }
            } else {
                if (call_static_f) {
                    tablator::Ipac_Table_Writer::write_subtable_by_column_and_row(
                            table, output_stream, column_list, row_list);
                } else {
                    table.write_ipac_subtable_by_column_and_row(output_stream,
                                                                column_list, row_list);
                }
            }
        } else if (do_subtable) {
            boost::filesystem::ifstream input_stream(input_path);
            tablator::Table table(input_stream, input_format);
            boost::filesystem::ofstream output_stream(output_path);
            if (option_variables.count("start-row")) {
                if (call_static_f) {
                    tablator::Ipac_Table_Writer::write_subtable_by_row(
                            table, output_stream, start_row, row_count);
                } else {
                    table.write_ipac_subtable_by_row(output_stream, start_row,
                                                     row_count);
                }
            } else {
                if (call_static_f) {
                    tablator::Ipac_Table_Writer::write_subtable_by_row(
                            table, output_stream, row_list);
                } else {
                    table.write_ipac_subtable_by_row(output_stream, row_list);
                }
            }
        } else if (option_variables.count("lookup-col-names")) {
            boost::filesystem::ifstream input_stream(input_path);
            tablator::Table table(input_stream, input_format);
            std::vector<size_t> col_ids;
            if (exclude_cols_f) {
                col_ids = table.find_omitted_column_ids(column_names_list);
            } else {
                col_ids = table.find_column_ids(column_names_list);
            }
            boost::filesystem::ofstream output_stream(output_path);
            std::copy(col_ids.begin(), col_ids.end(),
                      std::ostream_iterator<size_t>(output_stream, " "));
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
