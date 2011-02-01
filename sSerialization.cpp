/**
 * You received this file as part of RRLib serialization
 *
 * Copyright (C) 2010-2011 Max Reichardt,
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
#include <cxxabi.h>

#include "rrlib/serialization/sSerialization.h"
#include "rrlib/serialization/tStringOutputStream.h"
#include "rrlib/serialization/tStringInputStream.h"
#include <stdexcept>
#include <stdint.h>

namespace rrlib
{
namespace serialization
{
char sSerialization::cTO_HEX[16] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };
int sSerialization::cTO_INT[256];

std::string sSerialization::Demangle(const char* mangled)
{
  int status = 0;
  char* tmp = abi::__cxa_demangle(mangled, 0, 0, &status);
  std::string result(tmp);
  free(tmp);
  return result;
}

int sSerialization::cINIT_HELPER = StaticInit();

void sSerialization::ConvertBinaryToHexString(tInputStream& src, tStringOutputStream& os)
{
  while (src.MoreDataAvailable())
  {
    uint8_t b = src.ReadByte();
    uint b1 = b >> 4u;
    uint b2 = b & 0xFu;
    os.Append(cTO_HEX[b1]);
    os.Append(cTO_HEX[b2]);
  }
}

void sSerialization::ConvertHexStringToBinary(tStringInputStream& src, tOutputStream& co)
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

void sSerialization::DeserializeFromHexString(tSerializable* cs, tStringInputStream& s)
{
  tStackMemoryBuffer<65536> cb;
  tOutputStream co(&(cb));
  ConvertHexStringToBinary(s, co);
  co.Close();
  tInputStream ci(&(cb));
  cs->Deserialize(ci);
  ci.Close();
}

std::string sSerialization::Serialize(const tSerializable& rs)
{
  tStringOutputStream os;
  rs.Serialize(os);
  return os.ToString();
}

void sSerialization::SerializeToHexString(const tSerializable* cs, tStringOutputStream& os)
{
  tStackMemoryBuffer<65536> cb;
  tOutputStream co(&(cb));
  cs->Serialize(co);
  co.Close();
  tInputStream ci(&(cb));
  ConvertBinaryToHexString(ci, os);
  ci.Close();
}

int sSerialization::StaticInit()
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

} // namespace rrlib
} // namespace serialization

