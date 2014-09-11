//
// You received this file as part of RRLib
// Robotics Research Library
//
// Copyright (C) AG Robotersysteme TU Kaiserslautern
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
/*!\file    rrlib/serialization/tests/serialization.h
 *
 * \author  Max Reichardt
 *
 * \date    2014-09-02
 *
 * Header for general serialization test (required to get enum strings built)
 *
 */
//----------------------------------------------------------------------
#ifndef __rrlib__serialization__tests__serialization_h__
#define __rrlib__serialization__tests__serialization_h__

//----------------------------------------------------------------------
// External includes (system with <>, local with "")
//----------------------------------------------------------------------
#include <limits>

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
enum class tEnumSigned : int64_t
{
  MIN_VALUE = std::numeric_limits<int64_t>::min(),
  MAX_VALUE = std::numeric_limits<int64_t>::max()
};

enum class tEnumUnsigned : uint64_t
{
  MIN_VALUE = std::numeric_limits<uint64_t>::min(),
  MAX_VALUE = std::numeric_limits<uint64_t>::max()
};

//----------------------------------------------------------------------
// Function declarations
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// End of namespace declaration
//----------------------------------------------------------------------
}
}


#endif
