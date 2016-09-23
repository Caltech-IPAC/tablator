/// This program is free software: you can redistribute it and/or modify
/// it under the terms of the GNU Lesser General Public License as published by
/// the Free Software Foundation, version 3 of the License.

/// This program is distributed in the hope that it will be useful,
/// but WITHOUT ANY WARRANTY; without even the implied warranty of
/// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
/// GNU Lesser General Public License for more details.

/// You should have received a copy of the GNU Lesser General Public License
/// along with this program.  If not, see <http://www.gnu.org/licenses/>.

/// Copied from libcsv++
/// https://github.com/jainyzau/libcsv-

#pragma once

#include <vector>
#include <list>
#include <string>
namespace CSV
{
  class CSV_Document
  {
  public:
    typedef std::string element_type;
    typedef std::vector<element_type> row_type;
    typedef std::list<row_type> document_type;
    typedef document_type::size_type row_index_type;
    typedef row_type::size_type column_index_type;
    typedef document_type::iterator iterator;
    typedef document_type::const_iterator const_iterator;

    CSV_Document(const std::string& file_path);
    bool empty() const { return document.empty(); }
    iterator begin() { return document.begin(); }
    iterator end() { return document.end(); }
    const_iterator begin() const { return document.begin(); }
    const_iterator end() const { return document.end(); }

    void add_row(const row_type& row) { document.push_back(row); }

    document_type document;
  };
}

