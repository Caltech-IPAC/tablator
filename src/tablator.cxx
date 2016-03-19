/// A simple converter program to test out the tablator library.

#include <CCfits/CCfits>
#include <json5_parser.h>

#include "Table.hxx"

int main (int argc, char *argv[])
{
  bool stream_intermediate (false);
  if (argc != 3)
    {
      stream_intermediate=true;
      if (argc != 4 || std::string(argv[2])!="--stream-intermediate")
        {
          std::cerr << "Usage: tablator <input> [--stream-intermediate] <output>"
                    << "\n";
          exit (1);
        }
    }

  try
  {
    H5::Exception::dontPrint ();
    tablator::Table table (argv[1]);

    boost::filesystem::path output_path (argv[stream_intermediate ? 3 : 2]);
    tablator::Format output_format (output_path);
    if (stream_intermediate)
      {
        boost::filesystem::ofstream outfile (output_path);
        table.write_output (outfile, output_format);
      }
    else
      {
        table.write_output (output_path, output_format);
      }
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
    std::cerr << "In " << exception.getFuncName () << ": "
              << exception.getDetailMsg () << "\n";
    std::cerr.flush ();
    exit (1);
  }
  catch (json5_parser::Error_position &exception)
  {
    std::cerr << "On line " << exception.line_ << ", column "
              << exception.column_ << ": " << exception.reason_ << "\n";
    std::cerr.flush ();
    exit (1);
  }
}
