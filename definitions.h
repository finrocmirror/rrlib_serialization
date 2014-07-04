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
/*!\file    rrlib/serialization/definitions.h
 *
 * \author  Max Reichardt
 *
 * \date    2013-05-17
 *
 * Definitions for rrlib_serialization
 *
 */
//----------------------------------------------------------------------
#ifndef __rrlib__serialization__definitions_h__
#define __rrlib__serialization__definitions_h__

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

/*!
 * Enum for different types of data encoding
 */
enum class tDataEncoding
{
  BINARY,
  STRING,
  XML
};

/*!
 * Type encoding for streams
 */
enum class tTypeEncoding
{
  LOCAL_UIDS,  //!< Type uids (int16_t) of current process are used. Efficient, but not suitable transferring data types to another process nor making data persistent
  NAMES,       //!< Type names are used. Less efficient, but suitable for transferring data types to another process and making data persistent.
  CUSTOM       //!< A custom type encoder is used.
};

#ifndef __BYTE_ORDER__
#warning __BYTE_ORDER__ not defined
#endif

//----------------------------------------------------------------------
// Function declarations
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// End of namespace declaration
//----------------------------------------------------------------------
}
}


#endif
