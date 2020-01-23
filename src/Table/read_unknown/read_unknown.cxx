#include "../../Table.hxx"

#include <functional>

namespace tablator {
bool is_ipac_table(std::istream &input_stream);
bool is_ipac_table(const boost::filesystem::path &path);

bool is_json5(std::istream &input_stream);
bool is_json5(const boost::filesystem::path &path);

bool is_votable(std::istream &input_stream);
bool is_votable(const boost::filesystem::path &path);

bool is_fits(const boost::filesystem::path &path);
}  // namespace tablator

namespace {
template <typename... Arguments>
bool convert_to_error_code(void (tablator::Table::*f)(Arguments &...),
                           tablator::Table *table, Arguments &... args) {
    bool result(false);
    try {
        std::cout << "reading\n";
        (table->*f)(args...);
        result = true;
        std::cout << "read\n";
    } catch (...) {
    }
    return result;
}
}  // namespace

void tablator::Table::read_unknown(std::istream &input_stream) {
    // FIXME: Implement reading hdf5 and fits in memory
    if (is_ipac_table(input_stream)) {
        read_ipac_table(input_stream);
    } else if (is_json5(input_stream)) {
        read_json5(input_stream);
    } else if (is_votable(input_stream)) {
        read_votable(input_stream);
    } else {
        /// The input stream is not guaranteed to be seekable.  So we
        /// make a copy and use that.

        /// No robust way to know if a file is a csv or dsv without
        /// parsing everything, because elements can have tabs and
        /// commas.
        std::stringstream ss;
        ss << input_stream.rdbuf();
        try {
            read_dsv(ss, Format("csv"));
        } catch (...) {
            ss.seekg(0);
            try {
                read_dsv(ss, Format("tsv"));
            } catch (...) {
                throw std::runtime_error("Unable to read stream");
            }
        }
    }
    const auto &columns = get_columns();
    if (columns.size() < 2) {
        throw std::runtime_error("This stream has no columns");
    }
}

void tablator::Table::read_unknown(const boost::filesystem::path &input_path) {
    if (H5::H5File::isHdf5(input_path.string())) {
        read_hdf5(input_path);
    } else if (is_fits(input_path)) {
        read_fits(input_path);
    } else if (is_json5(input_path)) {
        read_json5(input_path);
    } else if (is_votable(input_path)) {
        read_votable(input_path);
    } else if (is_ipac_table(input_path)) {
        read_ipac_table(input_path);
    } else {
        try {
            read_dsv(input_path, Format("csv"));
        } catch (...) {
            try {
                read_dsv(input_path, Format("tsv"));
            } catch (...) {
                throw std::runtime_error("Unsupported format for input file: " +
                                         input_path.string());
            }
        }
    }
    const auto &columns = get_columns();
    if (columns.size() < 2) {
        throw std::runtime_error("This file has no columns: " + input_path.string());
    }
}
