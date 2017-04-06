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
/*!\file    rrlib/serialization/tSerializationInfo.h
 *
 * \author  Max Reichardt
 *
 * \date    2017-02-04
 *
 * \brief   Contains tSerializationInfo
 *
 * \b tSerializationInfo
 *
 * Info on target or source of serialization
 *
 */
//----------------------------------------------------------------------
#ifndef __rrlib__serialization__tSerializationInfo_h__
#define __rrlib__serialization__tSerializationInfo_h__

//----------------------------------------------------------------------
// External includes (system with <>, local with "")
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// Internal includes with ""
//----------------------------------------------------------------------
#include "rrlib/serialization/definitions.h"

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
//! Serialization Info
/*!
 * Info on target or source of serialization.
 * This is used for a simple mechanism to support different versions of serialization.
 */
struct tSerializationInfo
{
  /*!
   * Revision of serialization.
   * The release version is encoded in this number (yymmdd). If revision is zero, this indicates the 14.08 release or earlier.
   *
   * For serialization targets: The release version that is supported by the target.
   * For serialization sources: At least this revision is required for deserialization. This is typically the minimum of the release version that serialization was created with and for.
   */
  uint revision;

  /*! Contains encodings for all register uids (2 bit per register uid). First is default encoding. */
  uint register_entry_encodings;
  static_assert((cMAX_PUBLISHED_REGISTERS) / 4 < sizeof(register_entry_encodings), "Needs adjusting");

  /*! Custom information that user of binary streams can attach to streams */
  size_t custom_info;

  /*!
   * \param revision Revision of serialization (see above)
   * \param default_register_entry_encoding Register encoding set as default for all registers
   * \param custom_info Custom information that user of binary streams can attach to streams
   */
  tSerializationInfo(uint revision = 0, tRegisterEntryEncoding default_register_entry_encoding = tRegisterEntryEncoding::UID, size_t custom_info = 0) :
    revision(revision),
    register_entry_encodings(static_cast<uint>(default_register_entry_encoding) * 0x55555555),
    custom_info(custom_info)
  {}

  /*!
   * \param register_uid Uid of register to get entry encoding for (-1 returns default register encoding)
   * \return Register entry encoding to use
   */
  tRegisterEntryEncoding GetRegisterEntryEncoding(int register_uid) const
  {
    return static_cast<tRegisterEntryEncoding>((register_entry_encodings >>(2 * (register_uid + 1))) & 0x3);
  }

  /*!
   * \return Whether any registers are published
   */
  bool HasPublishedRegisters() const
  {
    return register_entry_encodings & 0xAAAAAAAA;
  }

  /*!
   * Sets register entry encoding for register with specified uid.
   *
   * \param register_uid Uid of register to set entry encoding for
   * \param encoding Encoding to set
   */
  inline void SetRegisterEntryEncoding(uint register_uid, tRegisterEntryEncoding encoding)
  {
    uint shift = (register_uid + 1) * 2;
    uint mask = (0x3) << shift;
    register_entry_encodings &= (~mask);
    register_entry_encodings |= (static_cast<uint>(encoding) << shift);
  }
};

//----------------------------------------------------------------------
// End of namespace declaration
//----------------------------------------------------------------------
}
}


#endif
