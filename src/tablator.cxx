/// A simple converter program to test out the tablator library.

#include <CCfits/CCfits>

#include "Table.hxx"

int main (int argc, char *argv[])
{
  if (argc != 3)
    {
      std::cerr << "Need two arguments but found " << argc - 1 << "\n";
      exit (1);
    }

  try
    {
      H5::Exception::dontPrint ();
      tablator::Table table (argv[1]);

      boost::filesystem::path output_path (argv[2]);
      tablator::Format output_format (output_path);
      table.write_output (output_path, output_format);
    }
  catch (std::runtime_error &exception)
    {
      std::cerr << exception.what () << "\n";
      std::cerr.flush ();
      exit (1);
    }
  catch (CCfits::FitsException &exception)
    {
      std::cerr << exception.message () << "\n";
      std::cerr.flush ();
      exit (1);
    }
  catch (H5::Exception &exception)
    {
      std::cerr << "In "
                << exception.getFuncName () << ": "
                << exception.getDetailMsg () << "\n";
      std::cerr.flush ();
      exit (1);
    }
}
