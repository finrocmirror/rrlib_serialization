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
#ifndef __rrlib__serialization__tCustomTypeInitialization_h__
#define __rrlib__serialization__tCustomTypeInitialization_h__

namespace rrlib
{
namespace serialization
{

/*!
 * \author Max Reichardt
 *
 * Allows to perform custom type initializations when tDataType<T> is
 * instantiated.
 *
 * For this, T should be a subclass of tCustomTypeInitialization
 * and provide a static function:
 *
 * template <typename T>
 * static void CustomTypeInitialization(tDataTypeBase dt, T* dummy);
 *
 * or
 *
 * static void CustomTypeInitialization(tDataTypeBase dt, void* dummy);
 *
 * T is that parameter T of tDataType.
 * dt is the untyped tDataTypeBase object (with uid set) for T.
 * dummy is a casted NULL pointer - to allow using void* when template
 * parameter T is not required.
 */
class tCustomTypeInitialization
{
};

} // namespace rrlib
} // namespace serialization

#endif // __rrlib__serialization__tCustomTypeInitialization_h__
