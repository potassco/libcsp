#include <cppunit/extensions/HelperMacros.h>
#include "order/domain.h"
#include "order/config.h"
#include <iostream>

using namespace order;
class DomainTest : public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE( DomainTest );
    CPPUNIT_TEST( testAddition );
    CPPUNIT_TEST( testSets );
    CPPUNIT_TEST( test_lower_bound );
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
        Domain d(0,100);
        d.unify(200,300);
        d.unify(-100,-50);
        CPPUNIT_ASSERT(d.size()==253);
        Domain::const_iterator begin = d.begin();
        Domain::const_iterator end = d.end();
        int count = 0;
        for (auto i = begin; i != end; ++i)
        {
            ++count;
            if (count==57)
            {
                CPPUNIT_ASSERT(*i==5);
            }
        }
        CPPUNIT_ASSERT(count==253);
        CPPUNIT_ASSERT(end-begin==253);
        CPPUNIT_ASSERT(*(begin+50)==-50);
        CPPUNIT_ASSERT(*(begin+51)==0);
        CPPUNIT_ASSERT(*(begin+76)==25);
        CPPUNIT_ASSERT(*(begin+252)==300);

        CPPUNIT_ASSERT(*(std::lower_bound(begin, end, 20))==20);
        CPPUNIT_ASSERT(*(std::lower_bound(begin, end, -20))==0);
        CPPUNIT_ASSERT(*(std::lower_bound(begin, end, -30))==0);
        CPPUNIT_ASSERT(*(std::lower_bound(begin, end, -50))==-50);
        CPPUNIT_ASSERT(*(std::lower_bound(begin, end, 75))==75);
        CPPUNIT_ASSERT(*(std::lower_bound(begin, end, 100))==100);
        CPPUNIT_ASSERT(*(std::lower_bound(begin, end, 200))==200);
        CPPUNIT_ASSERT(*(std::lower_bound(begin, end, 150))==200);
        CPPUNIT_ASSERT(*(std::lower_bound(begin, end, 101))==200);
        CPPUNIT_ASSERT((std::lower_bound(begin, end, 1500))==end);


        Domain e(0,100);
        e.unify(-100,50);
        Domain::const_iterator begin2 = e.begin();
        Domain::const_iterator end2 = e.end();
        
        CPPUNIT_ASSERT(end2-begin2==201);
        CPPUNIT_ASSERT(e.size()==201);


        Domain f(0,0);
        f.unify(d);
        CPPUNIT_ASSERT(f==d);
        f.unify(e);

        Domain g(-100,100);
        g.unify(200,300);
        CPPUNIT_ASSERT(g==f);

        g.unify(101,199);
        CPPUNIT_ASSERT(g.size()==401);


        unsigned int domSize=10000;
        Domain h(-10,5);
        h.inplace_times(5, domSize);
        CPPUNIT_ASSERT(*h.begin()==-50);
        CPPUNIT_ASSERT(*(h.end()-1)==25);
        h.unify(50,75);
        h.inplace_times(-3, domSize);
        CPPUNIT_ASSERT(*h.begin()==-225);
        CPPUNIT_ASSERT(*(h.end()-1)==150);
        //std::cout << h << std::endl;

        e+=h;

        CPPUNIT_ASSERT(*e.begin()==-325);
        CPPUNIT_ASSERT(*(e.end()-1)==250);

        Domain i(1,1);
        i.unify(5,6);

        CPPUNIT_ASSERT(h+i==i+h);
        i+=h;
        CPPUNIT_ASSERT(*i.begin()==-224);
        CPPUNIT_ASSERT(*(i.end()-1)==156);

        Domain j(1,1);
        j.unify(6,8);
        j.unify(2,5);

        CPPUNIT_ASSERT(j.getRanges().size()==1);
        CPPUNIT_ASSERT(j.getRanges().back()==Range(1,8));
    }

    void testSets()
    {
        unsigned int domSize=10000;
        Domain d(0,100);
        d.inplace_times(5, domSize);
        d.inplace_times(-43, domSize);
        CPPUNIT_ASSERT(d.size()==101);

        Domain e(1,100);
        e += Domain(3,3);

        CPPUNIT_ASSERT(e.lower()==4);
        CPPUNIT_ASSERT(e.upper()==103);
        CPPUNIT_ASSERT(e.size()==100);

        e += Domain(1000,1999);
        CPPUNIT_ASSERT(e.size()==1099);

        Domain f(0,99);
        f.intersect(50,70);
        CPPUNIT_ASSERT(f.lower()==50);
        CPPUNIT_ASSERT(f.upper()==70);
        f.intersect(65,60);
        CPPUNIT_ASSERT(f.empty());

        Domain g(0,99);
        g.remove(10,19);
        g.remove(50,59);
        g.remove(20,29);
        CPPUNIT_ASSERT(g.size()==100-30);
        g.remove(70,79);
        g.remove(65,69);
        CPPUNIT_ASSERT(g.size()==100-30-15);
        g.remove(7,62);
        CPPUNIT_ASSERT(g.size()==29);
        g.intersect(23,72);
        CPPUNIT_ASSERT(g.size()==2);
        g.unify(0,99);
        CPPUNIT_ASSERT(g.size()==100);


        Domain h(0,13);
        h.unify(22,34);
        h.unify(58,96);
        h.unify(1000,1000);
        h.unify(34,-27);

        Domain i(-17,4);
        i.unify(97,97);
        i.unify(27,29);
        i.unify(30,60);

        h.unify(i);

        Domain erg(-17,13);
        erg.unify(22,97);
        erg.unify(1000,1000);
        CPPUNIT_ASSERT(erg==h);


        Domain k(0,1000);
        k.remove(101,899);
        Domain j(5,10);
        j.unify(20,30);
        j.unify(40,50);
        k.intersect(j);
        CPPUNIT_ASSERT(k.size()==28);

    }

    void test_lower_bound()
    {
        {
        Domain d(1,100);

        Restrictor r(View(0,-1,0),d);

        CPPUNIT_ASSERT(r.begin() < r.end());

        }

        Domain d(0,100);
        d.unify(200,300);
        d.unify(-100,-50);

        {
        Restrictor r(View(0,1,0),d);

        CPPUNIT_ASSERT(*(order::wrap_lower_bound(r.begin(), r.end(), 50))==50);
        CPPUNIT_ASSERT(*(order::wrap_lower_bound(r.begin(), r.end(), 0))==0);
        CPPUNIT_ASSERT(*(order::wrap_lower_bound(r.begin(), r.end(), -1))==0);
        CPPUNIT_ASSERT(*(order::wrap_lower_bound(r.begin(), r.end(), -101))==-100);
        CPPUNIT_ASSERT(*(order::wrap_lower_bound(r.begin(), r.end(), -500))==-100);
        CPPUNIT_ASSERT(*(order::wrap_lower_bound(r.begin(), r.end(), -50))==-50);
        CPPUNIT_ASSERT(*(order::wrap_lower_bound(r.begin(), r.end(), 300))==300);
        CPPUNIT_ASSERT(order::wrap_lower_bound(r.begin(), r.end(), 301)==r.end());
        CPPUNIT_ASSERT(order::wrap_lower_bound(r.begin(), r.end(), 12030)==r.end());
        }

        {
        Restrictor r(View(0,1,1000),d);

        CPPUNIT_ASSERT(*(order::wrap_lower_bound(r.begin(), r.end(), 1000+ 50))  ==1000+ 50);
        CPPUNIT_ASSERT(*(order::wrap_lower_bound(r.begin(), r.end(), 1000+ 0))   ==1000+ 0);
        CPPUNIT_ASSERT(*(order::wrap_lower_bound(r.begin(), r.end(), 1000+ -1))  ==1000+ 0);
        CPPUNIT_ASSERT(*(order::wrap_lower_bound(r.begin(), r.end(), 1000+ -101))==1000+ -100);
        CPPUNIT_ASSERT(*(order::wrap_lower_bound(r.begin(), r.end(), 1000+ -500))==1000+ -100);
        CPPUNIT_ASSERT(*(order::wrap_lower_bound(r.begin(), r.end(), 1000+ -50)) ==1000+ -50);
        CPPUNIT_ASSERT(*(order::wrap_lower_bound(r.begin(), r.end(), 1000+ 300)) ==1000+ 300);
        CPPUNIT_ASSERT(order::wrap_lower_bound(r.begin(), r.end(), 1000+ 301)==r.end());
        CPPUNIT_ASSERT(order::wrap_lower_bound(r.begin(), r.end(), 1000+ 12030)==r.end());
        }

        {
        Restrictor r(View(0,-1,0),d);


        CPPUNIT_ASSERT(*(order::wrap_lower_bound(r.begin(), r.end(), -300))==-300);
        CPPUNIT_ASSERT(*(order::wrap_lower_bound(r.begin(), r.end(), -200))==-200);
        CPPUNIT_ASSERT(*(order::wrap_lower_bound(r.begin(), r.end(), -199))==-100);
        CPPUNIT_ASSERT(*(order::wrap_lower_bound(r.begin(), r.end(), -201))==-201);
        CPPUNIT_ASSERT(*(order::wrap_lower_bound(r.begin(), r.end(), -100))==-100);
        CPPUNIT_ASSERT(*(order::wrap_lower_bound(r.begin(), r.end(), -101))==-100);
        CPPUNIT_ASSERT(*(order::wrap_lower_bound(r.begin(), r.end(), -99))==-99);
        CPPUNIT_ASSERT(*(order::wrap_lower_bound(r.begin(), r.end(), -1))==-1);
        CPPUNIT_ASSERT(*(order::wrap_lower_bound(r.begin(), r.end(), 0))==0);
        CPPUNIT_ASSERT(*(order::wrap_lower_bound(r.begin(), r.end(), 1))==50);
        CPPUNIT_ASSERT(*(order::wrap_lower_bound(r.begin(), r.end(), 25))==50);
        CPPUNIT_ASSERT(*(order::wrap_lower_bound(r.begin(), r.end(), 50))==50);
        CPPUNIT_ASSERT(*(order::wrap_lower_bound(r.begin(), r.end(), 0))==0);
        CPPUNIT_ASSERT(*(order::wrap_lower_bound(r.begin(), r.end(), -1))==-1);
        CPPUNIT_ASSERT(*(order::wrap_lower_bound(r.begin(), r.end(), 1))==50);
        CPPUNIT_ASSERT(*(order::wrap_lower_bound(r.begin(), r.end(), -101))==-100);
        CPPUNIT_ASSERT(*(order::wrap_lower_bound(r.begin(), r.end(), -500))==-300);
        CPPUNIT_ASSERT(*(order::wrap_lower_bound(r.begin(), r.end(), -50))==-50);
        CPPUNIT_ASSERT(*(order::wrap_lower_bound(r.begin(), r.end(), 100))==100);
        CPPUNIT_ASSERT(order::wrap_lower_bound(r.begin(), r.end(), 101)==r.end());
        CPPUNIT_ASSERT(order::wrap_lower_bound(r.begin(), r.end(), 12030)==r.end());
        }

    }
};

CPPUNIT_TEST_SUITE_REGISTRATION (DomainTest);
