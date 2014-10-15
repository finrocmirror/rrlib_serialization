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
/*!\file    rrlib/serialization/tFileSource.h
 *
 * \author  Michael Arndt
 *
 * \date    2013-09-06
 *
 * \brief   Contains tFileSource
 *
 * \b tFileSource
 *
 * A data source that reads binary data from a file.
 *
 */
//----------------------------------------------------------------------
#ifndef __rrlib__serialization__tFileSource_h__
#define __rrlib__serialization__tFileSource_h__

//----------------------------------------------------------------------
// External includes (system with <>, local with "")
//----------------------------------------------------------------------
#include <iostream>
#include <fstream>

//----------------------------------------------------------------------
// Internal includes with ""
//----------------------------------------------------------------------
#include "rrlib/serialization/tSource.h"
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
//! A data source that reads binary data from a file.
/*!
 * A data source that reads binary data from a file.
 *
 * Example usage:
 *
 *  using namespace rrlib::serialization;
 *  tFileSource src("/path/to/some_file");
 *  tInputStream is(src);
 *  std::string str;
 *  is >> str;
 *  is.Close();
 *  std::cout << "Read string: " << str << std::endl;
 */
class tFileSource : public tSource
{

//----------------------------------------------------------------------
// Public methods and typedefs
//----------------------------------------------------------------------
public:

  /**
   * Create a new file source for the specified file
   * \param file_path path to the file
   * \param buffer_size the size of the internal buffer
   */
  tFileSource(const std::string &file_path, size_t buffer_size = 8192);

private:

  /*!
   * Close stream/source.
   *
   * Possibly clean up buffer(s).
   *
   * \param input_stream tInputStream that requests fetch operation.
   * \param buffer BufferInfo object that may contain buffer that needs to be deleted
   */
  virtual void Close(tInputStream& input_stream, tBufferInfo& buffer) override;

  /*!
   * (Optional operation)
   * Fetch next bytes for reading - and copy them directly to target buffer.
   *
   * \param input_stream tInputStream that requests fetch operation.
   * \param buffer Buffer to copy data to (buffer provided and managed by client)
   * \param offset Offset in provided buffer
   * \param len Minimum number of bytes to read
   */
  virtual void DirectRead(tInputStream& input_stream, tFixedBuffer& buffer, size_t offset, size_t len = 0) override;

  /*!
   * \return Does source support reading directly into target buffer?
   * (optional optimization - does not have to make sense, depending on source)
   */
  virtual bool DirectReadSupport() const override;

  /*!
   * Is any more data available?
   *
   * \param input_stream Buffer that requests operation
   * \param buffer Current buffer (managed by source)
   * \return Answer
   */
  virtual bool MoreDataAvailable(tInputStream& input_stream, tBufferInfo& buffer) override;

  /*!
   * Fetch next bytes for reading.
   *
   * Source is responsible for managing buffers that is writes/creates in buffer object
   *
   * if len is <= zero, method will not block
   * if len is greater, method may block until number of bytes is available
   *
   * \param input_stream tInputStream that requests fetch operation.
   * \param buffer BufferInfo object that contains result of read operation (buffer managed by Source)
   * \param len Minimum number of bytes to read
   */
  virtual void Read(tInputStream& input_stream, tBufferInfo& buffer, size_t len = 0) override;

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
  virtual void Reset(tInputStream& input_stream, tBufferInfo& buffer) override;

  virtual void Seek(tInputStream& input_stream, tBufferInfo& buffer, uint64_t position) override;
  virtual bool SeekSupport() override;

//----------------------------------------------------------------------
// Private fields and methods
//----------------------------------------------------------------------
private:

  /*! The file that should be opened */
  std::string file_path;
  /*! Input stream to read from */
  std::ifstream ifstream;

  /*! Wrapped memory buffer */
  tFixedBuffer backend;

};

//----------------------------------------------------------------------
// End of namespace declaration
//----------------------------------------------------------------------
}
}


#endif
