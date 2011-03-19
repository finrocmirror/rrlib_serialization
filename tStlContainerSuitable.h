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

#ifndef __rrlib__serialization__tStlContainerSuitable_h__
#define __rrlib__serialization__tStlContainerSuitable_h__

#include <boost/utility.hpp>

/*!
 * \author Max Reichardt
 *
 * This class provides the trait tStlContainerSuitable, which indicates whether
 * object of type T can be used in STL containers directly.
 * This is usually the case, if a class is either copyable or movable.
 *
 * Custom specializations of this trait may be added by template specialization.
 */

namespace rrlib
{
namespace serialization
{

// By deriving from this class STL container suitability can be indicated.
class tStlSuitable {};

// By deriving from this class STL container unsuitability can be indicated.
class tStlUnsuitable : public boost::noncopyable {};


template <typename T, bool SUITABLE, bool UNSUITABLE, bool NONCOPYABLE>
struct tStlContainerSuitableImpl
{
  enum { value = 1 };
};

template <typename T, bool NONCOPYABLE>
struct tStlContainerSuitableImpl<T, true, false, NONCOPYABLE>
{
  enum { value = 1 };
};

template <typename T, bool NONCOPYABLE>
struct tStlContainerSuitableImpl<T, false, true, NONCOPYABLE>
{
  enum { value = 0 };
};

template <typename T>
struct tStlContainerSuitableImpl<T, false, false, true>
{
  enum { value = 0 };
};

template <typename T, bool SUITABLE, bool UNSUITABLE, bool NONCOPYABLE>
struct tStlContainerSuitableImpl<std::vector<T>, SUITABLE, UNSUITABLE, NONCOPYABLE>
{
  enum { value = 0 };
};

template <typename T>
struct tStlContainerSuitable : public tStlContainerSuitableImpl<T, boost::is_base_of<tStlSuitable, T>::value, boost::is_base_of<tStlUnsuitable, T>::value, boost::is_base_of<boost::noncopyable, T>::value>
{

};

} // namespace
} // namespace

#endif // __rrlib__serialization__tStlContainerSuitable_h__
