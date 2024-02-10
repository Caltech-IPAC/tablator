// A simple converter program based on the tablator library.

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
    auto array_size = table.get_columns().at(column_id).get_array_size();
    auto num_rows = table.num_rows();

    if (boost::iequals(type_str, "INT8_LE")) {
        auto val_array_vec = table.extract_column<int8_t>(column_id);
        dump_column_vector(output_stream, val_array_vec, num_rows, array_size);
    } else if (boost::iequals(type_str, "UINT8_LE")) {
        auto val_array_vec = table.extract_column<uint8_t>(column_id);
        dump_column_vector(output_stream, val_array_vec, num_rows, array_size);
    } else if (boost::iequals(type_str, "INT16_LE")) {
        auto val_array_vec = table.extract_column<int16_t>(column_id);
        dump_column_vector(output_stream, val_array_vec, num_rows, array_size);
    } else if (boost::iequals(type_str, "UINT16_LE")) {
        auto val_array_vec = table.extract_column<uint16_t>(column_id);
        dump_column_vector(output_stream, val_array_vec, num_rows, array_size);
    } else if (boost::iequals(type_str, "INT32_LE")) {
        auto val_array_vec = table.extract_column<int32_t>(column_id);
        dump_column_vector(output_stream, val_array_vec, num_rows, array_size);
    } else if (boost::iequals(type_str, "UINT32_LE")) {
        auto val_array_vec = table.extract_column<uint32_t>(column_id);
        dump_column_vector(output_stream, val_array_vec, num_rows, array_size);
    } else if (boost::iequals(type_str, "INT64_LE")) {
        auto val_array_vec = table.extract_column<int64_t>(column_id);
        dump_column_vector(output_stream, val_array_vec, num_rows, array_size);
    } else if (boost::iequals(type_str, "UINT64_LE")) {
        auto val_array_vec = table.extract_column<uint64_t>(column_id);
        dump_column_vector(output_stream, val_array_vec, num_rows, array_size);
    } else if (boost::iequals(type_str, "FLOAT32_LE")) {
        auto val_array_vec = table.extract_column<float>(column_id);
        dump_column_vector(output_stream, val_array_vec, num_rows, array_size);
    } else if (boost::iequals(type_str, "FLOAT64_LE")) {
        auto val_array_vec = table.extract_column<double>(column_id);
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


void handle_write_ipac_subtable(boost::filesystem::ofstream &output_stream,
                                const tablator::Table &table,
                                const std::vector<size_t> &column_id_list,
                                const std::vector<size_t> &row_id_list, size_t row_id,
                                size_t start_row, size_t row_count, bool call_static_f,
                                const tablator::Command_Line_Options &options) {
    static size_t MAX_SIZE_T = std::numeric_limits<size_t>::max();
    const std::vector<size_t> *active_column_id_ptr = &column_id_list;
    std::vector<size_t> modified_column_id_list;
    if (column_id_list.empty()) {
        modified_column_id_list.resize(table.get_columns().size() - 1);
        std::iota(modified_column_id_list.begin(), modified_column_id_list.end(), 1);
        active_column_id_ptr = &modified_column_id_list;
    }

    const std::vector<size_t> *active_row_id_ptr = &row_id_list;
    std::vector<size_t> modified_row_id_list;
    if (row_id_list.empty()) {
        if (row_id != MAX_SIZE_T) {
            modified_row_id_list.emplace_back(row_id);
        } else if (start_row != MAX_SIZE_T) {
            size_t active_row_count = std::max(
                    size_t(0), std::min(row_count, table.num_rows() - start_row));
            modified_row_id_list.resize(active_row_count);
            std::iota(modified_row_id_list.begin(), modified_row_id_list.end(),
                      start_row);
        } else {
            modified_row_id_list.resize(table.num_rows());
            std::iota(modified_row_id_list.begin(), modified_row_id_list.end(), 0);
        }
        active_row_id_ptr = &modified_row_id_list;
    }

    if (call_static_f) {
        tablator::Ipac_Table_Writer::write_subtable_by_column_and_row(
                table, output_stream, *active_column_id_ptr, *active_row_id_ptr,
                options);
    } else {
        table.write_ipac_subtable_by_column_and_row(
                output_stream, *active_column_id_ptr, *active_row_id_ptr, options);
    }
}

/////////////////////////////////////////////////////////////////////

int main(int argc, char *argv[]) {
    bool stream_intermediate(false);
    tablator::Format input_format, output_format;
    std::vector<size_t> column_id_list;
    std::string column_id_string;
    std::vector<std::string> column_name_list;
    std::string column_name_string;
    size_t row_id = std::numeric_limits<size_t>::max();
    size_t start_row = std::numeric_limits<size_t>::max();
    size_t row_count = std::numeric_limits<size_t>::max();
    std::vector<size_t> row_list;
    std::string row_string;
    bool call_static_f = false;
    bool exclude_cols_f = false;
    bool skip_comments_f = false;
    bool as_string_f = false;
    std::string input_format_str;
    std::string output_format_str;
    std::string column_to_extract;
    std::string type_str;
    bool write_null_strings_f = false;
    bool idx_lookup = false;

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
            "column-ids", boost::program_options::value<std::string>(&column_id_string),
            "list of 1-based ids of columns to write (output-format ipac_table only)")(
            "column-names",
            boost::program_options::value<std::string>(&column_name_string),
            "list names of columns to write (output-format ipac_table only) "
            "or (via idx-lookup) look up indices for")(
            "idx-lookup", boost::program_options::value<bool>(&idx_lookup),
            "return list of column indices rather than subtable (default is 'false')")(
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
            "column-to-extract",
            boost::program_options::value<std::string>(&column_to_extract),
            "name of single column whose value(s) to extract")(
            "as-string", boost::program_options::value<bool>(&as_string_f),
            "return values as strings")(
            "exclude-cols", boost::program_options::value<bool>(&exclude_cols_f),
            "named columns are to be excluded (flag is true) or included (false, "
            "default)")(
            "skip-comments",
            boost::program_options::value<bool>(&skip_comments_f)->default_value(false),
            // NOTE: support for implicit_value() is buggy and implicit_value()
            //       is retired in future (post 1_59(?)) versions of boost.
            //               ->implicit_value(true),
            "header comments are to be skipped (flag is true) or included (false, "
            "default)")("write-null-string",
                        boost::program_options::bool_switch(&write_null_strings_f)
                                ->default_value(false),
                        "render null values in tsv/csv tables  as \"null\" rather than "
                        "as empty string (default false)");

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

        /***************************/
        /*** Validate and adjust ***/
        /***************************/

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
            std::copy(std::istream_iterator<size_t>(row_stream),
                      std::istream_iterator<size_t>(), std::back_inserter(row_list));
        }

        if (!option_variables.count("column-to-extract") &&
            option_variables.count("as-string")) {
            std::cerr << "The parameter 'as-string' is valid only if "
                         "'column-to-extract' is "
                         "present.\n";
            return 1;
        }

        if (option_variables.count("type") && as_string_f) {
            std::cerr
                    << "The value of 'as-string' must be false if 'type' is present.\n";
            return 1;
        }

        if (option_variables.count("column-ids")) {
            if (option_variables.count("exclude-cols")) {
                std::cerr << "The combination 'column-ids' and 'exclude-cols' is "
                             "not supported.\n";
                return 1;
            }
            if (option_variables.count("column-to-extract")) {
                std::cerr << "The parameters 'column-ids' and 'column-to-extract' are "
                             "mutually "
                             "incompatible.\n";
                return 1;
            }
            std::stringstream column_id_stream(column_id_string);
            std::copy(std::istream_iterator<size_t>(column_id_stream),
                      std::istream_iterator<size_t>(),
                      std::back_inserter(column_id_list));
        }

        if (option_variables.count("column-names")) {
            if (option_variables.count("column-to-extract") ||
                (option_variables.count("column-ids"))) {
                std::cerr << "The parameter 'column-names' is incompatible with both "
                             "'column-ids'"
                          << " and 'column-to-extract'.\n";
            }
            std::stringstream column_name_stream(column_name_string);
            std::copy(std::istream_iterator<std::string>(column_name_stream),
                      std::istream_iterator<std::string>(),
                      std::back_inserter(column_name_list));
        }

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

        /********************************/
        /*** Identify type of request ***/
        /********************************/

        bool extract_single_value = false;
        bool extract_single_value_as_string = false;
        bool extract_column = false;
        bool extract_column_as_string = false;

        if (option_variables.count("column-to-extract")) {
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

        bool do_subtable_by_row = (option_variables.count("start-row") ||
                                   option_variables.count("row-list"));


        if (do_subtable_by_row && !column_to_extract.empty()) {
            std::cerr << "Subtable-by-row feature is incompatible with "
                         "column-to-extract feature.\n";
            return 1;
        }

        bool do_subtable = option_variables.count("column-ids") ||
                           (option_variables.count("column-names") && !idx_lookup) ||
                           do_subtable_by_row;

        if (do_subtable &&
            (output_format.enum_format != tablator::Format::Enums::IPAC_TABLE)) {
            std::cerr << "Subtable feature requested via 'start-row', 'row-list', "
                      << "'column-ids', or 'column-names' options is supported only "
                      << "when output-format is ipac_table.\n";
            return 1;
        }

        if (write_null_strings_f &&
            output_format.enum_format != tablator::Format::Enums::CSV &&
            output_format.enum_format != tablator::Format::Enums::TSV) {
            std::cerr << "\"write-null-string\" feature is supported only when writing "
                         "in CSV or TSV format.";
        }

        /**************/
        /*** Do it! ***/
        /**************/

        tablator::Command_Line_Options options(write_null_strings_f, skip_comments_f);

        if (extract_single_value) {
            handle_extract_single_value(input_path, input_format, output_path,
                                        column_to_extract, type_str, row_id);
        } else if (extract_column) {
            handle_extract_column(input_path, input_format, output_path,
                                  column_to_extract, type_str);
        } else if (extract_single_value_as_string) {
            boost::filesystem::ifstream input_stream(input_path);
            tablator::Table table(input_stream, input_format);
            std::string value =
                    table.extract_value_as_string(column_to_extract, row_id);
            boost::filesystem::ofstream output_stream(output_path);
            output_stream << value;
        } else if (extract_column_as_string) {
            boost::filesystem::ifstream input_stream(input_path);
            tablator::Table table(input_stream, input_format);
            std::vector<std::string> col_values =
                    table.extract_column_values_as_strings(column_to_extract);
            boost::filesystem::ofstream output_stream(output_path);
            std::copy(col_values.begin(), col_values.end(),
                      std::ostream_iterator<std::string>(output_stream, "\n"));
        } else if (option_variables.count("column-names")) {
            boost::filesystem::ifstream input_stream(input_path);
            tablator::Table table(input_stream, input_format);
            std::vector<size_t> col_ids;
            if (exclude_cols_f) {
                col_ids = table.find_omitted_column_ids(column_name_list);
            } else {
                col_ids = table.find_column_ids(column_name_list);
            }

            boost::filesystem::ofstream output_stream(output_path);
            if (idx_lookup) {
                std::copy(col_ids.begin(), col_ids.end(),
                          std::ostream_iterator<size_t>(output_stream, " "));
            } else if (col_ids.empty()) {
                std::string msg("Error: subtable must contain at least one column.\n");
                throw(std::runtime_error(msg));
            } else {
                handle_write_ipac_subtable(output_stream, table, col_ids, row_list,
                                           row_id, start_row, row_count, call_static_f,
                                           options);
            }
        } else if (do_subtable) {
            boost::filesystem::ifstream input_stream(input_path);
            tablator::Table table(input_stream, input_format);
            boost::filesystem::ofstream output_stream(output_path);
            handle_write_ipac_subtable(output_stream, table, column_id_list, row_list,
                                       row_id, start_row, row_count, call_static_f,
                                       options);
        } else if (stream_intermediate) {
            boost::filesystem::ifstream input_stream(input_path);
            tablator::Table table(input_stream, input_format);
            boost::filesystem::ofstream output_stream(output_path);
            table.write(output_stream, output_path.stem().native(), output_format,
                        options);
        } else {
            tablator::Table table(input_path, input_format);
            table.write(output_path, output_format, options);
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
