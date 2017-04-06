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
/*!\file    rrlib/serialization/tRegister.h
 *
 * \author  Max Reichardt
 *
 * \date    2017-02-05
 *
 * \brief   Contains tRegister
 *
 * \b tRegister
 *
 * Global register that is suitable for auto-publishing via binary streams.
 *
 */
//----------------------------------------------------------------------
#ifndef __rrlib__serialization__tRegister_h__
#define __rrlib__serialization__tRegister_h__

//----------------------------------------------------------------------
// External includes (system with <>, local with "")
//----------------------------------------------------------------------
#include "rrlib/concurrent_containers/tRegister.h"
#include <utility>

//----------------------------------------------------------------------
// Internal includes with ""
//----------------------------------------------------------------------
#include "rrlib/serialization/tInputStream.h"
#include "rrlib/serialization/tOutputStream.h"

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
//! Auto-publishable register
/*!
 * Global concurrent register that is suitable for auto-publishing via binary streams.
 *
 * \tparam THandle Handle data type that is used to encode handle (== index in this register) to stream.
 *                 It should not be larger than an uint32_t (more would be a waste of bandwidth). Note that the value -1 is used for escaping.
 *                 It can be chosen larger than currently required to be suitable also for later increase of register size limits.
 *                 It can be chosen smaller if register - for some reason - is specified slightly too large.
 */
template < typename TEntry, size_t Tchunk_count, size_t Tchunk_size,
         typename THandle = typename std::conditional<Tchunk_count * Tchunk_size >= (std::numeric_limits<uint16_t>::max()), uint32_t, typename std::conditional<Tchunk_count * Tchunk_size >= (std::numeric_limits<uint8_t>::max()), uint16_t, uint8_t >::type >::type,
         typename TMutex = rrlib::thread::tMutex >
class tRegister : public rrlib::concurrent_containers::tRegister<TEntry, Tchunk_count, Tchunk_size, TMutex>
{

//----------------------------------------------------------------------
// Public methods and typedefs
//----------------------------------------------------------------------
public:

  typedef rrlib::concurrent_containers::tRegister<TEntry, Tchunk_count, Tchunk_size, TMutex> tBase;
  typedef THandle tHandle;
  static_assert(std::is_integral<tHandle>::value && sizeof(tHandle) <= 4, "Invalid handle type");

  tRegister(std::atomic<size_t>* external_size_variable = nullptr) :
    tBase(external_size_variable),
    uid(-1)
  {}

  /*!
   * \return Uid of this (global) register. -1 means that no uid has been assigned yet.
   */
  int GetUid() const
  {
    return uid;
  }

  /*!
   * Reads local entry of this register from stream
   *
   * \param stream Stream to deserialize entry from
   * \param result Object to store result in
   * \return Returns true if 'UID' encoding has been specified in stream. In this case, the caller must deserialize the entry's uid.
   * \throw Throws exception if read handle is out of bounds
   */
  bool ReadEntry(tInputStream& stream, TEntry& result) const
  {
    tRegisterEntryEncoding encoding = stream.GetSourceInfo().GetRegisterEntryEncoding(uid);
    if (encoding == tRegisterEntryEncoding::UID)
    {
      return true;
    }
    tHandle handle;
    stream >> handle;
    if (handle >= this->Size())
    {
      throw std::runtime_error("tRegister::ReadEntry: handle is out of bounds");
    }
    result = (*this)[handle];
    return false;
  }

  /*!
   * Writes entry of this register to stream - using encoding specified in stream.
   *
   * \param stream Stream to serialize entry to
   * \param handle Handle of entry to write (must be its index in this register)
   * \return Returns true if 'UID' encoding has been specified in stream. In this case, the caller must serialize the entry's uid.
   */
  bool WriteEntry(tOutputStream& stream, tHandle handle) const
  {
    tRegisterEntryEncoding encoding = stream.GetTargetInfo().GetRegisterEntryEncoding(uid);
    if (encoding == tRegisterEntryEncoding::UID)
    {
      return true;
    }
    if (static_cast<uint>(encoding) >= static_cast<uint>(tRegisterEntryEncoding::PUBLISH_REGISTER_ON_DEMAND))
    {
      stream.WriteRegisterUpdates(uid, handle, sizeof(tHandle));
    }
    stream.WriteNumber(handle);
    return false;
  }

  /*!
   * Write last entry of this register stream.
   * If this register is published, this will update the complete remote register (main purpose of this convenience method).
   *
   * \param stream Stream to serialize to
   * \return Returns true if 'UID' encoding has been specified in stream. In this case, the caller must serialize the entry's uid.
   */
  bool WriteLastEntry(tOutputStream& stream) const
  {
    return WriteEntry(stream, static_cast<tHandle>(this->Size() - 1));
  }

//----------------------------------------------------------------------
// Private fields and methods
//----------------------------------------------------------------------
private:

  friend class PublishedRegisters;

  /*!
   * Uid of this (global) register. This must be consistent across all processes that read/write serialized data created with the automatic register publishing mechanisms.
   * The uid is assigned when first registered at PublishedRegisters.
   * -1 means that no uid has been assigned yet.
   */
  mutable int uid;

};

//----------------------------------------------------------------------
// End of namespace declaration
//----------------------------------------------------------------------
}
}


#endif
