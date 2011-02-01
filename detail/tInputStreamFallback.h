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

#ifndef __rrlib__serialization__detail__tInputStreamFallback_h__
#define __rrlib__serialization__detail__tInputStreamFallback_h__

#include "rrlib/serialization/tOutputStream.h"
#include "rrlib/serialization/tInputStream.h"
#include "rrlib/serialization/tMemoryBuffer.h"
#include "rrlib/serialization/sSerialization.h"

namespace rrlib
{
namespace serialization
{
namespace detail
{
/*!
 * \author Max Reichardt
 *
 * Wrapper tInputStream to implement fallback mechanism
 * when operators are not overloaded for string streams.
 */
class tInputStreamFallback : public tInputStream
{
  std::shared_ptr<tMemoryBuffer> mb;
public:
  tInputStreamFallback(tStringInputStream& s) :
      tInputStream(),
      mb(new tMemoryBuffer(50000))
  {
    tOutputStream co(mb);
    sSerialization::ConvertHexStringToBinary(s, co);
    co.Close();
    Reset(mb.get());
  }
};

template <typename T>
inline tInputStream& operator>> (tInputStreamFallback is, T& t)
{
  is >> t;
  return is;
}

} // namespace
} // namespace
} // namespace

#endif // __rrlib__serialization__detail__tInputStreamFallback_h__
