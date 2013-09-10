//
// You received this file as part of RRLib
// Robotics Research Library
//
// Copyright (C) AG Robotersysteme TU Kaiserslautern
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
/*!\file    rrlib/serialization/tFileSink.cpp
 *
 * \author  Michael Arndt
 *
 * \date    2013-09-06
 *
 */
//----------------------------------------------------------------------
#include "rrlib/serialization/tFileSink.h"

//----------------------------------------------------------------------
// External includes (system with <>, local with "")
//----------------------------------------------------------------------
#include "rrlib/logging/messages.h"

//----------------------------------------------------------------------
// Internal includes with ""
//----------------------------------------------------------------------

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
// tFileSink constructors
//----------------------------------------------------------------------
tFileSink::tFileSink(const std::string &file_path) : tSink(), file_path(file_path),
  backend(1024)
{
  ofstream.exceptions(std::ofstream::failbit | std::ofstream::badbit);
}

void tFileSink::Close(tOutputStream& output_stream, tBufferInfo& buffer)
{
  RRLIB_LOG_PRINT(DEBUG_VERBOSE_1, "Closing file");

  if (ofstream.is_open())
  {
    ofstream.close();
  }
}

void tFileSink::Reset(tOutputStream& output_stream, tBufferInfo& buffer)
{
  RRLIB_LOG_PRINT(DEBUG_VERBOSE_1, "Resetting stream");

  this->Close(output_stream, buffer);

  ofstream.open(file_path.c_str());
  if (!ofstream.is_open())
  {
    RRLIB_LOG_PRINT(ERROR, "Could not open stream for file ", file_path);
  }

  buffer.buffer = &backend;
  buffer.position = 0u;
  buffer.SetRange(0u, backend.Capacity());

}

bool tFileSink::Write(tOutputStream& output_stream, tBufferInfo& buffer, int write_size_hint)
{
  RRLIB_LOG_PRINT(DEBUG_VERBOSE_1, "Write with length ", buffer.GetWriteLen());

  ofstream.write(buffer.buffer->GetPointer(), buffer.GetWriteLen());
  buffer.position = 0u;
  buffer.SetRange(0u, backend.Capacity());

  return true;
}

void tFileSink::Flush(tOutputStream& output_stream, const tBufferInfo& buffer)
{
  RRLIB_LOG_PRINT(DEBUG_VERBOSE_1, "Flush, remaining length ", buffer.GetWriteLen());
  ofstream.write(buffer.buffer->GetPointer(), buffer.GetWriteLen());
  ofstream.flush();
}

bool tFileSink::DirectWriteSupport()
{
  return true;
}

void tFileSink::DirectWrite(tOutputStream& output_stream, const tFixedBuffer& buffer, size_t offset, size_t len)
{
  RRLIB_LOG_PRINT(DEBUG_VERBOSE_1, "Direct write of offset ", offset, " and length ", len);
  ofstream.write(buffer.GetPointer() + offset, len);
}


//----------------------------------------------------------------------
// End of namespace declaration
//----------------------------------------------------------------------
}
}
