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
/*!\file    rrlib/serialization/tFileSource.cpp
 *
 * \author  Michael Arndt
 *
 * \date    2013-09-06
 *
 */
//----------------------------------------------------------------------
#include "rrlib/serialization/tFileSource.h"

//----------------------------------------------------------------------
// External includes (system with <>, local with "")
//----------------------------------------------------------------------
#include "rrlib/logging/messages.h"

//----------------------------------------------------------------------
// Internal includes with ""
//----------------------------------------------------------------------
#include "rrlib/serialization/tBufferInfo.h"

//----------------------------------------------------------------------
// Debugging
//----------------------------------------------------------------------
#include <cassert>

//----------------------------------------------------------------------
// Namespace usage
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

//----------------------------------------------------------------------
// Const values
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// Implementation
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// tFileSource constructors
//----------------------------------------------------------------------
tFileSource::tFileSource(const std::string &file_path, size_t buffer_size) :
  file_path(file_path),
  backend(buffer_size)
{
  ifstream.exceptions(std::ifstream::failbit | std::ifstream::badbit);

  std::ifstream file_exists(this->file_path);
  if (!file_exists)
  {
    throw std::runtime_error("File '" + this->file_path + "' does not exist");
  }
}

void tFileSource::Close(tInputStream& input_stream, tBufferInfo& buffer)
{
  RRLIB_LOG_PRINT(DEBUG_VERBOSE_1, "Closing file");
  if (ifstream.is_open())
  {
    ifstream.close();
  }
}

/*!
 * (Optional operation)
 * Fetch next bytes for reading - and copy them directly to target buffer.
 *
 * \param input_stream tInputStream that requests fetch operation.
 * \param buffer Buffer to copy data to (buffer provided and managed by client)
 * \param offset Offset in provided buffer
 * \param len Number of bytes to read
 */
void tFileSource::DirectRead(tInputStream& input_stream, tFixedBuffer& buffer, size_t offset, size_t len)
{
  // this may block
  ifstream.read(buffer.GetPointer() + offset, len);
  std::streamsize read = ifstream.gcount();

  RRLIB_LOG_PRINT(DEBUG_VERBOSE_1, "Read ", read, " bytes (", len, " bytes requested)");
}

bool tFileSource::DirectReadSupport() const
{
  return true;
}

/*!
 * Is any more data available?
 *
 * \param input_stream Buffer that requests operation
 * \param buffer Current buffer (managed by source)
 * \return Answer
 */
bool tFileSource::MoreDataAvailable(tInputStream& input_stream, tBufferInfo& buffer)
{
  // in this case we can be sure that there is nothing more to read ...
  if (!ifstream.good() || ifstream.eof())
    return false;

  ifstream.peek(); // we somehow need to peek before checking for EOF
  bool avail = ifstream.good() && !ifstream.eof();
  RRLIB_LOG_PRINT(DEBUG_VERBOSE_1, "More data available: ", avail);
  return avail;
}

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
void tFileSource::Read(tInputStream& input_stream, tBufferInfo& buffer, size_t len)
{
  std::streamsize read = ifstream.readsome(backend.GetPointer(), buffer.buffer->Capacity());
  while (len > 0 && read < static_cast<std::streamsize>(len))
  {
    // this may block
    ifstream.read(backend.GetPointer() + read, len - read);
    read += ifstream.gcount();
  }
  buffer.position = 0;

  buffer.SetRange(0, read);
  RRLIB_LOG_PRINT(DEBUG_VERBOSE_1, "Read ", read, " bytes (", len, " bytes requested), buffer capacity (max possible): ", buffer.buffer->Capacity());
}

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
void tFileSource::Reset(tInputStream& input_stream, tBufferInfo& buffer)
{
  RRLIB_LOG_PRINT(DEBUG_VERBOSE_1, "Resetting stream");

  this->Close(input_stream, buffer);

  ifstream.open(file_path.c_str());
  if (!ifstream.is_open())
  {
    RRLIB_LOG_PRINT(ERROR, "Could not open stream for file ", file_path);
  }

  buffer.buffer = &backend;
  buffer.position = 0u;
  buffer.SetRange(0u, 0u);

}

void tFileSource::Seek(tInputStream& input_stream, tBufferInfo& buffer, uint64_t position)
{
  ifstream.seekg(position);

  buffer.buffer = &backend;
  buffer.position = 0u;
  buffer.SetRange(0u, 0u);

}

bool tFileSource::SeekSupport()
{
  return true;
}

//----------------------------------------------------------------------
// End of namespace declaration
//----------------------------------------------------------------------
}
}
