/**
 * You received this file as part of RRLib serialization
 *
 * Copyright (C) 2011 Max Reichardt,
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

#ifndef __rrlib__serialization__detail__tStringInputStreamFallback_h__
#define __rrlib__serialization__detail__tStringInputStreamFallback_h__

#include <string>

namespace rrlib
{
namespace serialization
{
namespace detail
{
/*!
 * \author Max Reichardt
 *
 * Wrapper tStringInputStream to implement fallback mechanism
 * when operators are not overloaded for XML nodes.
 */
class tStringInputStreamFallback : public tStringInputStream
{
public:
  const xml2::tXMLNode& node;

  tStringInputStreamFallback(const xml2::tXMLNode& node_) :
      tStringInputStream(node_.GetTextContent()),
      node(node_)
  {}

};

} // namespace
} // namespace
} // namespace

template <typename T>
inline const rrlib::xml2::tXMLNode& operator>> (rrlib::serialization::detail::tStringInputStreamFallback && is, T& t)
{
  static_cast<rrlib::serialization::tStringInputStream&>(is) >> t;
  return is.node;
}

inline const rrlib::xml2::tXMLNode& operator>> (const rrlib::xml2::tXMLNode& node, std::string& s)
{
  s = node.GetTextContent();
  return node;
}

template <typename T>
inline const rrlib::xml2::tXMLNode& operator>> (const rrlib::xml2::tXMLNode& n, std::vector<T>& v)
{
  v.clear();
  for (rrlib::xml2::tXMLNode::const_iterator node = n.GetChildrenBegin(); node != n.GetChildrenEnd(); ++node)
  {
    assert(node->GetName().compare("element") == 0);
    v.push_back(rrlib::serialization::sStaticFactory<T>::CreateByValue());
    (*node) >> rrlib::serialization::detail::tListElemInfo<T>::GetElem(v[v.size() - 1]);
  }
  return n;
}

#endif // __rrlib__serialization__detail__tStringInputStreamFallback_h__
