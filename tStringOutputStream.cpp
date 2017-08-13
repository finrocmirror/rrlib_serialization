//
// You received this file as part of RRLib
// Robotics Research Library
//
// Copyright (C) Finroc GbR (finroc.org)
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along
// with this program; if not, write to the Free Software Foundation, Inc.,
// 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
//
//----------------------------------------------------------------------
/*!\file    rrlib/serialization/tStringOutputStream.cpp
 *
 * \author  Max Reichardt
 *
 * \date    2017-08-13
 *
 */
//----------------------------------------------------------------------
#include "rrlib/serialization/tStringOutputStream.h"

//----------------------------------------------------------------------
// External includes (system with <>, local with "")
//----------------------------------------------------------------------
#include <cstring>

//----------------------------------------------------------------------
// Internal includes with ""
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// Debugging
//----------------------------------------------------------------------
#include <cassert>

//----------------------------------------------------------------------
// Namespace usage
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// Namespace declaration
//----------------------------------------------------------------------
namespace rrlib
{
namespace serialization
{

//----------------------------------------------------------------------
// Forward declarations / typedefs / enums
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// Const values
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// Implementation
//----------------------------------------------------------------------

namespace
{

const char* cFLOAT_FORMAT_STRING = "%1.9g";
const char* cDOUBLE_FORMAT_STRING = "%1.17g";

inline float Parse(const char* buffer, float& result)
{
  char* end;
  result = strtof(buffer, &end);
  return result;
}

inline double Parse(const char* buffer, double& result)
{
  result = atof(buffer);
  return result;
}

template <typename T>
inline void WriteFloatingPointImplementation(std::ostream& stream, T value)
{
  std::stringstream output_stream;
  output_stream << value;

  T test_value;
  Parse(output_stream.str().c_str(), test_value);
  if (test_value == value || std::isnan(value))
  {
    stream << output_stream.str();
    return;
  }

  enum { cBUFFER_SIZE = 40 };
  char buffer[cBUFFER_SIZE];
  char buffer2[cBUFFER_SIZE];
  char exponent_buffer[cBUFFER_SIZE];

  sprintf(buffer, sizeof(T) <= 4 ? cFLOAT_FORMAT_STRING : cDOUBLE_FORMAT_STRING, value);
  size_t base_string_length = strlen(buffer);
  char* exponent = strchr(buffer, 'e');
  if (exponent)
  {
    strcpy(exponent_buffer, exponent);
    base_string_length = exponent - buffer;
  }
  char* working_buffer = buffer;
  char* other_buffer = buffer2;

  while (base_string_length > 3)
  {
    memcpy(other_buffer, working_buffer, base_string_length);

    char* last_char = &other_buffer[base_string_length - 1];
    char* prev_char = &other_buffer[base_string_length - 2];
    while ((*last_char) >= '5' && (*prev_char) == '9' && base_string_length > 3)
    {
      last_char--;
      prev_char--;
      base_string_length--;
    }
    (*prev_char) = (*last_char) >= '5' ? ((*prev_char) + 1) : (*prev_char);
    if (exponent)
    {
      memcpy(last_char, exponent_buffer, base_string_length);
    }
    else
    {
      last_char = 0;
    }
    base_string_length--;

    Parse(other_buffer, test_value);
    if (test_value != value)
    {
      break;
    }
    std::swap(other_buffer, working_buffer);
  }
  stream << working_buffer;
  assert(Parse(working_buffer, test_value) == value);
  return;
}

}

void tStringOutputStream::WriteFloatingPoint(double d)
{
  WriteFloatingPointImplementation(wrapped, d);
}

void tStringOutputStream::WriteFloatingPoint(float f)
{
  WriteFloatingPointImplementation(wrapped, f);
}


//----------------------------------------------------------------------
// End of namespace declaration
//----------------------------------------------------------------------
}
}
