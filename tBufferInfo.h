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
/*!\file    rrlib/serialization/tBufferInfo.h
 *
 * \author  Max Reichardt
 *
 * \date    2013-05-17
 *
 * \brief   Contains tBufferInfo
 *
 * \b tBufferInfo
 *
 * Buffer information.
 * Used in binary stream classes, sources and sinks.
 *
 */
//----------------------------------------------------------------------
#ifndef __rrlib__serialization__tBufferInfo_h__
#define __rrlib__serialization__tBufferInfo_h__

//----------------------------------------------------------------------
// External includes (system with <>, local with "")
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// Internal includes with ""
//----------------------------------------------------------------------
#include "rrlib/serialization/tFixedBuffer.h"

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
//! Buffer information
/*!
 * Buffer information.
 * Used in binary stream classes, sources and sinks.
 * Sources and sinks manage such buffers (and respective buffer information).
 */
struct tBufferInfo
{

  /*! Buffer that is currently operated on */
  tFixedBuffer* buffer;

  /*! Start offset of buffer */
  size_t start;

  /*! End offset of buffer */
  size_t end;

  /*! Current read or write position */
  size_t position;

  /*! Custom data that can be filled by source/sink that manages this buffer */
  void* custom_data;


  tBufferInfo() :
    buffer(NULL),
    start(0),
    end(0),
    position(0),
    custom_data(NULL)
  {}

  /*!
   * \return Total size of buffer - as described by this BufferInfo object: end - start
   */
  inline size_t Capacity() const
  {
    return end - start;
  }

  /*!
   * \return Number of bytes to write (for Sinks)
   */
  inline size_t GetWriteLen() const
  {
    return position - start;
  }

  /*!
   * \return Remaining bytes in buffer
   */
  inline size_t Remaining() const
  {
    return end - position;
  }

  /*!
   * Reset info to default/null values
   */
  inline void Reset()
  {
    *this = tBufferInfo();
  }

  /*!
   * Set start and end position in buffer backend
   *
   * \param start Start position (inclusive)
   * \param end End position (exclusive
   */
  inline void SetRange(size_t start_, size_t end_)
  {
    this->start = start_;
    this->end = end_;
  }

};

//----------------------------------------------------------------------
// End of namespace declaration
//----------------------------------------------------------------------
}
}


#endif
