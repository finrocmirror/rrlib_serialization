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
/*!\file    rrlib/serialization/tFileSink.h
 *
 * \author  Michael Arndt
 *
 * \date    2013-09-06
 *
 * \brief   Contains tFileSink
 *
 * \b tFileSink
 *
 * A data sink that writes binary data to a file.
 *
 */
//----------------------------------------------------------------------
#ifndef __rrlib__serialization__tFileSink_h__
#define __rrlib__serialization__tFileSink_h__

//----------------------------------------------------------------------
// External includes (system with <>, local with "")
//----------------------------------------------------------------------
#include <iostream>
#include <fstream>

//----------------------------------------------------------------------
// Internal includes with ""
//----------------------------------------------------------------------
#include "rrlib/serialization/tSink.h"
#include "rrlib/serialization/tBufferInfo.h"

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
//! A data sink that writes binary data to a file.
/*!
 * A data sink that writes binary data to a file.
 *
 * Example usage:
 *
 *  using namespace rrlib::serialization;
 *  tFileSink sink("/path/to/some_file");
 *  tOutputStream os(sink);
 *  std::string str("Some String");
 *  os << str;
 *  os.Close();
 *
 */
class tFileSink : public tSink
{

//----------------------------------------------------------------------
// Public methods and typedefs
//----------------------------------------------------------------------
public:

  /**
   * Create a new file sink for the specified file
   * \param file_path path to the file
   */
  tFileSink(const std::string &file_path);

private:

  /*!
   * Close stream/sink
   *
   * \param output_stream Buffer that requests operation
   * \param buffer Buffer that is managed - and was possibly allocated by - sink
   */
  virtual void Close(tOutputStream& output_stream, tBufferInfo& buffer) override;

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
  virtual void DirectWrite(tOutputStream& output_stream, const tFixedBuffer& buffer, size_t offset, size_t len) override;

  /*!
   * \return Does Sink support direct writing?
   * (optional optimization for reduction of copying overhead)
   */
  virtual bool DirectWriteSupport() override;

  /*!
   * Flush/Commit data written to sink
   *
   * \param output_stream Stream that requests operation
   * \param buffer Buffer that contains data to write - managed by client
   */
  virtual void Flush(tOutputStream& output_stream, const tBufferInfo& buffer) override;

  /*!
   * Reset sink for writing content (again)
   * (may only be supported once - typically the case with streams)
   *
   * \param output_stream Stream that requests operation
   * \param buffer Buffer that is managed - and was possibly allocated by - sink
   */
  virtual void Reset(tOutputStream& output_stream, tBufferInfo& buffer) override;

  /*!
   * Write/flush data to sink/"device".
   * Bytes from buffer "limit" to buffer position are written.
   *
   * \param output_stream Stream that requests operation
   * \param buffer Buffer that is managed and contains data. Needs to be cleared/reset/replaced by this method.
   * \param write_size_hint Hint about how much data we plan to write additionally (mostly makes sense, when there's no direct read support); -1 indicates manual flush without need for size increase
   * \return Invalidate any Placeholder? (usually true, when buffer changes)
   */
  virtual bool Write(tOutputStream& output_stream, tBufferInfo& buffer, int write_size_hint) override;


//----------------------------------------------------------------------
// Private fields and methods
//----------------------------------------------------------------------
private:

  /*! The file that should be opened */
  std::string file_path;
  /*! Output stream to write to */
  std::ofstream ofstream;

  /*! Wrapped memory buffer */
  tFixedBuffer backend;
};

//----------------------------------------------------------------------
// End of namespace declaration
//----------------------------------------------------------------------
}
}


#endif
