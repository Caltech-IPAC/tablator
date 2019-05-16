#include <CCfits/CCfits>
#include <boost/filesystem.hpp>

namespace tablator {
bool is_fits(const boost::filesystem::path &path) {
    bool result(true);
    try {
        CCfits::FITS fits(path.string(), CCfits::Read, false);
    } catch (...) {
        result = false;
    }
    return result;
}
}  // namespace tablator
