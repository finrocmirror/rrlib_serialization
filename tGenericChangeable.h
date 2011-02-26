/**
 * You received this file as part of an advanced experimental
 * robotics framework prototype ('finroc')
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

#ifndef __rrlib__serialization__tGenericChangeable_h__
#define __rrlib__serialization__tGenericChangeable_h__

#include <stdint.h>

namespace rrlib
{
namespace serialization
{
/*!
 * \author Max Reichardt
 *
 * Interface for data types that can be changed using transactions T.
 */
template<typename T>
class tGenericChangeable
{
public:

  /*!
   * \param t Change/Transaction to apply
   * \param parameter1 Custom parameter (e.g. start index)
   * \param parameter2 Custom parameter 2 (e.g. length)
   */
  virtual void ApplyChange(T* t, int64_t parameter1, int64_t parameter2) = 0;

};

} // namespace rrlib
} // namespace serialization

#endif // __rrlib__serialization__tGenericChangeable_h__
