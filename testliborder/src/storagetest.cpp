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
