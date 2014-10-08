#include "Table.hxx"

int main(int argc, char *argv[])
{
  if(argc!=3)
    {
      std::cerr << "Need two arguments but found " << argc-1 << "\n";
      exit(1);
    }

  Tablator::Table table(argv[1]);
  boost::filesystem::path output_path(argv[2]);
  Tablator::Format output_format(output_path);
  table.write_output(output_path,output_format);
}
