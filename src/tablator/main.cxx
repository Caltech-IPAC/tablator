// A simple converter program based on the tablator library.
#include <iostream>
#include <sstream>

#include <json5_parser.h>
#include <CCfits/CCfits>

#include <boost/algorithm/string.hpp>
#include <boost/program_options.hpp>

#include "../Table.hxx"
#include "../Table_Ops.hxx"

static constexpr size_t SIZE_T_MAX = std::numeric_limits<size_t>::max();


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
    auto column_id = table.get_column_index(column_name);
    auto array_size = table.get_columns().at(column_id).get_array_size();
    auto num_rows = table.get_num_rows();

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
        if (row_id < table.get_num_rows()) {
            modified_row_id_list.emplace_back(row_id);
        } else if (row_id < SIZE_T_MAX) {
            // row_id == SIZE_T_MAX if user did not specify row-id.
            std::string msg("Error: row-id value is too large.\n");
            throw(std::runtime_error(msg));
        } else if (start_row < table.get_num_rows()) {
            size_t modified_row_count =
                    std::min(row_count, table.get_num_rows() - start_row);
            modified_row_id_list.resize(modified_row_count);
            std::iota(modified_row_id_list.begin(), modified_row_id_list.end(),
                      start_row);
        } else if (start_row < SIZE_T_MAX) {
            // start_row == SIZE_T_MAX if user did not specify start-row.
            std::string msg("Error: start-row value is too large.\n");
            throw(std::runtime_error(msg));
        } else {
            // User didn't specify constraints, so return all rows.
            modified_row_id_list.resize(table.get_num_rows());
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


ushort parse_trim_decimal_runs(const std::string &trim_str) {
    static const std::string dec_run_error(
            "Error: value of trim-decimal-runs must be \"0\", \"1\", or \"1:N\" where "
            "3 <= N. Trimming will not take place for N >= 25. \n");
    bool dec_error = false;
    ushort min_run_length = tablator::MIN_RUN_LENGTH_FOR_TRIMMING;
    if (trim_str == "0") {
        min_run_length = tablator::SIGNAL_NO_TRIMMING;
    } else if (boost::starts_with(trim_str, "1:")) {
        std::string min_run_str = trim_str.substr(2);
        try {
            min_run_length = (ushort)std::stoi(min_run_str);
        } catch (std::exception &e) {
            throw(std::runtime_error(dec_run_error));
        }
        if (min_run_length < 3) {
            dec_error = true;
        }
    } else if (trim_str != "1") {
        dec_error = true;
    }

    if (dec_error) {
        throw(std::runtime_error(dec_run_error));
    }
    return min_run_length;
}

/////////////////////////////////////////////////////////////////////

int main(int argc, char *argv[]) {
    bool stream_intermediate(false);
    tablator::Format input_format, input2_format, output_format;
    std::vector<size_t> column_id_list;
    std::string column_id_string;
    std::vector<std::string> column_name_list;
    std::string column_name_string;
    size_t row_id = SIZE_T_MAX;
    size_t start_row = SIZE_T_MAX;
    size_t row_count = SIZE_T_MAX;
    std::vector<size_t> row_list;
    std::string row_string;
    bool call_static_f = false;
    bool exclude_cols_f = false;
    bool skip_comments_f = false;
    bool as_string_f = false;
    std::string input_format_str;
    std::string input2_format_str;
    std::string output_format_str;
    std::string column_to_extract;
    std::string type_str;
    bool write_null_strings_f = false;
    bool idx_lookup = false;
    std::string trim_decimal_runs = "1";
    std::string counter_column_name = "";
    bool combine_tables_f = false;
    bool append_rows_f = true;

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
            "input2-format",
            boost::program_options::value<std::string>(&input2_format_str),
            "Input2 file format (json,json5,votable,csv,tsv,fits,ipac_table,"
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
                        "as empty string (default false)")(
            "counter-column",
            boost::program_options::value<std::string>(&counter_column_name),
            "add counter column with indicated name")(
            "combine-tables",
            boost::program_options::value<bool>(&combine_tables_f)
                    ->default_value(false),
            "combine tables having same number of rows)")(
            "append-rows",
            boost::program_options::value<bool>(&append_rows_f)->default_value(false),
            "append rows of second table having same columns and offsets)")(
            "trim-decimal-runs",
            boost::program_options::value<std::string>(&trim_decimal_runs),
            "check for and round round up/down decimal runs of length N of 9s or of 0s "
            "in string "
            "representations of doubles (valid values \"0\", \"1\", \"1:N\"; \"1\" "
            "gets default run length; default \"1\".)");

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

        if (option_variables.count("input-format")) {
            input_format.init(input_format_str);
        }
        if (option_variables.count("input2-format")) {
            input2_format.init(input2_format_str);
        }
        if (option_variables.count("output-format")) {
            output_format.init(output_format_str);
        }
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

        boost::filesystem::path input_path, input2_path, output_path;
        if (option_variables.count("files")) {
            std::vector<std::string> paths(
                    option_variables["files"].as<std::vector<std::string> >());
            if (paths.size() == 1) {
                std::cerr << "Missing an output file\n";
                return 1;
            }

            if (combine_tables_f || append_rows_f) {
                if (paths.size() != 3) {
                    std::cerr << "combine-tables and append-rows options require 3 "
                                 "paths.\n";
                    return 1;
                }
                input_path = paths.at(0);
                input2_path = paths.at(1);
                output_path = paths.at(2);
                if (input_format.is_unknown()) {
                    input_format.set_from_extension(input_path);
                }
                if (input2_format.is_unknown()) {
                    input2_format.set_from_extension(input2_path);
                }
                if (output_format.is_unknown()) {
                    output_format.set_from_extension(output_path);
                }
            } else {
                if (paths.size() != 2) {
                    std::cerr << "Too many filenames\n";
                    return 1;
                }
                input_path = paths.at(0);
                output_path = paths.at(1);
                if (input_format.is_unknown())
                    input_format.set_from_extension(input_path);
                if (output_format.is_unknown())
                    output_format.set_from_extension(output_path);
            }
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

        ushort min_run_length = parse_trim_decimal_runs(trim_decimal_runs);
        tablator::Command_Line_Options options(min_run_length, write_null_strings_f,
                                               skip_comments_f);

        /**************/
        /*** Do it! ***/
        /**************/

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
                    table.extract_value_as_string(column_to_extract, row_id, options);
            boost::filesystem::ofstream output_stream(output_path);
            output_stream << value;
        } else if (extract_column_as_string) {
            boost::filesystem::ifstream input_stream(input_path);
            tablator::Table table(input_stream, input_format);
            std::vector<std::string> col_values =
                    table.extract_column_values_as_strings(column_to_extract, options);
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
        } else if (!counter_column_name.empty()) {
            // JTODO make this option incompatible with other options
            boost::filesystem::ifstream input_stream(input_path);
            tablator::Table in_table(input_stream, input_format);
            tablator::Table out_table =
                    tablator::add_counter_column(in_table, counter_column_name);
            boost::filesystem::ofstream output_stream(output_path);
            out_table.write(output_stream, output_path.stem().native(), output_format,
                            options);
        } else if (combine_tables_f) {
            // JTODO make this option incompatible with other options
            boost::filesystem::ifstream input_stream(input_path);
            tablator::Table in_table1(input_stream, input_format);
            input_stream.close();

            boost::filesystem::ifstream input2_stream(input2_path);
            tablator::Table in_table2(input2_stream, input2_format);
            input2_stream.close();

            tablator::Table out_table = tablator::combine_tables(in_table1, in_table2);

            boost::filesystem::ofstream output_stream(output_path);
            out_table.write(output_stream, output_path.stem().native(), output_format,
                            options);
        } else if (append_rows_f) {
            // JTODO make this option incompatible with other options
            boost::filesystem::ifstream input_stream(input_path);
            tablator::Table in_table1(input_stream, input_format);
            input_stream.close();

            boost::filesystem::ifstream input2_stream(input2_path);
            tablator::Table in_table2(input2_stream, input2_format);
            input2_stream.close();

            in_table1.append_rows(in_table2);

            boost::filesystem::ofstream output_stream(output_path);
            in_table1.write(output_stream, output_path.stem().native(), output_format,
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
