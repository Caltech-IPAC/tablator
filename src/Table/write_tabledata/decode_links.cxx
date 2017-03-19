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
class Link_and_Prefix
{
public:
  std::string prefix, href, link;
};

class Entry
{
public:
  std::vector<Link_and_Prefix> links;
  std::string suffix;
};

std::ostream & operator<< (std::ostream &os,
                           const tablator::Link_and_Prefix &link)
{
  os << link.prefix << "<a href=\"" << link.href << "\">"
     << link.link << "</a>";
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

BOOST_FUSION_ADAPT_STRUCT (tablator::Link_and_Prefix,
                           (std::string, prefix)
                           (std::string, href)
                           (std::string, link))

BOOST_FUSION_ADAPT_STRUCT (tablator::Entry,
                           (std::vector<tablator::Link_and_Prefix>, links)
                           (std::string, suffix)
                           )

namespace tablator
{
std::string decode_links (const std::string &encoded)
{
  auto first (encoded.begin ()), last (encoded.end ());
  boost::spirit::qi::rule<std::string::const_iterator, std::string ()> href;

  boost::spirit::qi::rule<std::string::const_iterator, Link_and_Prefix ()>
    link_and_prefix;

  boost::spirit::qi::rule<std::string::const_iterator, Entry ()> entry_rule;
  
  href %= boost::spirit::qi::lit("href=&quot;")
    >> (*(boost::spirit::qi::char_ - boost::spirit::qi::char_('&')))
    >> boost::spirit::qi::lit("&quot;");

  link_and_prefix %=
    (*boost::spirit::qi::lexeme[boost::spirit::qi::char_
                                - boost::spirit::qi::char_('&')])
    >> boost::spirit::qi::lit("&lt;a")
    >> *boost::spirit::qi::omit[boost::spirit::ascii::space]
    >> href
    >> *boost::spirit::qi::omit[boost::spirit::ascii::space]
    >> boost::spirit::qi::lit("&gt;")
    >> (*boost::spirit::qi::lexeme[boost::spirit::qi::char_
                                   - boost::spirit::qi::char_('&')])
    >> boost::spirit::qi::lit("&lt;")
    >> boost::spirit::qi::lit("/a&gt;");

  entry_rule %= *link_and_prefix
    >> (*(boost::spirit::qi::char_ - boost::spirit::qi::char_('&')));
  
  Entry entry;
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
