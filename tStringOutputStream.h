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
/*!\file    rrlib/serialization/tStringOutputStream.h
 *
 * \author  Max Reichardt
 *
 * \date    2013-05-18
 *
 * \brief   Contains tStringOutputStream
 *
 * \b tStringOutputStream
 *
 * String output stream.
 * Used for completely serializing objects to a (this) character stream.
 *
 */
//----------------------------------------------------------------------
#ifndef __rrlib__serialization__tStringOutputStream_h__
#define __rrlib__serialization__tStringOutputStream_h__

//----------------------------------------------------------------------
// External includes (system with <>, local with "")
//----------------------------------------------------------------------
#include <sstream>
#include "rrlib/time/time.h"
#include "rrlib/util/tEnumBasedFlags.h"

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
//! String output stream
/*!
 * String output stream.
 * Used for completely serializing objects to a (this) character stream.
 */
class tStringOutputStream : public util::tNoncopyable
{

//----------------------------------------------------------------------
// Public methods and typedefs
//----------------------------------------------------------------------
public:

  /*!
   * \param preallocate Number of bytes to preallocate for stream (has no effect in current implementation)
   */
  tStringOutputStream(size_t preallocate = 0) : wrapped() {}

  template <typename T>
  tStringOutputStream& Append(const T& t)
  {
    *this << t;
    return *this;
  }

  /*!
   * Clear contents and reset
   */
  inline void Clear()
  {
    wrapped.str(std::string());
  }

  /*!
   * \return Wrapped string stream
   */
  inline std::ostringstream& GetWrappedStringStream()
  {
    return wrapped;
  }

  /*!
   * \return Text content that has been written to this stream
   */
  inline std::string ToString()
  {
    return wrapped.str();
  }

  /*!
   * Writes floating point value with the required number of digits to deserialize the exact same value later
   *
   * \param d floating point value
   */
  void WriteFloatingPoint(double d);
  void WriteFloatingPoint(float f);

//----------------------------------------------------------------------
// Private fields and methods
//----------------------------------------------------------------------
private:

  /*! Wrapped string stream */
  std::ostringstream wrapped;

};

//----------------------------------------------------------------------
// End of namespace declaration
//----------------------------------------------------------------------
}
}

#include "rrlib/serialization/detail/tStringOutputStreamFallback.h"

namespace rrlib
{
namespace serialization
{
inline tStringOutputStream& operator<< (tStringOutputStream& stream, char t)
{
  stream.GetWrappedStringStream() << t;
  return stream;
}
inline tStringOutputStream& operator<< (tStringOutputStream& stream, int8_t t)
{
  stream.GetWrappedStringStream() << t;
  return stream;
}
inline tStringOutputStream& operator<< (tStringOutputStream& stream, int16_t t)
{
  stream.GetWrappedStringStream() << t;
  return stream;
}
inline tStringOutputStream& operator<< (tStringOutputStream& stream, int t)
{
  stream.GetWrappedStringStream() << t;
  return stream;
}
inline tStringOutputStream& operator<< (tStringOutputStream& stream, long int t)
{
  stream.GetWrappedStringStream() << t;
  return stream;
}
inline tStringOutputStream& operator<< (tStringOutputStream& stream, long long int t)
{
  stream.GetWrappedStringStream() << t;
  return stream;
}
inline tStringOutputStream& operator<< (tStringOutputStream& stream, uint8_t t)
{
  stream.GetWrappedStringStream() << t;
  return stream;
}
inline tStringOutputStream& operator<< (tStringOutputStream& stream, uint16_t t)
{
  stream.GetWrappedStringStream() << t;
  return stream;
}
inline tStringOutputStream& operator<< (tStringOutputStream& stream, unsigned int t)
{
  stream.GetWrappedStringStream() << t;
  return stream;
}
inline tStringOutputStream& operator<< (tStringOutputStream& stream, unsigned long int t)
{
  stream.GetWrappedStringStream() << t;
  return stream;
}
inline tStringOutputStream& operator<< (tStringOutputStream& stream, unsigned long long int t)
{
  stream.GetWrappedStringStream() << t;
  return stream;
}
inline tStringOutputStream& operator<< (tStringOutputStream& stream, float t)
{
  stream.WriteFloatingPoint(t);
  return stream;
}
inline tStringOutputStream& operator<< (tStringOutputStream& stream, double t)
{
  stream.WriteFloatingPoint(t);
  return stream;
}
inline tStringOutputStream& operator<< (tStringOutputStream& stream, const char* t)
{
  stream.GetWrappedStringStream() << t;
  return stream;
}
inline tStringOutputStream& operator<< (tStringOutputStream& stream, bool t)
{
  stream << (t ? "true" : "false");
  return stream;
}
inline tStringOutputStream& operator<< (tStringOutputStream& stream, const std::string& t)
{
  stream.GetWrappedStringStream() << t;
  return stream;
}

template <typename R, typename P>
inline tStringOutputStream& operator<< (tStringOutputStream& stream, const std::chrono::duration<R, P>& t)
{
  stream << rrlib::time::ToIsoString(std::chrono::duration_cast<rrlib::time::tDuration>(t));
  return stream;
}
template <typename D>
inline tStringOutputStream& operator<< (tStringOutputStream& stream, const std::chrono::time_point<std::chrono::system_clock, D>& t)
{
  stream << rrlib::time::ToIsoString(t);
  return stream;
}
template <typename ENUM>
inline tStringOutputStream& operator<< (typename std::enable_if<std::is_enum<ENUM>::value, tStringOutputStream&>::type stream, const ENUM& t)
{
#ifdef _LIB_ENUM_STRINGS_PRESENT_
  const char* s = make_builder::GetEnumString(t);
  if (s != NULL)
  {
    stream.GetWrappedStringStream() << s << " ";
  }
#endif

  stream.GetWrappedStringStream() << "(" << static_cast<typename std::underlying_type<ENUM>::type>(t) << ")";
  return stream;
}
#ifdef _LIB_ENUM_STRINGS_PRESENT_
template <typename TFlag, typename TStorage>
inline tStringOutputStream& operator<< (tStringOutputStream& stream, const util::tEnumBasedFlags<TFlag, TStorage>& flags)
{
  auto enum_strings = make_builder::GetEnumStrings<TFlag>();
  uint flag_count = 0;
  for (size_t i = 0, n = make_builder::GetEnumStringsDimension<TFlag>(); i < n; i++)
  {
    if (flags.Get(static_cast<TFlag>(i)))
    {
      stream << (flag_count ? ", " : "") << enum_strings[i] << " (" << i << ")";
      flag_count++;
    }
  }
  return stream;
}
#endif


} // namespace rrlib
} // namespace serialization


#endif
