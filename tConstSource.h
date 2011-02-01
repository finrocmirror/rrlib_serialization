/**
 * You received this file as part of RRLib serialization
 *
 * Copyright (C) 2009-2010 Max Reichardt,
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

#ifndef __rrlib__serialization__tConstSource_h__
#define __rrlib__serialization__tConstSource_h__

namespace rrlib
{
namespace serialization
{
class tInputStream;
class tBufferInfo;
class tFixedBuffer;

/*!
 * \author Max Reichardt
 *
 * Abstract Data Source that can be used with InputStreamBuffer.
 *
 * Somewhat similar to boost iostreams input devices.
 * Is responsible for buffer management.
 *
 * Same as source, but with const-methods.
 * Typically, only const-sources allow concurrent reading
 */
class tConstSource
{
public:

  /*!
   * Close stream/source.
   *
   * Possibly clean up buffer(s).
   *
   * \param input_stream_buffer InputStreamBuffer that requests fetch operation.
   * \param buffer BufferInfo object that may contain buffer that needs to be deleted
   */
  virtual void Close(tInputStream* input_stream_buffer, tBufferInfo& buffer) const = 0;

  /*!
   * (Optional operation)
   * Fetch next bytes for reading - and copy them directly to target buffer.
   *
   * \param input_stream_buffer InputStreamBuffer that requests fetch operation.
   * \param buffer Buffer to copy data to (buffer provided and managed by client)
   * \param len Minimum number of bytes to read
   */
  virtual void DirectRead(tInputStream* input_stream_buffer, tFixedBuffer& buffer, size_t offset, size_t len = 0) const = 0;

  /*!
   * \return Does source support reading directly into target buffer?
   * (optional optimization - does not have to make sense, depending on source)
   */
  virtual bool DirectReadSupport() const = 0;

  /*!
   * Is any more data available?
   *
   * \param input_stream_buffer Buffer that requests operation
   * \param buffer Current buffer (managed by source)
   * \return Answer
   */
  virtual bool MoreDataAvailable(tInputStream* input_stream_buffer, tBufferInfo& buffer) const = 0;

  /*!
   * Fetch next bytes for reading.
   *
   * Source is responsible for managing buffers that is writes/creates in buffer object
   *
   * if len is <= zero, method will not block
   * if len is greater, method may block until number of bytes in available
   *
   * \param input_stream_buffer InputStreamBuffer that requests fetch operation.
   * \param buffer BufferInfo object that contains result of read operation (buffer managed by Source)
   * \param len Minimum number of bytes to read
   */
  virtual void Read(tInputStream* input_stream_buffer, tBufferInfo& buffer, size_t len = 0) const = 0;

  /*!
   * Reset input stream buffer for reading.
   * This is called initially when associating source with InputStreamBuffer.
   *
   * Supporting multiple reset operations is optional.
   * Streaming buffers typically won't support this (typically an assert will fail)
   *
   * \param input_stream_buffer InputStreamBuffer that requests reset operation.
   * \param buffer BufferInfo object that will contain result - about buffer to initially operate on
   */
  virtual void Reset(tInputStream* input_stream_buffer, tBufferInfo& buffer) const = 0;

};

} // namespace rrlib
} // namespace serialization

#endif // __rrlib__serialization__tConstSource_h__
