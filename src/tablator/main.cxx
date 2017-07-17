/// A simple converter program to test out the tablator library.

#include <CCfits/CCfits>
#include <boost/program_options.hpp>
#include <json5_parser.h>

#include "../Table.hxx"

std::string
usage (const boost::program_options::options_description &visible_options)
{
  std::stringstream ss;
  ss << "Usage: tablator [options] input_file output_file\n"
     << visible_options;
  return ss.str ();
}

int main (int argc, char *argv[])
{
  bool stream_intermediate (false);
  tablator::Format input_format, output_format;

  // Declare the supported options.
  boost::program_options::options_description visible_options ("Options");
  visible_options.add_options ()("help", "produce help message")(
      "stream-intermediate", boost::program_options::value<bool>(),
      "stream the intermediate file (for testing only)")(
      "input-format", boost::program_options::value<std::string>(),
      "Input file format (json,json5,votable,csv,tsv,fits,ipac_table,"
      "text,html,hdf5)")(
      "output-format", boost::program_options::value<std::string>(),
      "Output file format (json,json5,votable,csv,tsv,fits,ipac_table,"
      "text,html,hdf5)");

  boost::program_options::options_description hidden_options (
      "Hidden options");
  hidden_options.add_options ()(
      "files", boost::program_options::value<std::vector<std::string> >(),
      "input and output file");

  boost::program_options::options_description options;
  options.add (visible_options).add (hidden_options);
  boost::program_options::positional_options_description positional_options;
  positional_options.add ("files", -1);

  boost::program_options::variables_map option_variables;
  try
    {
      boost::program_options::store (
          boost::program_options::command_line_parser (argc, argv)
              .options (options)
              .positional (positional_options)
              .run (),
          option_variables);
      boost::program_options::notify (option_variables);

      if (option_variables.count ("help"))
        {
          std::cout << usage (visible_options) << "\n";
          return 1;
        }
      if (option_variables.count ("stream-intermediate"))
        stream_intermediate
            = option_variables["stream-intermediate"].as<bool>();
      if (option_variables.count ("input-format"))
        input_format.init (option_variables["input-format"].as<std::string>());
      if (option_variables.count ("output-format"))
        output_format.init (
            option_variables["output-format"].as<std::string>());

      boost::filesystem::path input_path, output_path;
      if (option_variables.count ("files"))
        {
          std::vector<std::string> paths (
              option_variables["files"].as<std::vector<std::string> >());
          if (paths.size () == 1)
            {
              std::cerr << "Missing an output file\n";
              return 1;
            }
          if (paths.size () != 2)
            {
              std::cerr << "Too many filenames\n";
              return 1;
            }
          input_path = paths.at (0);
          output_path = paths.at (1);
          if (input_format.is_unknown ())
            input_format.set_from_extension (input_path);
          if (output_format.is_unknown ())
            output_format.set_from_extension (output_path);
        }
      else
        {
          std::cerr << "Missing an input and output file\n"
                    << usage (visible_options) << "\n";
          return 1;
        }


      if (stream_intermediate)
        {
          boost::filesystem::ifstream input_stream (input_path);
          tablator::Table table (input_stream, input_format);
          boost::filesystem::ofstream output_stream (output_path);
          table.write (output_stream, output_path.stem ().native (),
                       output_format);
        }
      else
        {
          tablator::Table table (input_path, input_format);
          table.write (output_path, output_format);
        }
    }
  catch (boost::program_options::error &exception)
    {
      std::cerr << exception.what () << "\n" << usage (visible_options);
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
  catch (std::exception &exception)
    {
      std::cerr << exception.what () << "\n";
      std::cerr.flush ();
      exit (1);
    }
}
