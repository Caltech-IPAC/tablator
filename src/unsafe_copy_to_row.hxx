#pragma once

#include <algorithm>

template <typename T>
inline void unsafe_copy_to_row (const T &element, const size_t &offset,
                                char *row)
{
  // FIXME: This might be undefined, because element+1 is not
  // guaranteed to be valid
  std::copy (reinterpret_cast<const char *>(&element),
             reinterpret_cast<const char *>(&element + 1), row + offset);
}
