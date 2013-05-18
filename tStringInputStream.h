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
/*!\file    rrlib/serialization/tStringInputStream.h
 *
 * \author  Max Reichardt
 *
 * \date    2013-05-18
 *
 * \brief   Contains tStringInputStream
 *
 * \b tStringInputStream
 *
 * String input stream.
 * Used for completely deserializing objects from their string representation.
 *
 */
//----------------------------------------------------------------------
#ifndef __rrlib__serialization__tStringInputStream_h__
#define __rrlib__serialization__tStringInputStream_h__

//----------------------------------------------------------------------
// External includes (system with <>, local with "")
//----------------------------------------------------------------------
#include <sstream>
#include "rrlib/logging/messages.h"
#include "rrlib/util/tNoncopyable.h"

//----------------------------------------------------------------------
// Internal includes with ""
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
// Class declaration
//----------------------------------------------------------------------
//! String input stream
/*!
 * String input stream.
 * Used for completely deserializing objects from their string representation.
 */
class tStringInputStream : public util::tNoncopyable
{

//----------------------------------------------------------------------
// Public methods and typedefs
//----------------------------------------------------------------------
public:

  /*! Constants for character flags */
  static const int cLCASE = 1, cUCASE = 2, cLETTER = 4, cDIGIT = 8, cWHITESPACE = 16;

  tStringInputStream(const std::string& s);


  /*!
   * \return Wrapped string stream
   */
  inline std::istringstream& GetWrappedStringStream()
  {
    return wrapped;
  }

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
   * Read enum constant from string stream
   *
   * \return enum constant
   */
  template <typename ENUM>
  ENUM ReadEnum()
  {
    // parse input
    std::string enum_string(ReadWhile("", cDIGIT | cLETTER | cWHITESPACE, true));
    enum_string = Trim(enum_string);
    int c1 = Read();
    std::string num_string;
    if (c1 == '(')
    {
      num_string = ReadUntil(")");
      num_string = Trim(num_string);
      int c2 = Read();
      if (c2 != ')')
      {
        throw std::invalid_argument("Did not read expected bracket");
      }
    }

#ifdef _LIB_ENUM_STRINGS_PRESENT_
    // deal with input
    if (enum_string.length() > 0)
    {
      try
      {
        return make_builder::GetEnumValueFromString<ENUM>(enum_string);
      }
      catch (std::runtime_error &)
      {
        RRLIB_LOG_PRINTF(WARNING, "Could not find enum constant for string '%s'. Trying number '%s'", enum_string.c_str(), num_string.c_str());
      }
    }
#endif

    if (num_string.length() > 0)
    {
      int n = atoi(num_string.c_str());

#ifdef _LIB_ENUM_STRINGS_PRESENT_
      size_t enum_strings_dimension = make_builder::GetEnumStringsDimension<ENUM>();
      if (n >= static_cast<int64_t>(enum_strings_dimension))
      {
        RRLIB_LOG_PRINTF(ERROR, "Number %d out of range for enum (%d)", n, enum_strings_dimension);
        throw std::invalid_argument("Number out of range");
      }
#endif

      return static_cast<ENUM>(n);
    }
    throw std::invalid_argument("Could not parse enum string '" + enum_string + "'");
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
   * (string utility function - not particularly optimized/efficient)
   * Compares two string ignoring case
   *
   * \param string1 First string to compare
   * \param string2 Second string to compare
   * \return True if both strings have the same characters (ignoring case)
   */
  static bool StringsEqualIgnoreCase(const std::string& string1, const std::string& string2);

  /*!
   * (string utility function - not particularly optimized/efficient)
   * Trims specified string by cutting off white space at the front and at the back
   *
   * \param s String to trim
   * \return Trimmed string
   */
  static std::string Trim(const std::string& s);

  /*!
   * Put read character back to stream
   */
  inline void Unget()
  {
    wrapped.unget();
  }

//----------------------------------------------------------------------
// Private fields and methods
//----------------------------------------------------------------------
private:

  /*! Wrapped string stream */
  std::istringstream wrapped;
};

//----------------------------------------------------------------------
// End of namespace declaration
//----------------------------------------------------------------------
}
}

#include "rrlib/serialization/detail/tStringInputStreamFallback.h"

// stream operators for various standard types

namespace rrlib
{
namespace serialization
{
inline tStringInputStream& operator>> (tStringInputStream& stream, char& t)
{
  stream.GetWrappedStringStream() >> t;
  return stream;
}
inline tStringInputStream& operator>> (tStringInputStream& stream, int8_t& t)
{
  stream.GetWrappedStringStream() >> t;
  return stream;
}
inline tStringInputStream& operator>> (tStringInputStream& stream, int16_t& t)
{
  stream.GetWrappedStringStream() >> t;
  return stream;
}
inline tStringInputStream& operator>> (tStringInputStream& stream, int32_t& t)
{
  stream.GetWrappedStringStream() >> t;
  return stream;
}
inline tStringInputStream& operator>> (tStringInputStream& stream, long int& t)
{
  stream.GetWrappedStringStream() >> t;
  return stream;
}
inline tStringInputStream& operator>> (tStringInputStream& stream, long long int& t)
{
  stream.GetWrappedStringStream() >> t;
  return stream;
}
inline tStringInputStream& operator>> (tStringInputStream& stream, uint8_t& t)
{
  stream.GetWrappedStringStream() >> t;
  return stream;
}
inline tStringInputStream& operator>> (tStringInputStream& stream, uint16_t& t)
{
  stream.GetWrappedStringStream() >> t;
  return stream;
}
inline tStringInputStream& operator>> (tStringInputStream& stream, uint32_t& t)
{
  stream.GetWrappedStringStream() >> t;
  return stream;
}
inline tStringInputStream& operator>> (tStringInputStream& stream, unsigned long int& t)
{
  stream.GetWrappedStringStream() >> t;
  return stream;
}
inline tStringInputStream& operator>> (tStringInputStream& stream, unsigned long long int& t)
{
  stream.GetWrappedStringStream() >> t;
  return stream;
}
inline tStringInputStream& operator>> (tStringInputStream& stream, float& t)
{
  stream.GetWrappedStringStream() >> t;
  return stream;
}
inline tStringInputStream& operator>> (tStringInputStream& stream, double& t)
{
  stream.GetWrappedStringStream() >> t;
  return stream;
}
inline tStringInputStream& operator>> (tStringInputStream& stream, bool& t)
{
  std::string s = stream.ReadWhile("", tStringInputStream::cLETTER | tStringInputStream::cDIGIT | tStringInputStream::cWHITESPACE, true);
  t = tStringInputStream::StringsEqualIgnoreCase(s, "true") || (s.length() == 1 && s[0] == '1');
  return stream;
}
inline tStringInputStream& operator>> (tStringInputStream& stream, std::vector<bool>::reference t)  // for std::vector<bool> support
{
  bool b;
  stream >> b;
  t = b;
  return stream;
}
inline tStringInputStream& operator>> (tStringInputStream& stream, std::string& t)
{
  t = stream.ReadLine();
  return stream;
}
template <typename R, typename P>
inline tStringInputStream& operator>> (tStringInputStream& stream, std::chrono::duration<R, P>& t)
{
  t = rrlib::time::ParseIsoDuration(stream.ReadLine());
  return stream;
}
template <typename D>
inline tStringInputStream& operator>> (tStringInputStream& stream, std::chrono::time_point<std::chrono::system_clock, D>& t)
{
  t = rrlib::time::ParseIsoTimestamp(stream.ReadLine());
  return stream;
}

template <typename ENUM>
inline tStringInputStream& operator>> (typename std::enable_if<std::is_enum<ENUM>::value, tStringInputStream&>::type stream, ENUM& t)
{
  tStringInputStream& stream_reference = stream;
  t = stream_reference.ReadEnum<ENUM>();
  return stream;
}

} // namespace rrlib
} // namespace serialization


#endif
