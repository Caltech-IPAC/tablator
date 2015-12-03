#include "../Format.hxx"

void tablator::Format::set_from_extension (const boost::filesystem::path &path)
{
  std::string extension = path.extension ().string ();
  if (!extension.empty ())
    extension=extension.substr (1);
  bool found = false;
  for (index = formats.begin (); index != formats.end (); ++index)
    {
      for (auto &e : index->second.second)
        {
          if (boost::iequals (e, extension))
            {
              found = true;
              break;
            }
        }
      if (found)
        break;
    }
  if (index == formats.end ())
    {
      if (extension.empty ())
        index = formats.find (enum_format::IPAC_TABLE);
      else
        throw std::runtime_error ("Unknown extension: " + extension);
    }
}
