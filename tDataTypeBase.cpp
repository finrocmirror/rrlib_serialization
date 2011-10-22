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
#include <cstring>
#include "rrlib/serialization/sSerialization.h"
#include "rrlib/serialization/tDataTypeAnnotation.h"

#include "rrlib/serialization/tDataTypeBase.h"

namespace rrlib
{
namespace serialization
{
tDataTypeBase::tDataTypeBase(tDataTypeInfoRaw* info_) :
    info(info_)
{
  if (info_ != NULL && info_->new_info == true)
  {
    std::unique_lock<std::recursive_mutex>(GetMutex());
    AddType(info_);
    info_->Init();

  }
}

void tDataTypeBase::AddType(tDataTypeInfoRaw* nfo)
{
  nfo->uid = static_cast<int16_t>(GetTypes().size());
  GetTypes().push_back(*this);
  nfo->new_info = false;
  std::string msg("Adding data type ");
  msg += GetName();
  RRLIB_LOG_PRINT(logging::eLL_DEBUG_VERBOSE_1, log_domain, msg);
}

void* tDataTypeBase::CreateInstance(void* placement) const
{
  if (info == NULL)
  {
    return NULL;
  }
  return info->CreateInstance(placement);
}

tDataTypeBase tDataTypeBase::FindType(const std::string& name)
{
  bool nulltype = strcmp(name.c_str(), "NULL") == 0;
  if (nulltype)
  {
    return GetNullType();
  }

  for (size_t i = 0u; i < GetTypes().size(); i++)
  {
    tDataTypeBase dt = GetTypes()[i];
    bool eq = name.compare(dt.GetName()) == 0;
    if (eq)
    {
      return dt;
    }
  }

  return tDataTypeBase(NULL);
}

std::string tDataTypeBase::GetDataTypeNameFromRtti(const char* rtti)
{
  std::string demangled = sSerialization::Demangle(rtti);

  // remove ::
  long int last_pos = -1;
  for (long int i = demangled.size() - 1; i >= 0; i--)
  {
    char c = demangled[i];
    if (last_pos == -1)
    {
      if (c == ':')
      {
        last_pos = i + 1;

        // possibly cut off s or t prefix
        if (islower(demangled[last_pos]) && isupper(demangled[last_pos + 1]))
        {
          last_pos++;
        }
      }
    }
    else
    {
      if ((!isalnum(c)) && c != ':' && c != '_')
      {
        // ok, cut off here
        demangled = demangled.substr(0, i + 1) + demangled.substr(last_pos, demangled.size() - last_pos);
        last_pos = -1;
      }
    }
  }

  // ok, cut off rest
  if (last_pos > 0)
  {
    demangled = demangled.substr(last_pos, demangled.size() - last_pos);
  }

  // possibly cut off s or t prefix
  if (islower(demangled[0]) && isupper(demangled[1]))
  {
    demangled.erase(0, 1);
  }

  return demangled;

}

tDataTypeBase::tDataTypeInfoRaw::tDataTypeInfoRaw() :
    type(ePLAIN),
    name(),
    rtti_name(NULL),
    size(-1),
    type_traits(0),
    new_info(true),
    default_name(true),
    uid(-1),
    element_type(NULL),
    list_type(NULL),
    shared_ptr_list_type(NULL),
    binary()
{
  for (size_t i = 0; i < cMAX_ANNOTATIONS; i++)
  {
    annotations[i] = NULL;
  }
}

tDataTypeBase::tDataTypeInfoRaw::~tDataTypeInfoRaw()
{
  for (size_t i = 0; i < cMAX_ANNOTATIONS; i++)
  {
    delete annotations[i];
  }

}

void tDataTypeBase::tDataTypeInfoRaw::SetName(const std::string& new_name)
{
  if (!default_name)
  {
    assert(name.compare(new_name) == 0 && "Name already set");

    return;
  }

  default_name = false;
  name = new_name;

  if (list_type != NULL)
  {
    list_type->name = std::string("List<") + name + ">";
  }
  if (shared_ptr_list_type != NULL)
  {
    shared_ptr_list_type->name = std::string("List<") + name + "*>";
  }

}

} // namespace rrlib
} // namespace serialization

