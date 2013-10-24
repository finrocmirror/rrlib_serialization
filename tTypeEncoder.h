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
/*!\file    rrlib/serialization/tTypeEncoder.h
 *
 * \author  Max Reichardt
 *
 * \date    2013-05-18
 *
 * \brief   Contains tTypeEncoder
 *
 * \b tTypeEncoder
 *
 * Class to encode and decode types in tInputStream and tOutputStream
 *
 */
//----------------------------------------------------------------------
#ifndef __rrlib__serialization__tTypeEncoder_h__
#define __rrlib__serialization__tTypeEncoder_h__

//----------------------------------------------------------------------
// External includes (system with <>, local with "")
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// Internal includes with ""
//----------------------------------------------------------------------
#include "rrlib/serialization/definitions.h"

//----------------------------------------------------------------------
// Namespace declaration
//----------------------------------------------------------------------
namespace rrlib
{

//----------------------------------------------------------------------
// Forward declarations / typedefs / enums
//----------------------------------------------------------------------
namespace rtti
{
class tType;
}

namespace serialization
{
class tInputStream;
class tOutputStream;

//----------------------------------------------------------------------
// Class declaration
//----------------------------------------------------------------------
//! Type encoder interface
/*!
 * Class to encode and decode types in tInputStream and tOutputStream
 */
class tTypeEncoder
{

//----------------------------------------------------------------------
// Public methods and typedefs
//----------------------------------------------------------------------
public:

  /*!
   * \param stream Input Stream
   * \return Type decoded from input stream
   */
  virtual rtti::tType ReadType(tInputStream& stream) = 0;

  /*!
   * \param stream Output stream
   * \param type Type to encode to output stream
   */
  virtual void WriteType(tOutputStream& stream, rtti::tType type) = 0;

};

//----------------------------------------------------------------------
// End of namespace declaration
//----------------------------------------------------------------------
}
}


#endif
