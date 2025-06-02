#pragma once

#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>

#include <map>
#include <utility>

namespace tablator {
class Format {
public:
    enum class Enums {
        JSON,
        JSON5,
        VOTABLE,
        CSV,
        TSV,
        FITS,
        IPAC_TABLE,
        TEXT,
        HTML,
        HDF5,
        POSTGRES_SQL,
        ORACLE_SQL,
        SQLITE_SQL,
        SQLITE_DB,
        VOTABLE_BINARY2,
        UNKNOWN
    };

    // FIXME: This should really be a static, but then I ran into
    // problems with order of static initialization, where something
    // static would get destroyed before this variable, causing crashes
    // on exit.
    const std::map<Enums, std::pair<std::string, std::vector<std::string> > > formats{
            {Enums::JSON, {"json", {"js", "json"}}},
            {Enums::JSON5, {"json5", {"js5", "json5"}}},
            {Enums::VOTABLE, {"votable", {"xml", "vot", "vo"}}},
            {Enums::CSV, {"csv", {"csv"}}},
            {Enums::TSV, {"tsv", {"tsv"}}},
            {Enums::FITS, {"fits", {"fits"}}},
            {Enums::IPAC_TABLE, {"ipac_table", {"tbl"}}},
            {Enums::TEXT, {"text", {"txt"}}},
            {Enums::HTML, {"html", {"html"}}},
            {Enums::HDF5, {"hdf5", {"h5", "hdf", "hdf5"}}},
            {Enums::POSTGRES_SQL, {"postgres", {"postgres"}}},
            {Enums::ORACLE_SQL, {"oracle", {"oracle"}}},
            {Enums::SQLITE_SQL, {"sqlite", {"sqlite"}}},
            {Enums::SQLITE_DB, {"db", {"db"}}},
            {Enums::VOTABLE_BINARY2, {"votable_binary2", {"vbin2"}}},
            {Enums::UNKNOWN, {"unknown", {}}}};

    Enums enum_format = Enums::UNKNOWN;

    std::string extension() const {
        auto f = formats.find(enum_format);
        if (f == formats.end())
            throw std::runtime_error(
                    "INTERNAL ERROR: Format::enum_format is not valid: " +
                    std::to_string(static_cast<int>(enum_format)));
        if (f->second.second.empty()) return "";
        return "." + f->second.second.at(0);
    }

    Format() = default;
    Format(const boost::filesystem::path &path) { set_from_extension(path); }
    Format(const boost::filesystem::path &path, const Enums &default_format) {
        set_from_extension(path, default_format);
    }

    Format &operator=(const Format &f) {
        enum_format = f.enum_format;
        return *this;
    }
    void init(const std::string &format) {
        for (auto &f : formats) {
            if (boost::iequals(f.second.first, format)) {
                enum_format = f.first;
                break;
            }
        }
    }

    Format(const std::string &format) { init(format); }

    Format(const char *format) : Format(std::string(format)) {}

    void set_from_extension(const boost::filesystem::path &path) {
        set_from_extension(path, Enums::IPAC_TABLE);
    }
    void set_from_extension(const boost::filesystem::path &path,
                            const Enums &default_format);
    std::string content_type() const;

    bool is_ipac_table() const { return enum_format == Enums::IPAC_TABLE; }
    bool is_json() const { return enum_format == Enums::JSON; }
    bool is_json5() const { return enum_format == Enums::JSON5; }
    bool is_votable() const { return enum_format == Enums::VOTABLE; }
    bool is_votable_binary2() const { return enum_format == Enums::VOTABLE_BINARY2; }
    bool is_csv() const { return enum_format == Enums::CSV; }
    bool is_tsv() const { return enum_format == Enums::TSV; }
    bool is_text() const { return enum_format == Enums::TEXT; }
    bool is_fits() const { return enum_format == Enums::FITS; }
    bool is_html() const { return enum_format == Enums::HTML; }
    bool is_hdf5() const { return enum_format == Enums::HDF5; }
    bool is_sqlite_db() const { return enum_format == Enums::SQLITE_DB; }
    bool is_unknown() const { return enum_format == Enums::UNKNOWN; }

    std::string string() const {
        auto f = formats.find(enum_format);
        if (f == formats.end())
            throw std::runtime_error(
                    "INTERNAL ERROR: Format::enum_format is not valid: " +
                    std::to_string(static_cast<int>(enum_format)));
        return "." + f->second.first;
    }
};
}  // namespace tablator

inline std::ostream &operator<<(std::ostream &os, const tablator::Format &f) {
    os << f.string();
    return os;
}
