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
/*!\file    rrlib/serialization/tStringOutputStream.h
 *
 * \author  Max Reichardt
 *
 * \date    2011-02-01
 *
 * \brief
 *
 */
//----------------------------------------------------------------------
#ifndef __rrlib__serialization__tStringOutputStream_h__
#define __rrlib__serialization__tStringOutputStream_h__

#include "rrlib/time/time.h"
#include "rrlib/serialization/tSerializable.h"
#include <boost/utility.hpp>
#include <string>
#include <sstream>

namespace rrlib
{
namespace serialization
{
/*!
 * \author Max Reichardt
 *
 * String output stream.
 * Used for completely serializing object to a string stream (UTF-8).
 */
class tStringOutputStream : public boost::noncopyable
{
public:

  /*! Wrapped string stream */
  std::ostringstream wrapped;

  template <typename T>
  tStringOutputStream& Append(const T& t)
  {
    *this << t;
    return *this;
  }

  tStringOutputStream() :
    wrapped()
  {
  }

  /*!
   * \param length Initial length of buffer (TODO: in C++ this currently has now effect)
   */
  tStringOutputStream(int length) :
    wrapped()
  {}

  /*!
   * Clear contents and reset
   */
  inline void Clear()
  {
    wrapped.str(std::string());
  }

  inline std::string ToString()
  {
    return wrapped.str();
  }

};

} // namespace rrlib
} // namespace serialization

#include "rrlib/serialization/detail/tStringOutputStreamFallback.h"

namespace rrlib
{
namespace serialization
{
inline tStringOutputStream& operator<< (tStringOutputStream& os, char t)
{
  os.wrapped << t;
  return os;
}
inline tStringOutputStream& operator<< (tStringOutputStream& os, int8_t t)
{
  os.wrapped << t;
  return os;
}
inline tStringOutputStream& operator<< (tStringOutputStream& os, int16_t t)
{
  os.wrapped << t;
  return os;
}
inline tStringOutputStream& operator<< (tStringOutputStream& os, int32_t t)
{
  os.wrapped << t;
  return os;
}
inline tStringOutputStream& operator<< (tStringOutputStream& os, long int t)
{
  os.wrapped << t;
  return os;
}
inline tStringOutputStream& operator<< (tStringOutputStream& os, long long int t)
{
  os.wrapped << t;
  return os;
}
inline tStringOutputStream& operator<< (tStringOutputStream& os, uint8_t t)
{
  os.wrapped << t;
  return os;
}
inline tStringOutputStream& operator<< (tStringOutputStream& os, uint16_t t)
{
  os.wrapped << t;
  return os;
}
inline tStringOutputStream& operator<< (tStringOutputStream& os, uint32_t t)
{
  os.wrapped << t;
  return os;
}
inline tStringOutputStream& operator<< (tStringOutputStream& os, unsigned long int t)
{
  os.wrapped << t;
  return os;
}
inline tStringOutputStream& operator<< (tStringOutputStream& os, unsigned long long int t)
{
  os.wrapped << t;
  return os;
}
inline tStringOutputStream& operator<< (tStringOutputStream& os, float t)
{
  os.wrapped << t;
  return os;
}
inline tStringOutputStream& operator<< (tStringOutputStream& os, double t)
{
  os.wrapped << t;
  return os;
}
inline tStringOutputStream& operator<< (tStringOutputStream& os, const char* t)
{
  os.wrapped << t;
  return os;
}
inline tStringOutputStream& operator<< (tStringOutputStream& os, bool t)
{
  os << (t ? "true" : "false");
  return os;
}
inline tStringOutputStream& operator<< (tStringOutputStream& os, const std::string& t)
{
  os.wrapped << t;
  return os;
}

inline tStringOutputStream& operator<< (tStringOutputStream& os, const tSerializable& t)
{
  t.Serialize(os);
  return os;
}
template <typename R, typename P>
inline tStringOutputStream& operator<< (tStringOutputStream& os, const std::chrono::duration<R, P>& t)
{
  os << rrlib::time::ToIsoString(std::chrono::duration_cast<rrlib::time::tDuration>(t));
  return os;
}
template <typename D>
inline tStringOutputStream& operator<< (tStringOutputStream& os, const std::chrono::time_point<std::chrono::system_clock, D>& t)
{
  os << rrlib::time::ToIsoString(t);
  return os;
}
template <typename ENUM>
inline tStringOutputStream& operator<< (typename std::enable_if<std::is_enum<ENUM>::value, tStringOutputStream&>::type os, const ENUM& t)
{
#ifdef _LIB_ENUM_STRINGS_PRESENT_
  const char* s = make_builder::GetEnumString(t);
  if (s != NULL)
  {
    os.wrapped << s << " ";
  }
#endif

  os.wrapped << "(" << static_cast<int>(t) << ")";
  return os;
}


} // namespace rrlib
} // namespace serialization

#endif // __rrlib__serialization__tStringOutputStream_h__
