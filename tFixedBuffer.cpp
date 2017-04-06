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
/*!\file    rrlib/serialization/tFixedBuffer.cpp
 *
 * \author  Max Reichardt
 *
 * \date    2013-05-17
 *
 */
//----------------------------------------------------------------------
#include "rrlib/serialization/tFixedBuffer.h"

//----------------------------------------------------------------------
// External includes (system with <>, local with "")
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// Internal includes with ""
//----------------------------------------------------------------------
#include "rrlib/serialization/tStringOutputStream.h"

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

tFixedBuffer::tFixedBuffer(size_t capacity) :
  buffer_memory(capacity > 0 ? new char[capacity] : nullptr),
  capacity(capacity),
  owns_buffer(capacity > 0)
{
}

// move constructor
tFixedBuffer::tFixedBuffer(tFixedBuffer && o) :
  buffer_memory(NULL),
  capacity(0),
  owns_buffer(false)
{
  std::swap(buffer_memory, o.buffer_memory);
  std::swap(capacity, o.capacity);
  std::swap(owns_buffer, o.owns_buffer);
}

// move assignment
tFixedBuffer& tFixedBuffer::operator=(tFixedBuffer && o)
{
  std::swap(buffer_memory, o.buffer_memory);
  std::swap(capacity, o.capacity);
  std::swap(owns_buffer, o.owns_buffer);
  return *this;
}

tFixedBuffer::~tFixedBuffer()
{
  if (owns_buffer && buffer_memory)
  {
    delete[] buffer_memory;
  }
}

std::string tFixedBuffer::GetLine(size_t offset) const
{
  tStringOutputStream sb;
  for (int i = offset, n = Capacity(); i < n; i++)
  {
    char c = static_cast<char>(GetByte(i));
    if (c == 0 || c == '\n')
    {
      return sb.ToString();
    }
    sb.Append(c);
  }

  throw std::runtime_error("String not terminated");
}

std::string tFixedBuffer::GetString(size_t offset, size_t length) const
{
  tStringOutputStream sb(length);
  for (int i = offset, n = std::min(Capacity(), offset + length); i < n; i++)
  {
    char c = static_cast<char>(GetByte(i));
    if (c == 0)
    {
      break;
    }
    sb.Append(c);
  }
  return sb.ToString();
}

//----------------------------------------------------------------------
// End of namespace declaration
//----------------------------------------------------------------------
}
}
