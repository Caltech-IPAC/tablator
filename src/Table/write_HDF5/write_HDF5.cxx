#include "../../Table.hxx"

void TAP::Table::write_HDF5 (std::ostream &os) const
{
  /// This makes two copies of the file in memory.  It seems like
  /// there should be a way to only use one.

  hbool_t backing_store = 0;
  H5::FileAccPropList fapl;
  fapl.setCore ((size_t)1, backing_store);
  H5::H5File memory_file ("nosuch.h5", H5F_ACC_TRUNC,
                          H5::FileCreatPropList::DEFAULT, fapl);
  write_HDF5_to_file (memory_file);
  memory_file.flush (H5F_SCOPE_GLOBAL);

  hid_t fileid = memory_file.getId ();
  size_t size = H5Fget_file_image (fileid, NULL, 0);
  std::vector<char> buffer (size);
  H5Fget_file_image (fileid, buffer.data (), size);
  os.write (buffer.data (), size);
}

void TAP::Table::write_HDF5 (const boost::filesystem::path &p) const
{
  H5::H5File outfile{ p.string (), H5F_ACC_TRUNC };
  write_HDF5_to_file (outfile);
}
