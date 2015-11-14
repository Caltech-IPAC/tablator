#pragma once

namespace tablator
{
/// Type names are mostly lifted directly from the IVOA TAP spec.
/// IVOA has fixed length char[] arrays.  We just use a string.
enum class Type : char
{ BOOLEAN,
  SHORT,
  INT,
  LONG,
  FLOAT,
  DOUBLE,
  STRING };
}

