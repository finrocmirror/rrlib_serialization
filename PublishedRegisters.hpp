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
/*!\file    rrlib/serialization/PublishedRegisters.hpp
 *
 * \author  Max Reichardt
 *
 * \date    2017-02-09
 *
 */
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// External includes (system with <>, local with "")
//----------------------------------------------------------------------
#include "rrlib/concurrent_containers/tRegister.h"

//----------------------------------------------------------------------
// Internal includes with ""
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// Debugging
//----------------------------------------------------------------------
#include <cassert>

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

template <typename TRemoteType>
const TRemoteType& PublishedRegisters::DeserializeRemoteRegisterEntry(tInputStream& stream, std::array<std::unique_ptr<tInputStream::tRemoteRegister>, cMAX_PUBLISHED_REGISTERS>& register_array)
{
  int register_uid = GetRemoteEntryRegisterUid<TRemoteType>();
  if (register_uid < 0)
  {
    throw std::runtime_error("TRemoteType must be registered ad PublishedRegisters first");
  }
  if (!register_array[register_uid])
  {
    register_array[register_uid].reset(RegisteredRegisters()[register_uid]->CreateRemoteRegister());
  }
  const typename TRemoteType::tHandle cESCAPE = -2;
  auto handle = stream.ReadNumber<typename TRemoteType::tHandle>();
  if (handle == cESCAPE)
  {
    stream.ReadRegisterUpdatesImplementation();
    handle = stream.ReadNumber<typename TRemoteType::tHandle>();
  }
  return *reinterpret_cast<const TRemoteType*>(static_cast<tRemoteRegisterBase*>(register_array[register_uid].get())->GetRemoteElement(handle));
}

template <typename TElement>
static const TElement& EmptyElement()
{
  static const TElement cEMPTY_ELEMENT;
  return cEMPTY_ELEMENT;
}

template <typename TRemoteEntry, bool Tempty_element>
void PublishedRegisters::Register(const typename TRemoteEntry::tLocalRegister& r, uint uid)
{
  typedef typename TRemoteEntry::tLocalRegister tReg;
  typedef typename tReg::tHandle tHandle;
  static_assert(std::is_same<typename TRemoteEntry::tHandle, tHandle>::value, "Handle types must match");

  class tRemoteRegisterInstance : public tRemoteRegister<TRemoteEntry>
  {
    typedef tRemoteRegister<TRemoteEntry> tBase;

  public:
    tRemoteRegisterInstance()
    {
      tRemoteRegisterBase::size = &tBase::SizeAtomic();
    }

  private:
    virtual void DeserializeEntries(tInputStream& stream) override
    {
      TRemoteEntry t;
      if (stream.GetSourceInfo().revision == 0)
      {
        if (tBase::Size() == 0)
        {
          stream.ReadShort();
        }

        // legacy type exchange support
        short s = stream.ReadShort();
        while (s != -1)
        {
          t.DeserializeRegisterEntry(stream);
          tBase::SetHandle(t, tBase::Size());
          tBase::Add(t);
          s = stream.ReadShort();
        }
      }
      else
      {
        size_t count = stream.ReadInt();
        for (size_t i = 0; i < count; i++)
        {
          tBase::SetHandle(t, tBase::Size());
          t.DeserializeRegisterEntry(stream);
          tBase::Add(t);
        }
      }
    }

    virtual const void* GetRemoteElement(size_t index) override
    {
      tHandle handle = static_cast<tHandle>(index);
      if (Tempty_element && handle == static_cast<tHandle>(-1))
      {
        return &EmptyElement<typename std::conditional<Tempty_element, TRemoteEntry, std::string>::type>();
      }
      if (index >= this->Size())
      {
        throw std::runtime_error("Read invalid index deserializing remote register entry");
      }
      return &((*this)[index]);
    }
  };

  struct tInfo : tPerRegisterInfo
  {
    virtual tRemoteRegisterBase* CreateRemoteRegister() override
    {
      return new tRemoteRegisterInstance();
    }
    virtual void AddListener(const std::function<void()>& callback, const void* address) override
    {
      reg->AddListener(callback, address);
    }
    virtual bool RemoveListener(const void* address) override
    {
      return reg->RemoveListener(address);
    }
    virtual void SerializeEntries(tOutputStream& stream, uint start_element, uint end_element) override
    {
      for (size_t i = start_element; i < end_element; i++)
      {
        TRemoteEntry::SerializeRegisterEntry(stream, (*reg)[i]);
      }
    }
    const tReg* reg; // Pointer to register
  };

  auto& infos = RegisteredRegisters();
  if (infos[uid] && infos[uid]->raw_register_pointer != &r)
  {
    throw std::invalid_argument("Uid already occupied with different register");
  }
  r.uid = uid;
  if (infos[uid])
  {
    return;
  }

  std::unique_ptr<tInfo> info(new tInfo());
  info->reg = &r;
  info->raw_register_pointer = info->reg;
  info->size = &(r.SizeAtomic());
  infos[uid] = std::move(info);
  RemoteEntryRegisterUid<TRemoteEntry>::Uid() = uid;
}

template <typename TRemoteEntry>
inline const TRemoteEntry& tInputStream::ReadRegisterEntry()
{
  if (!shared_serialization_info.remote_registers)
  {
    throw std::runtime_error("tInputStream: No shared serialization info set");
  }
  return PublishedRegisters::DeserializeRemoteRegisterEntry<TRemoteEntry>(*this, *shared_serialization_info.remote_registers);
}

//----------------------------------------------------------------------
// End of namespace declaration
//----------------------------------------------------------------------
}
}
