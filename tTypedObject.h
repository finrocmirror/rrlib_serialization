/**
 * You received this file as part of an advanced experimental
 * robotics framework prototype ('finroc')
 *
 * Copyright (C) 2007-2010 Max Reichardt,
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

#ifndef __rrlib__serialization__tTypedObject_h__
#define __rrlib__serialization__tTypedObject_h__

#include "rrlib/serialization/tDataTypeBase.h"
#include "rrlib/serialization/tSerializable.h"

namespace rrlib
{
namespace serialization
{
/*!
 * \author Max Reichardt
 *
 * This is the abstract base class for any object that has additional
 * type information as provided in this package.
 *
 * Such classes can be cleanly serialized to the network
 *
 * C++ issue: Typed objects are not automatically jc objects!
 */
class tTypedObject : public tSerializable
{
protected:

  /*! Type of object */
  tDataTypeBase type;

public:

  /*!
   * \return Type of object
   */
  tTypedObject() :
      type(NULL)
  {}

  virtual ~tTypedObject() {}

  /*!
   * \return Log description (default implementation is "<class name> (<pointer>)"
   */
  inline const tTypedObject& GetLogDescription() const
  {
    return *this;
  }

  inline tDataTypeBase GetType() const
  {
    return type;
  }

};

} // namespace rrlib
} // namespace serialization

namespace rrlib
{
namespace serialization
{
inline std::ostream& operator << (std::ostream& output, const tTypedObject* lu)
{
  output << typeid(*lu).name() << " (" << ((void*)lu) << ")";
  return output;
}
inline std::ostream& operator << (std::ostream& output, const tTypedObject& lu)
{
  output << (&lu);
  return output;
}

} // namespace rrlib
} // namespace serialization

#endif // __rrlib__serialization__tTypedObject_h__
