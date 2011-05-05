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

#ifndef __rrlib__serialization__sStaticFactory_h__
#define __rrlib__serialization__sStaticFactory_h__

namespace rrlib
{
namespace serialization
{

/*!
 * \author Max Reichardt
 *
 * This class defines how objects are instantiated when using tDataTypeBase
 * or tDefaultFactory to create objects.
 *
 * By default, the empty constructor is used.
 * The template may, however, be specialized to use alternative constructors.
 */
template <typename T>
struct sStaticFactory
{
  static T* Create(void* placement)
  {
    return new(placement) T();
  }

  static T CreateByValue()
  {
    return T();
  }
};

} // namespace rrlib
} // namespace serialization

#endif // __rrlib__serialization__sStaticFactory_h__
