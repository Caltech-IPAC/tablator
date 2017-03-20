#include <string>
#include <vector>

#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/qi_string.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>

#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/phoenix_object.hpp>
#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/fusion/include/io.hpp>

namespace tablator
{
class Attribute
{
public:
  std::string key, value;
};

class Link_and_Prefix
{
public:
  std::string prefix, link;
  std::vector<Attribute> attributes;
};

class Entry
{
public:
  std::vector<Link_and_Prefix> links;
  std::string suffix;
};

std::ostream & operator<< (std::ostream &os,
                           const tablator::Attribute &attribute)
{
  os << attribute.key << "=\"" << attribute.value << "\"";
  return os;
}
  
std::ostream & operator<< (std::ostream &os,
                           const tablator::Link_and_Prefix &link)
{
  os << link.prefix << "<a";
  for (auto &attribute: link.attributes)
    { os << " " << attribute; }
  os << ">" << link.link << "</a>";
  return os;
}


std::ostream & operator<< (std::ostream &os, const tablator::Entry &entry)
{
  for (auto &link: entry.links)
    { os << link; }
  os << entry.suffix;
  return os;
}
}

BOOST_FUSION_ADAPT_STRUCT (tablator::Attribute,
                           (std::string, key)
                           (std::string, value))

BOOST_FUSION_ADAPT_STRUCT (tablator::Link_and_Prefix,
                           (std::string, prefix)
                           (std::vector<tablator::Attribute>, attributes)
                           (std::string, link))

BOOST_FUSION_ADAPT_STRUCT (tablator::Entry,
                           (std::vector<tablator::Link_and_Prefix>, links)
                           (std::string, suffix)
                           )

namespace tablator
{
std::string decode_links (const std::string &encoded)
{
  boost::spirit::qi::rule<std::string::const_iterator, std::string ()>
    quoted, single_quoted, double_quoted, attribute_key;

  single_quoted %= boost::spirit::qi::lit("&apos;")
    >> (*(boost::spirit::qi::char_ - "&apos;")) >> "&apos;";

  double_quoted %= boost::spirit::qi::lit("&quot;")
    >> (*(boost::spirit::qi::char_ - "&quot;")) >> "&quot;";

  quoted %= single_quoted | double_quoted;

  /// From https://developer.mozilla.org/en-US/docs/Web/HTML/Element/a
  /// Put hreflang in front of href so that we do not get a greedy
  /// match with href instead of hreflang.
  attribute_key %= boost::spirit::qi::string("download")
    | boost::spirit::qi::string("hreflang")
    | boost::spirit::qi::string("href")
    | boost::spirit::qi::string("referrerpolicy")
    | boost::spirit::qi::string("rel")
    | boost::spirit::qi::string("target")
    | boost::spirit::qi::string("type");

  boost::spirit::qi::rule<std::string::const_iterator, Attribute ()> attribute;
  attribute %= attribute_key >> "=" >> quoted;

  boost::spirit::qi::rule<std::string::const_iterator,
                          std::vector<Attribute> ()> attributes;
                          
  attributes %= attribute
    % *boost::spirit::qi::omit[boost::spirit::ascii::space];
  
  boost::spirit::qi::rule<std::string::const_iterator, Link_and_Prefix ()>
    link_and_prefix;
  link_and_prefix %=
    (*(boost::spirit::qi::char_ - "&lt;a")) >> "&lt;a"
    >> *boost::spirit::qi::omit[boost::spirit::ascii::space]
    >> attributes
    >> *boost::spirit::qi::omit[boost::spirit::ascii::space]
    >> "&gt;"
    >> (*(boost::spirit::qi::char_ - "&lt;")) >> "&lt;"
    >> "/a&gt;";

  boost::spirit::qi::rule<std::string::const_iterator, Entry ()> entry_rule;
  entry_rule %= *link_and_prefix >> (*(boost::spirit::qi::char_ - "&lt;a"));
  
  Entry entry;
  auto first (encoded.begin ()), last (encoded.end ());
  if (boost::spirit::qi::phrase_parse (first, last, entry_rule,
                                       boost::spirit::ascii::space, entry)
      && first == last)
    {
      std::stringstream ss;
      ss << entry;
      return ss.str ();
    }
  else
    {
      return encoded;
    }
}
}
