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
#include "rrlib/serialization/tSerializable.h"
#include "rrlib/serialization/serialization.h"
#include "rrlib/xml2_wrapper/tXMLNode.h"

namespace rrlib
{
namespace serialization
{
void tSerializable::Deserialize(tStringInputStream& s)
{
  serialization::DeserializeFromHexString(this, s);
}

void tSerializable::Deserialize(const xml2::tXMLNode& node)
{
  tStringInputStream is(node.GetTextContent());
  Deserialize(is);
}

void tSerializable::Deserialize(serialization::tInputStream& is, serialization::tDataEncoding enc)
{
  serialization::Deserialize(is, *this, enc);
}


void tSerializable::Serialize(tStringOutputStream& os) const
{
  serialization::SerializeToHexString(this, os);
}

void tSerializable::Serialize(xml2::tXMLNode& node) const
{
  node.SetContent(serialization::Serialize(*this));
}

void tSerializable::Serialize(serialization::tOutputStream& os, serialization::tDataEncoding enc) const
{
  serialization::Serialize(os, *this, enc);
}

} // namespace rrlib
} // namespace serialization

