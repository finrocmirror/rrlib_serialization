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

#ifndef __rrlib__serialization__clear_h__
#define __rrlib__serialization__clear_h__

#include <vector>
#include <list>
#include <deque>

/*!
 * \author Max Reichardt
 *
 * This file contains the clear namespace with diverse
 * functions for releasing any shared objects an object might hold on to.
 *
 * More functions for custom types may be added to this namespace
 * by a user of this library.
 */

namespace rrlib
{
namespace serialization
{
namespace clear
{

inline void Clear(void* p)
{
  // do nothing by default
}

template <typename T>
inline void Clear(std::vector<std::shared_ptr<T> >* v)
{
  v.clear();
}

template <typename T>
inline void Clear(std::list<std::shared_ptr<T> >* v)
{
  v.clear();
}

template <typename T>
inline void Clear(std::deque<std::shared_ptr<T> >* v)
{
  v.clear();
}

} // namespace
} // namespace
} // namespace

#endif // __rrlib__serialization__clear_h__
