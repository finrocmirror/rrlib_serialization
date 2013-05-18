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
/*!\file    rrlib/serialization/tStackMemoryBuffer.h
 *
 * \author  Max Reichardt
 *
 * \date    2013-05-17
 *
 * \brief   Contains tStackMemoryBuffer
 *
 * \b tStackMemoryBuffer
 *
 * Memory buffer with initial buffer allocated from stack (with size SIZE)
 *
 */
//----------------------------------------------------------------------
#ifndef __rrlib__serialization__tStackMemoryBuffer_h__
#define __rrlib__serialization__tStackMemoryBuffer_h__

//----------------------------------------------------------------------
// External includes (system with <>, local with "")
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// Internal includes with ""
//----------------------------------------------------------------------
#include "rrlib/serialization/tMemoryBuffer.h"

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
//! Memory buffer initially allocated on the stack
/*!
 * Memory buffer with initial buffer allocated from stack (with size SIZE)
 * Should not be moved!
 */
template <size_t SIZE>
class tStackMemoryBuffer : public tMemoryBuffer
{

//----------------------------------------------------------------------
// Public methods and typedefs
//----------------------------------------------------------------------
public:

  tStackMemoryBuffer(float resize_factor = cDEFAULT_RESIZE_FACTOR, bool empty = false) :
    tMemoryBuffer(initial_buffer, SIZE, empty),
    initial_buffer()
  {
    this->SetResizeReserveFactor(resize_factor);
  }

//----------------------------------------------------------------------
// Private fields and methods
//----------------------------------------------------------------------
private:

  /*! Memory of buffer initial buffer */
  char initial_buffer[SIZE];

};

//----------------------------------------------------------------------
// End of namespace declaration
//----------------------------------------------------------------------
}
}


#endif
