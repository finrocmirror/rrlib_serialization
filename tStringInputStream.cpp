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
/*!\file    rrlib/serialization/tStringInputStream.cpp
 *
 * \author  Max Reichardt
 *
 * \date    2013-05-18
 *
 */
//----------------------------------------------------------------------
#include "rrlib/serialization/tStringInputStream.h"

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

namespace internal
{

static uint8_t char_map[256];

int InitCharMap()
{
  for (int i = 0; i < 256; i++)
  {
    int mask = 0;
    if (islower(i))
    {
      mask |= tStringInputStream::cLCASE;
    }
    if (isupper(i))
    {
      mask |= tStringInputStream::cUCASE;
    }
    if (isalpha(i))
    {
      mask |= tStringInputStream::cLETTER;
    }
    if (isdigit(i))
    {
      mask |= tStringInputStream::cDIGIT;
    }
    if (isspace(i))
    {
      mask |= tStringInputStream::cWHITESPACE;
    }
    char_map[i] = static_cast<uint8_t>(mask);
  }
  return 0;
}

int string_input_stream_initializer = InitCharMap();

}

const int tStringInputStream::cLCASE, tStringInputStream::cUCASE, tStringInputStream::cLETTER,
      tStringInputStream::cDIGIT, tStringInputStream::cWHITESPACE;

tStringInputStream::tStringInputStream(const std::string& s) :
  wrapped(s)
{
}

std::string tStringInputStream::ReadUntil(const char* stop_at_chars, int stop_at_flags, bool trim_whitespace)
{
  std::ostringstream sb;
  size_t valid_char_len = strlen(stop_at_chars);
  const char* ca = stop_at_chars;
  while (true)
  {
    int c = Read();
    if (c == -1)
    {
      break;
    }

    if ((internal::char_map[c] & stop_at_flags) != 0)
    {
      Unget();
      break;
    }

    bool stop = false;
    for (size_t i = 0u; i < valid_char_len; i++)
    {
      if (c == ca[i])
      {
        stop = true;
        break;
      }
    }
    if (stop)
    {
      Unget();
      break;
    }

    sb << ((char)c);

  }

  if (trim_whitespace)
  {
    return Trim(sb.str());
  }

  return sb.str();
}

std::string tStringInputStream::ReadWhile(const char* valid_chars, int valid_flags, bool trim_whitespace)
{
  std::ostringstream sb;
  size_t valid_char_len = strlen(valid_chars);
  const char* ca = valid_chars;
  while (true)
  {
    int c = Read();
    if (c == -1)
    {
      break;
    }

    if ((internal::char_map[c] & valid_flags) == 0)
    {
      bool valid = false;
      for (size_t i = 0u; i < valid_char_len; i++)
      {
        if (c == ca[i])
        {
          valid = true;
          break;
        }
      }
      if (!valid)
      {
        Unget();
        break;
      }
    }

    sb << ((char)c);

  }

  if (trim_whitespace)
  {
    return Trim(sb.str());
  }

  return sb.str();
}

bool tStringInputStream::StringsEqualIgnoreCase(const std::string& string1, const std::string& string2)
{
  if (string1.length() != string2.length())
  {
    return false;
  }
  for (size_t i = 0; i < string1.length(); i++)
  {
    if (string1[i] != string2[i] && tolower(string1[i]) != tolower(string2[i]))
    {
      return false;
    }
  }
  return true;
}

std::string tStringInputStream::Trim(const std::string& s)
{
  std::string result;
  size_t len = s.size();
  size_t st = 0;

  while ((st < len) && (isspace(s[st])))
  {
    st++;
  }
  while ((st < len) && (isspace(s[len - 1])))
  {
    len--;
  }
  return ((st > 0) || (len < s.size())) ? s.substr(st, len - st) : s;
}

//----------------------------------------------------------------------
// End of namespace declaration
//----------------------------------------------------------------------
}
}
