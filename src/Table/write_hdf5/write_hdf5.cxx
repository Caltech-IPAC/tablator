#include "../../Table.hxx"

//#include <H5Cpp.h>
#include <H5Ppublic.h>

// Note: We would like to set the version to at least 1.8 in both
// versions of write_hdf5() in order to make the files more compact,
// but we wind up with corrupted files when we attempt to do so using
// H5Pset_libver_bounds().  In any case, the value H5F_LIBVER_V18 is
// not currently a legal value for the lower bound. 17Apr20

void tablator::Table::write_hdf5(std::ostream &os) const {
    /// This makes two copies of the file in memory.  It seems like
    /// there should be a way to only use one.

    hbool_t backing_store = 0;
    H5::FileAccPropList fapl;

    fapl.setCore(size_t(1), backing_store);
    H5::H5File memory_file("nosuch.h5", H5F_ACC_TRUNC, H5::FileCreatPropList::DEFAULT,
                           fapl);
    write_hdf5_to_H5File(memory_file);
    memory_file.flush(H5F_SCOPE_GLOBAL);

    hid_t fileid = memory_file.getId();
    size_t size = H5Fget_file_image(fileid, NULL, 0);
    std::vector<char> buffer(size);
    H5Fget_file_image(fileid, buffer.data(), size);
    os.write(buffer.data(), size);
}

void tablator::Table::write_hdf5(const boost::filesystem::path &p) const {
    H5::FileAccPropList fapl;
//    H5Pset_libver_bounds(fapl.getId(), H5F_LIBVER_EARLIEST, H5F_LIBVER_LATEST);
    H5::H5File outfile{p.string(), H5F_ACC_TRUNC, H5::FileCreatPropList::DEFAULT, fapl};
    write_hdf5_to_H5File(outfile);
}
