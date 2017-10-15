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
  XML,
  UNUSED_LEGACY_VALUE,
  NONE
};


/*!
 * Specifies encoding of register entries in stream
 * (including whether registers are published and when they are updated)
 */
enum class tRegisterEntryEncoding : uint8_t
{
  /*! Type handles of current process are used. Efficient, but not suitable for transferring entries to another process nor making data persistent. */
  LOCAL_HANDLE,

  /*!
   * UIDs (e.g. unique names) of register entries are written to stream.
   * Least efficient option for longer lasting streams: requires more bandwidth per entry and also overhead for lookup at deserialization.
   */
  UID,

  /*!
   * Register entries are encoded and sent to partner once. After that, entries are encoded with their local handle.
   * Remote register is updated, whenever entries are sent which have not yet been transferred (all entries up to the one are sent -> so sending the last one, will update the whole register).
   * Increased bandwidth requirements initially for publishing register entries.
   */
  PUBLISH_REGISTER_ON_DEMAND,

  /*!
   * Register entries are encoded and sent to partner once. After that, entries are encoded with their local handle.
   * Remote register is updated, whenever there are new entries locally and any register entry (possibly from another register) is written to stream.
   * Increased bandwidth requirements initially for publishing all register entries.
   */
  PUBLISH_REGISTER_ON_CHANGE
};

/*! Maximum number of published registers */
enum { cMAX_PUBLISHED_REGISTERS = 15 };

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
