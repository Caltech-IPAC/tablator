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

#include "../CSV_Document.hxx"
#include "CSV_Parser.hxx"

namespace tablator
{
  CSV_Document::CSV_Document(const std::string& file_path)
  {
    // why use a separate parser? 
    // Because parser uses some local variables, if we use a separate parser, all these 
    // variables are de-constructed after loading while parsed data is still valid.
    CSV_Parser parser(*this, file_path);
  }
}
