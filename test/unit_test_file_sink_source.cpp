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
/*!\file    unit_test_file_sink_source.cpp
 *
 * \author  Michael Arndt
 *
 * \date    2013-09-10
 *
 */
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// External includes (system with <>, local with "")
//----------------------------------------------------------------------
#include <cstdlib>
#include <iostream>

#include "rrlib/util/tUnitTestSuite.h"

//----------------------------------------------------------------------
// Internal includes with ""
//----------------------------------------------------------------------
#include "rrlib/serialization/tFileSink.h"
#include "rrlib/serialization/tFileSource.h"
#include "rrlib/serialization/tOutputStream.h"
#include "rrlib/serialization/tInputStream.h"

#include "rrlib/util/sFileIOUtils.h"

//----------------------------------------------------------------------
// Debugging
//----------------------------------------------------------------------
#include <cassert>



//----------------------------------------------------------------------
// Namespace usage
//----------------------------------------------------------------------
using namespace rrlib::serialization;
using rrlib::util::sFileIOUtils;

//----------------------------------------------------------------------
// Forward declarations / typedefs / enums
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// Const values
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// Implementation
//----------------------------------------------------------------------

class tTestFileSinkSource : public rrlib::util::tUnitTestSuite
{
  RRLIB_UNIT_TESTS_BEGIN_SUITE(tTestFileSinkSource);
  RRLIB_UNIT_TESTS_ADD_TEST(TestSinkUnwritable);
  RRLIB_UNIT_TESTS_ADD_TEST(TestSourceUnreadable);
  RRLIB_UNIT_TESTS_ADD_TEST(TestSinkSource);
  RRLIB_UNIT_TESTS_END_SUITE;

private:

  virtual void InitializeTests()
  {

  }
  virtual void CleanUp() {}

  void TestSinkUnwritable()
  {
    tFileSink sink("/hopefully/non-existent/path");
    RRLIB_UNIT_TESTS_EXCEPTION_MESSAGE("An exception must be thrown when creating the stream", tOutputStream os(sink), std::ios_base::failure);
  }

  void TestSourceUnreadable()
  {
    RRLIB_UNIT_TESTS_EXCEPTION_MESSAGE("An exception must be thrown when creating the file source", tFileSource src("/hopefully/non-existent/path"), std::runtime_error);
  }

  void TestSinkSource()
  {
    std::string path = rrlib::util::sFileIOUtils::CreateTempfile();
    int test_int = 42;
    std::string test_string("This is some string that will be serialized");

    // serialize something
    tFileSink sink(path);
    tOutputStream os(sink);
    os << test_int << test_string;
    os.Close();

    int test_int_;
    std::string test_string_;

    // read it back
    tFileSource src(path);
    tInputStream is(src);
    is >> test_int_ >> test_string_;
    is.Close();

    RRLIB_UNIT_TESTS_EQUALITY_MESSAGE("Written and read integer must be equal", test_int, test_int_);
    RRLIB_UNIT_TESTS_EQUALITY_MESSAGE("Written and read string must be equal", test_string, test_string_);
  }

};

RRLIB_UNIT_TESTS_REGISTER_SUITE(tTestFileSinkSource);
