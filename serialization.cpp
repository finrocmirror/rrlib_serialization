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
/*!\file    rrlib/serialization/serialization.cpp
 *
 * \author  Max Reichardt
 *
 * \date    2013-05-17
 *
 */
//----------------------------------------------------------------------
#include "rrlib/serialization/serialization.h"

//----------------------------------------------------------------------
// External includes (system with <>, local with "")
//----------------------------------------------------------------------

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

static char cTO_HEX[16] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };
static int cTO_INT[256];

static int StaticInit()
{
  for (size_t i = 0u; i < 256; i++)
  {
    cTO_INT[i] = -1;
  }
  cTO_INT['0'] = 0;
  cTO_INT['1'] = 1;
  cTO_INT['2'] = 2;
  cTO_INT['3'] = 3;
  cTO_INT['4'] = 4;
  cTO_INT['5'] = 5;
  cTO_INT['6'] = 6;
  cTO_INT['7'] = 7;
  cTO_INT['8'] = 8;
  cTO_INT['9'] = 9;
  cTO_INT['A'] = 0xA;
  cTO_INT['B'] = 0xB;
  cTO_INT['C'] = 0xC;
  cTO_INT['D'] = 0xD;
  cTO_INT['E'] = 0xE;
  cTO_INT['F'] = 0xF;
  cTO_INT['a'] = 0xA;
  cTO_INT['b'] = 0xB;
  cTO_INT['c'] = 0xC;
  cTO_INT['d'] = 0xD;
  cTO_INT['e'] = 0xE;
  cTO_INT['f'] = 0xF;
  return 0;
}

int cINIT_HELPER = StaticInit();

void ConvertBinaryToHexString(tInputStream& src, tStringOutputStream& os)
{
  while (src.MoreDataAvailable())
  {
    uint8_t b = src.ReadByte();
    uint bi = b;
    uint b1 = bi >> 4u;
    uint b2 = bi & 0xFu;
    assert((b1 >= 0 && b1 < 16));
    assert((b2 >= 0 && b2 < 16));
    os.Append(cTO_HEX[b1]);
    os.Append(cTO_HEX[b2]);
  }
}

void ConvertHexStringToBinary(tStringInputStream& src, tOutputStream& co)
{
  int c1;
  while ((c1 = src.Read()) != -1)
  {
    int c2 = src.Read();
    if (c2 == -1)
    {
      throw std::runtime_error("not a valid hex string (should have even number of chars)");
    }
    if (cTO_INT[c1] < 0 || cTO_INT[c2] < 0)
    {
      throw std::runtime_error("invalid hex chars");;
    }
    int b = (cTO_INT[c1] << 4) | cTO_INT[c2];
    co.WriteByte(static_cast<int8_t>(b));
  }
}

//----------------------------------------------------------------------
// End of namespace declaration
//----------------------------------------------------------------------
}
}
