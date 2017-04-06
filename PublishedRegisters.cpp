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
/*!\file    rrlib/serialization/PublishedRegisters.cpp
 *
 * \author  Max Reichardt
 *
 * \date    2017-02-05
 *
 */
//----------------------------------------------------------------------
#include "rrlib/serialization/PublishedRegisters.h"

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

std::array<std::unique_ptr<PublishedRegisters::tPerRegisterInfo>, cMAX_PUBLISHED_REGISTERS>& PublishedRegisters::RegisteredRegisters()
{
  static std::array<std::unique_ptr<PublishedRegisters::tPerRegisterInfo>, cMAX_PUBLISHED_REGISTERS> instance;
  return instance;
}

PublishedRegisters::tRemoteRegisterBase::~tRemoteRegisterBase()
{}

PublishedRegisters::tPerRegisterInfo::~tPerRegisterInfo()
{}

//----------------------------------------------------------------------
// End of namespace declaration
//----------------------------------------------------------------------
}
}
