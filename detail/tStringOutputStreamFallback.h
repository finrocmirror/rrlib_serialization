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
#ifndef __rrlib__serialization__detail__tStringOutputStreamFallback_h__
#define __rrlib__serialization__detail__tStringOutputStreamFallback_h__

namespace rrlib
{
namespace serialization
{
namespace detail
{
/*!
 * \author Max Reichardt
 *
 * Wrapper tStringOutputStream to implement fallback mechanism
 * when operators are not overloaded for XML nodes.
 */
class tStringOutputStreamFallback : public tStringOutputStream
{
public:
  xml::tNode& wrapped;

  tStringOutputStreamFallback(xml::tNode& node) :
    wrapped(node)
  {}

  virtual ~tStringOutputStreamFallback()
  {
    wrapped.SetContent(ToString());
  }
};

template <typename T>
struct tIsStringOutputSerializable
{
  template < typename U = T >
  static int16_t Test(decltype((*(tStringOutputStream*)(NULL)) << (*(U*)(NULL))))
  {
    return 0;
  }

  static int32_t Test(...)
  {
    return 0;
  }

  enum { value = sizeof(Test(*(tStringOutputStream*)(NULL))) == sizeof(int16_t) };
};

} // namespace
} // namespace
} // namespace

template < typename T, bool ENABLE = (!std::is_base_of<rrlib::serialization::tSerializable, T>::value) && rrlib::serialization::detail::tIsStringOutputSerializable<T>::value >
inline typename std::enable_if<ENABLE, rrlib::xml::tNode>::type & operator<< (rrlib::serialization::detail::tStringOutputStreamFallback && os, const T & t)
{
  os << t;
  return os.wrapped;
}

#endif // __rrlib__serialization__detail__tStringOutputStreamFallback_h__
