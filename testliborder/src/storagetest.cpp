// {{{ GPL License

// This file is part of libcsp - a library for handling linear constraints.
// Copyright (C) 2016  Max Ostrowski

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

// }}}

#include <cppunit/extensions/HelperMacros.h>
#include "order/storage.h"
#include "test/mysolver.h"
#include "order/normalizer.h"
#include <iostream>

using namespace order;
class StorageTest : public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE( StorageTest );
    CPPUNIT_TEST( testAddition );
    CPPUNIT_TEST_SUITE_END();
private:
public:
    void setUp()
    {
    }

    void tearDown()
    {
    }

    void testAddition()
    {
        
    }


};

CPPUNIT_TEST_SUITE_REGISTRATION (StorageTest);
