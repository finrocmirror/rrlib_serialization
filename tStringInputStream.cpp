//
// You received this file as part of RRLib
// Robotics Research Library
//
// Copyright (C) Finroc GbR (finroc.org)
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//
//----------------------------------------------------------------------
/*!\file    rrlib/serialization/tStringInputStream.cpp
 *
 * \author  Max Reichardt
 *
 * \date    2011-02-01
 *
 */
//----------------------------------------------------------------------
#include "rrlib/serialization/tStringInputStream.h"

#include <boost/algorithm/string/trim.hpp>
#include <boost/algorithm/string/predicate.hpp>

namespace rrlib
{
namespace serialization
{
int8_t tStringInputStream::char_map[256];
int string_input_stream_initializer = tStringInputStream::InitCharMap();

const int8_t tStringInputStream::cLCASE, tStringInputStream::cUCASE, tStringInputStream::cLETTER, tStringInputStream::cDIGIT, tStringInputStream::cWHITESPACE;

tStringInputStream::tStringInputStream(const std::string& s) :
  wrapped(s)
{
}

int tStringInputStream::InitCharMap()
{
  for (int i = 0; i < 256; i++)
  {
    int8_t mask = 0;
    if (islower(i))
    {
      mask |= cLCASE;
    }
    if (isupper(i))
    {
      mask |= cUCASE;
    }
    if (isalpha(i))
    {
      mask |= cLETTER;
    }
    if (isdigit(i))
    {
      mask |= cDIGIT;
    }
    if (isspace(i))
    {
      mask |= cWHITESPACE;
    }
    char_map[i] = mask;
  }
  return 0;
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

    if ((char_map[c] & stop_at_flags) != 0)
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

    if ((char_map[c] & valid_flags) == 0)
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

} // namespace rrlib
} // namespace serialization

