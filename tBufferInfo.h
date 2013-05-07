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
 * \date    2011-02-01
 *
 * \brief
 *
 */
//----------------------------------------------------------------------
#ifndef __rrlib__serialization__tBufferInfo_h__
#define __rrlib__serialization__tBufferInfo_h__

#include <cstddef>

namespace rrlib
{
namespace serialization
{
class tFixedBuffer;

/*!
 * Buffer information
 * (can be passed to and modified by Manager (by reference))
 */
struct tBufferInfo
{
public:

  /*! Buffer that read view currently operates on */
  tFixedBuffer* buffer;

  /*! Start of buffer */
  size_t start;

  /*! End of buffer */
  size_t end;

  /*! Current read or write position */
  size_t position;

  /*! Custom data that can be filled by source/sink that manages this buffer */
  void* custom_data;

  /*!
   * \param other Other Buffer Info to copy values from
   */
  inline void Assign(const tBufferInfo& other)
  {
    buffer = other.buffer;
    start = other.start;
    end = other.end;
    position = other.position;
    custom_data = other.custom_data;
  }

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
    buffer = NULL;
    start = 0u;
    end = 0u;
    position = 0u;
    custom_data = NULL;
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

} // namespace rrlib
} // namespace serialization

#endif // __rrlib__serialization__tBufferInfo_h__
