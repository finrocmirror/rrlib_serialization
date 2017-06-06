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
/*!\file    rrlib/serialization/tRegisterUpdate.h
 *
 * \author  Max Reichardt
 *
 * \date    2017-06-04
 *
 * \brief   Contains tRegisterUpdate
 *
 * \b tRegisterUpdate
 *
 * When serialized to a stream, writes updates on desired register and all registers to be updated on change.
 * After deserialization, the reader's data on these remote registers is up to date.
 *
 */
//----------------------------------------------------------------------
#ifndef __rrlib__serialization__tRegisterUpdate_h__
#define __rrlib__serialization__tRegisterUpdate_h__

//----------------------------------------------------------------------
// External includes (system with <>, local with "")
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// Internal includes with ""
//----------------------------------------------------------------------
#include "rrlib/serialization/tInputStream.h"
#include "rrlib/serialization/tOutputStream.h"

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
//! Utility class to write updates on auto-published registers to a stream - and read them on receiver side.
/*!
 * When serialized to a stream, writes updates on desired register and all registers to be updated on change.
 * After deserialization, the reader's data on these remote registers is up to date.
 */
class tRegisterUpdate
{

//----------------------------------------------------------------------
// Public methods and typedefs
//----------------------------------------------------------------------
public:

  /**
   * \param register_uid UID of register to update (registers to updated on_chane are also updated)
   */
  tRegisterUpdate(int register_uid = 0) :
    register_uid(register_uid)
  {}

  /**
   * \return UID of register to update
   */
  int RegisterUid() const
  {
    return register_uid;
  }

//----------------------------------------------------------------------
// Private fields and methods
//----------------------------------------------------------------------
private:

  /*! UID of register to update (registers to updated on_change are also updated) */
  int register_uid;
};

inline tOutputStream& operator << (tOutputStream& stream, const tRegisterUpdate& update)
{
  if (!stream.WriteRegisterUpdates(update.RegisterUid(), std::numeric_limits<uint>::max(), 0))
  {
    stream.WriteByte(-1);
  }
  return stream;
}

inline tInputStream& operator >> (tInputStream& stream, tRegisterUpdate& update)
{
  stream.ReadRegisterUpdatesImplementation();
  return stream;
}

//----------------------------------------------------------------------
// End of namespace declaration
//----------------------------------------------------------------------
}
}


#endif
