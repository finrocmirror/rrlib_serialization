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
/*!\file    rrlib/serialization/PublishedRegisters.h
 *
 * \author  Max Reichardt
 *
 * \date    2017-02-05
 *
 * \brief   Contains PublishedRegisters
 *
 * \b PublishedRegisters
 *
 * Register of tRegisters that are available for auto-publishing mechanism
 *
 */
//----------------------------------------------------------------------
#ifndef __rrlib__serialization__PublishedRegisters_h__
#define __rrlib__serialization__PublishedRegisters_h__

//----------------------------------------------------------------------
// External includes (system with <>, local with "")
//----------------------------------------------------------------------
#include <memory>
#include <atomic>
#include "rrlib/thread/tLock.h"

//----------------------------------------------------------------------
// Internal includes with ""
//----------------------------------------------------------------------
#include "rrlib/serialization/tSerializationInfo.h"
#include "rrlib/serialization/tRegister.h"

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
//! Shared registers
/*!
 * Register of tRegisters that are available for auto-publishing mechanism
 */
class PublishedRegisters
{

  /*!
   * For lookup TRemoteType -> Register UID
   */
  template <typename TRemoteEntry>
  struct RemoteEntryRegisterUid
  {
    static int& Uid()
    {
      static int uid = -1;
      return uid;
    }
  };

  /*! Base class for remote registers */
  class tRemoteRegisterBase : public tInputStream::tRemoteRegister
  {
  public:
    virtual ~tRemoteRegisterBase();
    virtual void DeserializeEntries(tInputStream& stream) = 0;
    virtual const void* GetRemoteElement(size_t index) = 0;
    const std::atomic<size_t>* size;
    template <typename TEntry>
    void SetHandle(TEntry& entry, typename TEntry::tHandle handle)
    {
      entry.handle = handle;
    }
  };

//----------------------------------------------------------------------
// Public methods and typedefs
//----------------------------------------------------------------------
public:

  /*! Base class for type used as TRemoteEntry (class with identical members work also) */
  template <typename THandle, typename TLocalRegister>
  class tRemoteEntryBase
  {
  public:

    typedef THandle tHandle;
    typedef TLocalRegister tLocalRegister;

    /*!
     * \return Remote handle of entry
     */
    tHandle GetHandle() const
    {
      return handle;
    }

  private:

    friend class tRemoteRegisterBase;

    uint handle; //!< Remote handle of entry
  };

  /*! Base class for remote registers */
  template <typename TRemoteEntry>
  class tRemoteRegister : public tRemoteRegisterBase, public concurrent_containers::tRegister<TRemoteEntry, TRemoteEntry::tLocalRegister::cCHUNK_COUNT, TRemoteEntry::tLocalRegister::cCHUNK_SIZE, rrlib::thread::tNoMutex>
  {
  };

  /*!
   * Adds listener to register.
   *
   * \param uid Uid of register
   * \param callback Callback function that will be called whenever a new element is added.
   * \param address Address of listener. Optional: only required for identification when removing listener.
   */
  static void AddListener(uint uid, const std::function<void()>& callback, const void* address = nullptr);

  /*!
   * \param stream Stream whose (shared) data on remote register to obtain
   * \return Returns stream's (shared) remote register for type TRemoteEntry
   */
  template <typename TRemoteEntry>
  static const tRemoteRegister<TRemoteEntry>* GetRemoteRegister(tInputStream& stream)
  {
    return static_cast<const tRemoteRegister<TRemoteEntry>*>((*stream.shared_serialization_info.remote_registers)[GetRemoteEntryRegisterUid<TRemoteEntry>()].get());
  }

  /*!
   * Registers tRegister for use in auto-publishing mechanism.
   * UIDs must be consistent across all processes that read/write the same serialized data created with the automatic register publishing mechanisms.
   * Therefore, typically a high-level entity that knows all relevant registers should manage assignment of the UIDs.
   *
   * \param r tRegister to register
   * \param uid Uid to assign to register (must be <= cMAX_PUBLISHED_REGISTERS)
   * \tparam TRemoteEntry Type to deserialize in remote runtime environment. It needs to be derived from PublishedRegisters::tRemoteEntryBase, default-constructible and have
   *                      (1) a method for serializing local elements (the handle needn't be serialized)
   *                      static void SerializeRegisterEntry(tOutputStream& stream, const TEntry& entry);
   *                      (2) an equivalent method for deserializing element as TRemoteEntry (the handle needn't be deserialized)
   *                      void DeserializeRegisterEntry(tInputStream& stream);
   * \throw Throws std::invalid_argument on already occupied uid
   */
  template <typename TRemoteEntry>
  static void Register(const typename TRemoteEntry::tLocalRegister& r, uint uid);

  /*!
   * Removes listener from tRegister
   *
   * \param uid Uid of register
   * \param address Address of listener
   */
  static bool RemoveListener(uint uid, const void* address)
  {
    return RegisteredRegisters()[uid]->RemoveListener(address);
  }

  /*!
   * \param uid Uid of register
   * \return Current size of register
   */
  static size_t Size(uint uid)
  {
    return RegisteredRegisters()[uid]->size->load();
  }

//----------------------------------------------------------------------
// Private fields and methods
//----------------------------------------------------------------------
private:

  friend class tInputStream;
  friend class tOutputStream;

  /*! Info on every registered register */
  struct tPerRegisterInfo
  {
    virtual ~tPerRegisterInfo();
    virtual void AddListener(const std::function<void()>& callback, const void* address) = 0;
    virtual tRemoteRegisterBase* CreateRemoteRegister() = 0;
    virtual bool RemoveListener(const void* address) = 0;
    virtual void SerializeEntries(tOutputStream& stream, uint start_element, uint end_element) = 0;
    const std::atomic<size_t>* size;
    const void* raw_register_pointer;
  };

  /*!
   * Deserializes remote register entry from input stream
   *
   * \param stream Stream to deserialize from
   * \param register_vector Input stream vector that stored remote registers
   */
  template <typename TRemoteType>
  static const TRemoteType& DeserializeRemoteRegisterEntry(tInputStream& stream, std::array<std::unique_ptr<tInputStream::tRemoteRegister>, cMAX_PUBLISHED_REGISTERS>& register_array);

  /*!
   * Looks up register uid for type TRemoteEntry
   *
   * \return Register uid, -1 if no register for this type has been registered
   */
  template <typename TRemoteEntry>
  static int GetRemoteEntryRegisterUid()
  {
    return RemoteEntryRegisterUid<TRemoteEntry>::Uid();
  }

  /*!
   * \return Registered registers
   */
  static std::array<std::unique_ptr<tPerRegisterInfo>, cMAX_PUBLISHED_REGISTERS>& RegisteredRegisters();

  /*!
   * Serializes entries from shared register to stream
   *
   * \param stream Stream to serialize to
   * \param uid Uid of register
   * \param start_element First element to serialize
   * \param end_element One past the last element to serialize
   */
  static void SerializeEntries(tOutputStream& stream, uint uid, uint start_element, uint end_element)
  {
    RegisteredRegisters()[uid]->SerializeEntries(stream, start_element, end_element);
  }
};

template <typename THandle, typename TLocalRegister>
inline tOutputStream& operator<<(tOutputStream& stream, const PublishedRegisters::tRemoteEntryBase<THandle, TLocalRegister>& entry)
{
  stream << entry.GetHandle();
  return stream;
}

//----------------------------------------------------------------------
// End of namespace declaration
//----------------------------------------------------------------------
}
}

#include "rrlib/serialization/PublishedRegisters.hpp"


#endif
