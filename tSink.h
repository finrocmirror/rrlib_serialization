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
/*!\file    rrlib/serialization/tSink.h
 *
 * \author  Max Reichardt
 *
 * \date    2013-05-17
 *
 * \brief   Contains tSink
 *
 * \b tSink
 *
 * Abstract data sink interface that can be used with binary output streams
 * (tOutputStream class).
 *
 * The data sink is responsible for buffer management.
 *
 */
//----------------------------------------------------------------------
#ifndef __rrlib__serialization__tSink_h__
#define __rrlib__serialization__tSink_h__

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
class tOutputStream;
struct tBufferInfo;

//----------------------------------------------------------------------
// Class declaration
//----------------------------------------------------------------------
//! Abstract interface for data sink
/*!
 * Abstract data sink interface that can be used with binary output streams
 * (tOutputStream class).
 *
 * The data sink is responsible for buffer management.
 */
class tSink
{

//----------------------------------------------------------------------
// Public methods and typedefs
//----------------------------------------------------------------------
public:

  virtual ~tSink() {}

  /*!
   * Close stream/sink
   *
   * \param output_stream Buffer that requests operation
   * \param buffer Buffer that is managed - and was possibly allocated by - sink
   */
  virtual void Close(tOutputStream& output_stream, tBufferInfo& buffer) = 0;

  /*!
   * Directly write buffer to sink
   * (optional optimization for reduction of copying overhead)
   * (will only be called after flush() operation)
   *
   * \param output_stream Stream that requests operation
   * \param buffer Buffer that contains data to write - managed by client
   * \param offset Offset to start reading in buffer
   * \param len Number of bytes to write
   */
  virtual void DirectWrite(tOutputStream& output_stream, const tFixedBuffer& buffer, size_t offset, size_t len) = 0;

  /*!
   * \return Does Sink support direct writing?
   * (optional optimization for reduction of copying overhead)
   */
  virtual bool DirectWriteSupport() = 0;

  /*!
   * Flush/Commit data written to sink
   *
   * \param output_stream Stream that requests operation
   * \param buffer Buffer that contains data to write - managed by client
   */
  virtual void Flush(tOutputStream& output_stream, const tBufferInfo& buffer) = 0;

  /*!
   * Reset sink for writing content (again)
   * (may only be supported once - typically the case with streams)
   *
   * \param output_stream Stream that requests operation
   * \param buffer Buffer that is managed - and was possibly allocated by - sink
   */
  virtual void Reset(tOutputStream& output_stream, tBufferInfo& buffer) = 0;

  /*!
   * Write/flush data to sink/"device".
   * Bytes from buffer "limit" to buffer position are written.
   *
   * \param output_stream Stream that requests operation
   * \param buffer Buffer that is managed and contains data. Needs to be cleared/reset/replaced by this method.
   * \param write_size_hint Hint about how much data we plan to write additionally (mostly makes sense, when there's no direct read support); -1 indicates manual flush without need for size increase
   * \return Invalidate any Placeholder? (usually true, when buffer changes)
   */
  virtual bool Write(tOutputStream& output_stream, tBufferInfo& buffer, int write_size_hint) = 0;

};

//----------------------------------------------------------------------
// End of namespace declaration
//----------------------------------------------------------------------
}
}


#endif
