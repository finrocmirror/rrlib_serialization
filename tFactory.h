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

#ifndef __rrlib__serialization__tFactory_h__
#define __rrlib__serialization__tFactory_h__

#include <memory>

namespace rrlib
{
namespace serialization
{
class tDataTypeBase;

/*!
 * \author Max Reichardt
 *
 * Factory to create objects (as shared_ptr) needed in input streams or
 * during deep copy operations.
 *
 * When deserializing pointer list, for example, buffers are needed.
 *
 * It may be specialized for more efficient buffer management.
 */
class tFactory
{
public:

  // Create buffer and place it in provided shared pointer
  template <typename T>
  void CreateBuffer(std::shared_ptr<T>& ptr, tDataTypeBase dt)
  {
    ptr = std::dynamic_pointer_cast<T>(CreateBuffer(dt));
  }

  /*!
   * Create buffer
   *
   * \param dt Data type
   * \return Created buffer
   */
  virtual ::std::shared_ptr<void> CreateBuffer(tDataTypeBase dt) = 0;

};

} // namespace rrlib
} // namespace serialization

#endif // __rrlib__serialization__tFactory_h__
