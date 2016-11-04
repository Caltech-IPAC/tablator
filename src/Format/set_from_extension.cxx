#include "../Format.hxx"

void tablator::Format::set_from_extension (const boost::filesystem::path &path,
                                           const Format::Enums &default_format)
{
  std::string extension = path.extension ().string ();
  if (extension.empty ())
    {
      enum_format = default_format;
    }
  else
    {
      extension = extension.substr (1);
      for (auto &f : formats)
        {
          for (auto &e : f.second.second)
            {
              if (boost::iequals (e, extension))
                {
                  enum_format = f.first;
                  break;
                }
            }
          if (enum_format != Enums::UNKNOWN)
            break;
        }
    }
}
