#include "../Format.hxx"

std::string tablator::Format::content_type() const {
    switch (enum_format) {
        case Enums::CSV:
            return "Content-type: text/csv\r\n\r\n";
            break;

        case Enums::TSV:
            return "Content-type: text/tab-separated-values\r\n\r\n";
            break;

        case Enums::TEXT:
        case Enums::IPAC_TABLE:
        case Enums::POSTGRES_SQL:
        case Enums::ORACLE_SQL:
        case Enums::SQLITE_SQL:
            return "Content-type: text/plain\r\n\r\n";
            break;

        case Enums::SQLITE_DB:
            return "Content-type: application/x-sqlite3\r\n\r\n";
            break;

        case Enums::JSON:
        case Enums::JSON5:
            return "Content-type: application/json\r\n\r\n";
            break;

        case Enums::VOTABLE:
            return "Content-type: application/x-votable+xml\r\n\r\n";
            break;

        case Enums::FITS:
            return "Content-type: application/fits\r\n\r\n";
            break;

        case Enums::HTML:
            return "Content-type: text/html\r\n\r\n";
            break;

        case Enums::HDF5:
            return "Content-type: application/x-hdf\r\n\r\n";
            break;

        default:
            throw std::runtime_error(
                    "INTERNAL ERROR: Unknown format when "
                    "generating content type: " +
                    std::to_string(static_cast<int>(enum_format)));
    }
}
