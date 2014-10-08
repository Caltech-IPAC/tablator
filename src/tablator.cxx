#include "Table.hxx"

int main(int argc, char *argv[])
{
  if(argc!=3)
    {
      std::cerr << "Need two arguments but found " << argc-1 << "\n";
      exit(1);
    }
  // Tablator::Format::formats.insert(make_pair(Tablator::Format::enum_format::VOTABLE,
  //                                            make_pair(std::string("votable"),
  //                                                      std::vector<std::string>(1,"xml"))));

  // Tablator::Format::formats.insert(make_pair(Tablator::Format::enum_format::FITS,
  //                                            make_pair(std::string("fits"),
  //                                                      std::vector<std::string>(1,"fits"))));

  // Tablator::Format::formats.insert(make_pair(Tablator::Format::enum_format::HDF5,
  //                                            make_pair(std::string("hdf5"),
  //                                                      std::vector<std::string>(1,"hdf5"))));

  Tablator::Table table(argv[1]);

  // boost::filesystem::path output_path(argv[2]);
  // Tablator::Format output_format(output_path);
  // std::cout << "formated\n";
  // std::cout.flush();
  // table.write_output(output_path,output_format);
  // std::cout << "output\n";
  // std::cout.flush();
}
