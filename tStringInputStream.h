/**
 * You received this file as part of RRLib serialization
 *
 * Copyright (C) 2011 Max Reichardt,
 *   Robotics Research Lab, University of Kaiserslautern
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#ifndef __rrlib__serialization__tStringInputStream_h__
#define __rrlib__serialization__tStringInputStream_h__

#include "rrlib/time/time.h"
#include "rrlib/serialization/tSerializable.h"
#include <boost/utility.hpp>
#include <boost/algorithm/string.hpp>
#include <stdint.h>
#include <string>
#include <vector>
#include <sstream>

namespace rrlib
{
namespace serialization
{

/*!
 * \author Max Reichardt
 *
 * String input stream.
 * Used for completely deserializing object from a string stream (UTF-8).
 */
class tStringInputStream : public boost::noncopyable
{
private:

  /*! Map with flags of all 256 UTF Characters */
  static int8_t char_map[256];

  /*!
   * Non-template core functionality of ReadEnum method
   */
  int ReadEnumHelper(const std::vector<const char*>* strings);

public:

  /*! Wrapped string stream */
  std::istringstream wrapped;

  /*! Constants for character flags */
  static const int8_t cLCASE = 1, cUCASE = 2, cLETTER = 4, cDIGIT = 8, cWHITESPACE = 16;

  tStringInputStream(const std::string& s);

  /*!
   * Initializes char map
   *
   * \return dummy value
   */
  static int InitCharMap();

  /*!
   * \return next character in stream (without advancing in stream). -1 when end of stream is reached
   */
  inline int Peek()
  {
    char result = wrapped.peek();
    if (wrapped.eof())
    {
      return -1;
    }
    return result;
  }

  std::string Trim(const std::string& s)
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

  /*!
   * \return next character in stream. -1 when end of stream is reached
   */
  inline int Read()
  {
    char result;
    wrapped.get(result);
    if (wrapped.eof())
    {
      return -1;
    }
    return result;
  }

  /*!
   * \return String until end of stream
   */
  inline std::string ReadAll()
  {
    return ReadUntil("", 0, false);
  }

  /*!
   * \return String util end of line
   */
  inline std::string ReadLine()
  {
    return ReadUntil("\n", 0, false);
  }

  /*!
   * Read characters until a "stop character" is encountered
   *
   * \param stop_at_chars List of "stop characters"
   * \param stop_at_flags Make all characters with specified flags "stop characters"
   * \param trim_whitespace Trim whitespace after reading?
   * \return String
   */
  std::string ReadUntil(const char* stop_at_chars, int stop_at_flags = 0, bool trim_whitespace = true);

  /*!
   * Read "valid" characters. Stops at "invalid" character
   *
   * \param valid_chars List of "valid characters"
   * \param valid_flags Make all characters with specified flags "valid characters"
   * \param trim_whitespace Trim whitespace after reading?
   * \return String
   */
  std::string ReadWhile(const char* valid_chars, int valid_flags = 0, bool trim_whitespace = true);

  /*!
   * Put read character back to stream
   */
  inline void Unget()
  {
    wrapped.unget();
  }

  /*!
   * Read enum constant from string stream
   *
   * \return enum constant
   */
  template <typename ENUM>
  ENUM ReadEnum()
  {
#ifdef _LIB_ENUM_STRINGS_PRESENT_
    return static_cast<ENUM>(ReadEnumHelper(make_builder::GetEnumStrings<ENUM>()));
#else
    return static_cast<ENUM>(ReadEnumHelper(NULL));
#endif
  }
};

} // namespace rrlib
} // namespace serialization

#include "rrlib/serialization/detail/tStringInputStreamFallback.h"

namespace rrlib
{
namespace serialization
{
inline tStringInputStream& operator>> (tStringInputStream& is, char& t)
{
  is.wrapped >> t;
  return is;
}
inline tStringInputStream& operator>> (tStringInputStream& is, int8_t& t)
{
  is.wrapped  >> t;
  return is;
}
inline tStringInputStream& operator>> (tStringInputStream& is, int16_t& t)
{
  is.wrapped  >> t;
  return is;
}
inline tStringInputStream& operator>> (tStringInputStream& is, int32_t& t)
{
  is.wrapped  >> t;
  return is;
}
inline tStringInputStream& operator>> (tStringInputStream& is, long int& t)
{
  is.wrapped  >> t;
  return is;
}
inline tStringInputStream& operator>> (tStringInputStream& is, long long int& t)
{
  is.wrapped  >> t;
  return is;
}
inline tStringInputStream& operator>> (tStringInputStream& is, uint8_t& t)
{
  is.wrapped  >> t;
  return is;
}
inline tStringInputStream& operator>> (tStringInputStream& is, uint16_t& t)
{
  is.wrapped  >> t;
  return is;
}
inline tStringInputStream& operator>> (tStringInputStream& is, uint32_t& t)
{
  is.wrapped  >> t;
  return is;
}
inline tStringInputStream& operator>> (tStringInputStream& is, unsigned long int& t)
{
  is.wrapped  >> t;
  return is;
}
inline tStringInputStream& operator>> (tStringInputStream& is, unsigned long long int& t)
{
  is.wrapped  >> t;
  return is;
}
inline tStringInputStream& operator>> (tStringInputStream& is, float& t)
{
  is.wrapped  >> t;
  return is;
}
inline tStringInputStream& operator>> (tStringInputStream& is, double& t)
{
  is.wrapped  >> t;
  return is;
}
inline tStringInputStream& operator>> (tStringInputStream& is, bool& t)
{
  std::string s = is.ReadWhile("", tStringInputStream::cLETTER | tStringInputStream::cDIGIT | tStringInputStream::cWHITESPACE, true);
  t = boost::iequals(s, "true") || (s.length() == 1 && s[0] == '1');
  return is;
}
inline tStringInputStream& operator>> (tStringInputStream& is, std::vector<bool>::reference t)  // for std::vector<bool> support
{
  bool b;
  is >> b;
  t = b;
  return is;
}
inline tStringInputStream& operator>> (tStringInputStream& is, std::string& t)
{
  t = is.ReadLine();
  return is;
}
template <typename R, typename P>
inline tStringInputStream& operator>> (tStringInputStream& is, std::chrono::duration<R, P>& t)
{
  t = rrlib::time::ParseIsoDuration(is.ReadLine());
  return is;
}
template <typename D>
inline tStringInputStream& operator>> (tStringInputStream& is, std::chrono::time_point<std::chrono::system_clock, D>& t)
{
  t = rrlib::time::ParseIsoTimestamp(is.ReadLine());
  return is;
}
inline tStringInputStream& operator>> (tStringInputStream& is, tSerializable& t)
{
  t.Deserialize(is);
  return is;
}

template <typename ENUM>
inline tStringInputStream& operator>> (typename std::enable_if<std::is_enum<ENUM>::value, tStringInputStream&>::type is, ENUM& t)
{
  tStringInputStream& is2 = is;
  t = is2.ReadEnum<ENUM>();
  return is;
}

} // namespace rrlib
} // namespace serialization

#endif // __rrlib__serialization__tStringInputStream_h__
