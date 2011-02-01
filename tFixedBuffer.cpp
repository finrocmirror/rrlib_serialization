/**
 * You received this file as part of RRLib serialization
 *
 * Copyright (C) 2008-2011 Max Reichardt,
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
#include "rrlib/serialization/tFixedBuffer.h"
#include "rrlib/serialization/tStringOutputStream.h"
#include <stdexcept>

namespace rrlib
{
namespace serialization
{
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

void tFixedBuffer::SetCurrentBuffer(tFixedBuffer* fb)
{
  assert(!owns_buf);
  buffer = fb->buffer;
  capacity_x = fb->capacity_x;
  owns_buf = false;

}

} // namespace rrlib
} // namespace serialization

