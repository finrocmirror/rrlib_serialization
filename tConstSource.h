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
/*!\file    rrlib/serialization/tConstSource.h
 *
 * \author  Max Reichardt
 *
 * \date    2013-05-17
 *
 * \brief   Contains tConstSource
 *
 * \b tConstSource
 *
 * Abstract data source interface that can be used with binary input streams
 * (tInputStream class).
 *
 * The data source is responsible for buffer management.
 *
 * Same as tSource, but with only const-classified methods.
 * Typically, only constant data sources (possibly a fixed memory buffer)
 * allow being read from multiple binary input streams concurrently.
 *
 */
//----------------------------------------------------------------------
#ifndef __rrlib__serialization__tConstSource_h__
#define __rrlib__serialization__tConstSource_h__

//----------------------------------------------------------------------
// External includes (system with <>, local with "")
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// Internal includes with ""
//----------------------------------------------------------------------

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
class tInputStream;
struct tBufferInfo;
class tFixedBuffer;

//----------------------------------------------------------------------
// Class declaration
//----------------------------------------------------------------------
//! Abstract interface for constant data sources
/*!
 * Abstract data source interface that can be used with binary input streams
 * (tInputStream class).
 *
 * The data source is responsible for buffer management.
 *
 * Same as tSource, but with only const-classified methods.
 * Typically, only constant data sources (possibly a fixed memory buffer)
 * allow being read from multiple binary input streams concurrently.
 */
class tConstSource
{

//----------------------------------------------------------------------
// Public methods and typedefs
//----------------------------------------------------------------------
public:

  virtual ~tConstSource() {}

  /*!
   * Close stream/source.
   *
   * Possibly clean up buffer(s).
   *
   * \param input_stream tInputStream that requests fetch operation.
   * \param buffer BufferInfo object that may contain buffer that needs to be deleted
   */
  virtual void Close(tInputStream& input_stream, tBufferInfo& buffer) const = 0;

  /*!
   * (Optional operation)
   * Fetch next bytes for reading - and copy them directly to target buffer.
   *
   * \param input_stream tInputStream that requests fetch operation.
   * \param buffer Buffer to copy data to (buffer provided and managed by client)
   * \param offset Offset in 'buffer'
   * \param len Minimum number of bytes to read
   */
  virtual void DirectRead(tInputStream& input_stream, tFixedBuffer& buffer, size_t offset, size_t len = 0) const = 0;

  /*!
   * \return Does source support reading directly into target buffer?
   * (optional optimization - does not have to make sense, depending on source)
   */
  virtual bool DirectReadSupport() const = 0;

  /*!
   * Is any more data available?
   *
   * \param input_stream Buffer that requests operation
   * \param buffer Current buffer (managed by source)
   * \return Answer
   */
  virtual bool MoreDataAvailable(tInputStream& input_stream, tBufferInfo& buffer) const = 0;

  /*!
   * Fetch next bytes for reading.
   *
   * Source is responsible for managing buffers that is writes/creates in buffer object
   *
   * if len is <= zero, method will not block
   * if len is greater, method may block until number of bytes in available
   *
   * \param input_stream tInputStream that requests fetch operation.
   * \param buffer BufferInfo object that contains result of read operation (buffer managed by Source)
   * \param len Minimum number of bytes to read
   */
  virtual void Read(tInputStream& input_stream, tBufferInfo& buffer, size_t len = 0) const = 0;

  /*!
   * Reset input stream buffer for reading.
   * This is called initially when associating source with tInputStream.
   *
   * Supporting multiple reset operations is optional.
   * Streaming buffers typically won't support this (typically an assert will fail)
   *
   * \param input_stream tInputStream that requests reset operation.
   * \param buffer BufferInfo object that will contain result - about buffer to initially operate on
   */
  virtual void Reset(tInputStream& input_stream, tBufferInfo& buffer) const = 0;

  /*!
   * Seek to specified absolute position in the file
   *
   * \param input_stream tInputStream that requests fetch operation.
   * \param buffer BufferInfo object that contains result of read operation (buffer managed by Source)
   * \param position the absolute position
   */
  virtual void Seek(tInputStream& input_stream, tBufferInfo& buffer, uint64_t position) const = 0;

  /*!
   * \return Does source support seeking?
   */
  virtual bool SeekSupport() const = 0;
};

//----------------------------------------------------------------------
// End of namespace declaration
//----------------------------------------------------------------------
}
}


#endif
