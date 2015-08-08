/// A simple function to return the mapping between FITS keywords and
/// UCD's.  It would be nice to do this with a static variable, but
/// that seems to cause problems on destruction, because the order of
/// static destruction is undefined, and the C++ runtime might be
/// destroyed before the static variable :(

#include <map>
#include <string>

namespace tablator
{
std::map<std::string, std::string> fits_keyword_mapping (const bool &reverse)
{
  std::map<std::string, std::string> mapping
      = { { "TELESCOP", "instr.obsty" },
          { "INSTRUME", "instr" },
          { "FREQ", "em.freq" },
          { "DETNAM", "instr.det" },
          { "OBJECT", "src" },
          { "OBJ_TYPE", "src.class" },
          { "OBJRA", "pos.eq.ra" },
          { "OBJDEC", "pos.eq.dec" },
          { "BUNIT", "meta.unit" },
          //  FIXME: These UCD's (radius, width, height) are completely
          //  made up.  We should probably convert it to a shape and use
          //  phys.angArea, but that would be annoying to convert both ways
          { "RADIUS", "pos.radius" },
          { "WIDTH", "pos.width" },
          { "HEIGHT", "pos.height" },
          { "OBJGLON", "pos.galactic.lon" },
          { "OBJGLAT", "pos.galactic.lat" },
          { "PROCVER", "meta.version" },
          /// From HEASARC recommendation
          /// http://heasarc.gsfc.nasa.gov/docs/heasarc/ofwg/ofwg_recomm.html
          { "CREATOR", "meta.software" },
          { "DATE", "time.creation" },
          { "ADQL", "meta.adql" } };
  if (!reverse)
    return mapping;
  std::map<std::string, std::string> reverse_mapping;
  for (auto &m : mapping)
    reverse_mapping.insert (std::make_pair (m.second, m.first));
  return reverse_mapping;
}
}
