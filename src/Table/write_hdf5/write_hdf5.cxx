#include "../../Table.hxx"

/// Only recent versions of the C++ api have a way to call
/// set_libver_bounds.  So we have to use the C api.
#include <H5Ppublic.h>

void tablator::Table::write_hdf5 (std::ostream &os) const
{
  /// This makes two copies of the file in memory.  It seems like
  /// there should be a way to only use one.

  hbool_t backing_store = 0;
  H5::FileAccPropList fapl;
  /// Set the version to at least 1.8 to make the files more compact.
  H5Pset_libver_bounds (fapl.getId (), H5F_LIBVER_18, H5F_LIBVER_LATEST);
  fapl.setCore (size_t (1), backing_store);
  H5::H5File memory_file ("nosuch.h5", H5F_ACC_TRUNC,
                          H5::FileCreatPropList::DEFAULT, fapl);
  write_hdf5_to_H5File (memory_file);
  memory_file.flush (H5F_SCOPE_GLOBAL);

  hid_t fileid = memory_file.getId ();
  size_t size = H5Fget_file_image (fileid, NULL, 0);
  std::vector<char> buffer (size);
  H5Fget_file_image (fileid, buffer.data (), size);
  os.write (buffer.data (), size);
}

void tablator::Table::write_hdf5 (const boost::filesystem::path &p) const
{
  /// Set the version to at least 1.8 to make the files more compact.
  H5::FileAccPropList fapl;
  H5Pset_libver_bounds (fapl.getId (), H5F_LIBVER_18, H5F_LIBVER_LATEST);
  H5::H5File outfile{ p.string (), H5F_ACC_TRUNC,
                      H5::FileCreatPropList::DEFAULT, fapl };
  write_hdf5_to_H5File (outfile);
}
