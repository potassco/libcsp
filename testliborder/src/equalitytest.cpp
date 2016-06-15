#include <cppunit/extensions/HelperMacros.h>
#include "test/mysolver.h"
#include "order/normalizer.h"
#include "clasp/clasp_facade.h"
#include "order/equality.h"
#include "order/configs.h"
#include <iostream>

using namespace order;

namespace
{

}
class EqualityTest : public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE( EqualityTest );
    CPPUNIT_TEST( test1 );
    CPPUNIT_TEST( test2 );
    CPPUNIT_TEST( test3 );
    CPPUNIT_TEST( test4 );
    CPPUNIT_TEST( test5 );
    CPPUNIT_TEST( test6 );
    CPPUNIT_TEST( test7 );
    CPPUNIT_TEST( test8 );
    CPPUNIT_TEST( test9 );
    CPPUNIT_TEST( test10 );
    CPPUNIT_TEST_SUITE_END();
private:
public:
    void setUp()
    {
    }

    void tearDown()
    {
    }


    
    void test1()
    {
        MySolver s;
        Normalizer n(s,lazySolveConfigProp4);
        EqualityProcessor p(s,n.getVariableCreator());

        std::vector<ReifiedLinearConstraint> lc;
        View a = n.createView();
        View b = n.createView();
        {
            LinearConstraint l(LinearConstraint::Relation::EQ);
            l.add(a);
            l.add(b*-1);
            l.normalize();
            lc.emplace_back(ReifiedLinearConstraint(std::move(l),s.trueLit(),false));
        }
        CPPUNIT_ASSERT(p.process(lc));
        CPPUNIT_ASSERT(p.getEqualities(a.v)==p.getEqualities(b.v));
        CPPUNIT_ASSERT(lc.size()==0);
    }

    void test2()
    {
        MySolver s;
        Normalizer n(s,lazySolveConfigProp4);
        EqualityProcessor p(s,n.getVariableCreator());

        std::vector<ReifiedLinearConstraint> lc;
        View a = n.createView();

        {
            LinearConstraint l(LinearConstraint::Relation::EQ);
            l.add(a);
            l.normalize();
            lc.emplace_back(ReifiedLinearConstraint(std::move(l),s.trueLit(),false));
        }
        CPPUNIT_ASSERT(p.process(lc));
        CPPUNIT_ASSERT(lc.size()==0);
    }


    void test3()
    {
        MySolver s;
        Normalizer n(s,lazySolveConfigProp4);
        EqualityProcessor p(s,n.getVariableCreator());

        std::vector<ReifiedLinearConstraint> lc;
        {
            LinearConstraint l(LinearConstraint::Relation::EQ);
            l.addRhs(3);
            l.normalize();
            lc.emplace_back(ReifiedLinearConstraint(std::move(l),s.trueLit(),false));
        }
        CPPUNIT_ASSERT(!p.process(lc));
    }

    void test4()
    {
        MySolver s;
        Normalizer n(s,lazySolveConfigProp4);
        EqualityProcessor p(s,n.getVariableCreator());

        View a = n.createView();
        View b = n.createView();

        std::vector<ReifiedLinearConstraint> lc;
        {
            LinearConstraint l(LinearConstraint::Relation::EQ);
            l.add(a);
            l.add(b*-1);
            l.addRhs(3);
            l.normalize();
            lc.emplace_back(ReifiedLinearConstraint(std::move(l),s.trueLit(),false));
        }
        CPPUNIT_ASSERT(p.process(lc));

        CPPUNIT_ASSERT(p.getEqualities(a.v)==p.getEqualities(b.v));
        CPPUNIT_ASSERT(p.getEqualities(a.v)->top()==a.v);
        CPPUNIT_ASSERT(p.getEqualities(a.v)->getConstraints().at(b.v).constant==-3);
    }


    void test5()
    {
        MySolver s;
        Normalizer n(s,lazySolveConfigProp4);
        EqualityProcessor p(s,n.getVariableCreator());

        View a = n.createView();
        View b = n.createView();
        View c = n.createView();
        View d = n.createView();

        std::vector<ReifiedLinearConstraint> lc;
        {
            LinearConstraint l(LinearConstraint::Relation::EQ);
            l.add(a);
            l.add(b);
            l.add(c);
            l.add(d*-1);
            l.addRhs(-3);
            l.normalize();
            lc.emplace_back(ReifiedLinearConstraint(std::move(l),s.trueLit(),false));
        }
        {
            LinearConstraint l(LinearConstraint::Relation::EQ);
            l.add(a);
            l.add(b);
            l.add(c*-1);
            l.addRhs(-2);
            l.normalize();
            lc.emplace_back(ReifiedLinearConstraint(std::move(l),s.trueLit(),false));
        }
        {
            LinearConstraint l(LinearConstraint::Relation::EQ);
            l.add(a);
            l.add(b*-1);
            l.addRhs(1);
            l.normalize();
            lc.emplace_back(ReifiedLinearConstraint(std::move(l),s.trueLit(),false));
        }

        CPPUNIT_ASSERT(p.process(lc));

        CPPUNIT_ASSERT(p.getEqualities(a.v)==p.getEqualities(b.v));
        CPPUNIT_ASSERT(p.getEqualities(b.v)==p.getEqualities(c.v));
        CPPUNIT_ASSERT(p.getEqualities(c.v)==p.getEqualities(d.v));
        CPPUNIT_ASSERT(p.getEqualities(a.v)->top()==a.v);
        CPPUNIT_ASSERT(p.getEqualities(a.v)->getConstraints().at(b.v).firstCoef==1);
        CPPUNIT_ASSERT(p.getEqualities(a.v)->getConstraints().at(b.v).secondCoef==1);
        CPPUNIT_ASSERT(p.getEqualities(a.v)->getConstraints().at(b.v).constant==-1);

        CPPUNIT_ASSERT(p.getEqualities(a.v)->getConstraints().at(c.v).firstCoef==1);
        CPPUNIT_ASSERT(p.getEqualities(a.v)->getConstraints().at(c.v).secondCoef==2);
        CPPUNIT_ASSERT(p.getEqualities(a.v)->getConstraints().at(c.v).constant==1);

        CPPUNIT_ASSERT(p.getEqualities(a.v)->getConstraints().at(d.v).firstCoef==1);
        CPPUNIT_ASSERT(p.getEqualities(a.v)->getConstraints().at(d.v).secondCoef==4);
        CPPUNIT_ASSERT(p.getEqualities(a.v)->getConstraints().at(d.v).constant==3);
    }
    
    
    void test6()
    {
        /// 2 times a = b + 3
        MySolver s;
        Normalizer n(s,lazySolveConfigProp4);
        EqualityProcessor p(s,n.getVariableCreator());

        View a = n.createView();
        View b = n.createView();

        std::vector<ReifiedLinearConstraint> lc;
        {
            LinearConstraint l(LinearConstraint::Relation::EQ);
            l.add(a);
            l.add(b*-1);
            l.addRhs(3);
            l.normalize();
            lc.emplace_back(ReifiedLinearConstraint(std::move(l),s.trueLit(),false));
        }
        
        {
            LinearConstraint l(LinearConstraint::Relation::EQ);
            l.add(a);
            l.add(b*-1);
            l.addRhs(3);
            l.normalize();
            lc.emplace_back(ReifiedLinearConstraint(std::move(l),s.trueLit(),false));
        }
        CPPUNIT_ASSERT(p.process(lc));

        CPPUNIT_ASSERT(p.getEqualities(a.v)==p.getEqualities(b.v));
        CPPUNIT_ASSERT(p.getEqualities(a.v)->top()==a.v);
        CPPUNIT_ASSERT(p.getEqualities(a.v)->getConstraints().at(b.v).constant==-3);
    }
    
    void test7()
    {
        // a = b + 3
        // a = b + 4
        // fail
        MySolver s;
        Normalizer n(s,lazySolveConfigProp4);
        EqualityProcessor p(s,n.getVariableCreator());

        View a = n.createView();
        View b = n.createView();

        std::vector<ReifiedLinearConstraint> lc;
        {
            LinearConstraint l(LinearConstraint::Relation::EQ);
            l.add(a);
            l.add(b*-1);
            l.addRhs(3);
            l.normalize();
            lc.emplace_back(ReifiedLinearConstraint(std::move(l),s.trueLit(),false));
        }
        
        {
            LinearConstraint l(LinearConstraint::Relation::EQ);
            l.add(a);
            l.add(b*-1);
            l.addRhs(4);
            l.normalize();
            lc.emplace_back(ReifiedLinearConstraint(std::move(l),s.trueLit(),false));
        }
        CPPUNIT_ASSERT(!p.process(lc));
    }
    
    
    void test8()
    {
        // 3*a = 2*b
        // 3*a = 4*b
        MySolver s;
        Normalizer n(s,lazySolveConfigProp4);
        EqualityProcessor p(s,n.getVariableCreator());

        View a = n.createView();
        View b = n.createView();

        std::vector<ReifiedLinearConstraint> lc;
        {
            LinearConstraint l(LinearConstraint::Relation::EQ);
            l.add(a*3);
            l.add(b*-2);
            l.addRhs(0);
            l.normalize();
            lc.emplace_back(ReifiedLinearConstraint(std::move(l),s.trueLit(),false));
        }
        
        {
            LinearConstraint l(LinearConstraint::Relation::EQ);
            l.add(a*3);
            l.add(b*-4);
            l.addRhs(0);
            l.normalize();
            lc.emplace_back(ReifiedLinearConstraint(std::move(l),s.trueLit(),false));
        }
        
        CPPUNIT_ASSERT(p.process(lc));
        CPPUNIT_ASSERT(lc.size()==0);
        
        CPPUNIT_ASSERT(p.getEqualities(a.v)==p.getEqualities(b.v));
        CPPUNIT_ASSERT(p.getEqualities(a.v)->top()==a.v);
        CPPUNIT_ASSERT(p.getEqualities(a.v)->getConstraints().at(b.v).constant==0);
        CPPUNIT_ASSERT(p.getEqualities(a.v)->getConstraints().at(b.v).firstCoef==2);
        CPPUNIT_ASSERT(p.getEqualities(a.v)->getConstraints().at(b.v).secondCoef==0);
        
        CPPUNIT_ASSERT(n.getVariableCreator().getDomain(a.v) == Domain(0,0));
        CPPUNIT_ASSERT(n.getVariableCreator().getDomain(b.v) == Domain(0,0));
        
    }
    
            // 7*a = 18*b
    
    void test9()
    {
        // 3*a = 2*b
        // 3*a = 4*b
        // 7*a = 18*b
        MySolver s;
        Normalizer n(s,lazySolveConfigProp4);
        EqualityProcessor p(s,n.getVariableCreator());

        View a = n.createView();
        View b = n.createView();

        std::vector<ReifiedLinearConstraint> lc;
        {
            LinearConstraint l(LinearConstraint::Relation::EQ);
            l.add(a*3);
            l.add(b*-2);
            l.addRhs(0);
            l.normalize();
            lc.emplace_back(ReifiedLinearConstraint(std::move(l),s.trueLit(),false));
        }
        
        {
            LinearConstraint l(LinearConstraint::Relation::EQ);
            l.add(a*3);
            l.add(b*-4);
            l.addRhs(0);
            l.normalize();
            lc.emplace_back(ReifiedLinearConstraint(std::move(l),s.trueLit(),false));
        }
        
        {
            LinearConstraint l(LinearConstraint::Relation::EQ);
            l.add(a*7);
            l.add(b*-18);
            l.addRhs(0);
            l.normalize();
            lc.emplace_back(ReifiedLinearConstraint(std::move(l),s.trueLit(),false));
        }
        
        CPPUNIT_ASSERT(p.process(lc));
        CPPUNIT_ASSERT(lc.size()==0);
        
        CPPUNIT_ASSERT(p.getEqualities(a.v)==p.getEqualities(b.v));
        CPPUNIT_ASSERT(p.getEqualities(a.v)->top()==a.v);
        CPPUNIT_ASSERT(p.getEqualities(a.v)->getConstraints().at(b.v).constant==0);
        CPPUNIT_ASSERT(p.getEqualities(a.v)->getConstraints().at(b.v).firstCoef==2);
        CPPUNIT_ASSERT(p.getEqualities(a.v)->getConstraints().at(b.v).secondCoef==0);
        
        CPPUNIT_ASSERT(n.getVariableCreator().getDomain(a.v) == Domain(0,0));
        CPPUNIT_ASSERT(n.getVariableCreator().getDomain(b.v) == Domain(0,0));
        
    }
    
    
    void test10()
    {
        // a = 2*b
        // b = 2*c
       
        // d = 2*e
        // e = 2*f
        
        ///merge both
        // c = 2*d
        
        
        
        /// a = 2b
        /// a = 4c
        /// a = 8d
        /// a = 16e
        /// a = 32f
        
        MySolver s;
        Normalizer n(s,lazySolveConfigProp4);
        EqualityProcessor p(s,n.getVariableCreator());

        View a = n.createView();
        View b = n.createView();
        View c = n.createView();
        View d = n.createView();
        View e = n.createView();
        View f = n.createView();

        std::vector<ReifiedLinearConstraint> lc;
        {
            LinearConstraint l(LinearConstraint::Relation::EQ);
            l.add(a);
            l.add(b*-2);
            l.addRhs(0);
            l.normalize();
            lc.emplace_back(ReifiedLinearConstraint(std::move(l),s.trueLit(),false));
        }
        
        {
            LinearConstraint l(LinearConstraint::Relation::EQ);
            l.add(b);
            l.add(c*-2);
            l.addRhs(0);
            l.normalize();
            lc.emplace_back(ReifiedLinearConstraint(std::move(l),s.trueLit(),false));
        }
        
        {
            LinearConstraint l(LinearConstraint::Relation::EQ);
            l.add(d);
            l.add(e*-2);
            l.addRhs(0);
            l.normalize();
            lc.emplace_back(ReifiedLinearConstraint(std::move(l),s.trueLit(),false));
        }
        {
            LinearConstraint l(LinearConstraint::Relation::EQ);
            l.add(e);
            l.add(f*-2);
            l.addRhs(0);
            l.normalize();
            lc.emplace_back(ReifiedLinearConstraint(std::move(l),s.trueLit(),false));
        }
        
        {
            LinearConstraint l(LinearConstraint::Relation::EQ);
            l.add(c);
            l.add(d*-2);
            l.addRhs(0);
            l.normalize();
            lc.emplace_back(ReifiedLinearConstraint(std::move(l),s.trueLit(),false));
        }
        
        
        /// further linear constraints
        
        View g = n.createView();
        {
        LinearConstraint l(LinearConstraint::Relation::LE);
        l.add(a);
        l.add(d*14); // this is 1.75 a's
        l.add(f*-3); /// is is only a fraction of a
        l.add(b); /// this is 0.5a
        l.add(g);
        l.addRhs(0);
        l.normalize();
        lc.emplace_back(ReifiedLinearConstraint(std::move(l),s.trueLit(),false));
        }
        
        CPPUNIT_ASSERT(p.process(lc));
        CPPUNIT_ASSERT(lc.size()==1);
        
        CPPUNIT_ASSERT(p.getEqualities(a.v)==p.getEqualities(b.v));
        CPPUNIT_ASSERT(p.getEqualities(b.v)==p.getEqualities(c.v));
        CPPUNIT_ASSERT(p.getEqualities(c.v)==p.getEqualities(d.v));
        CPPUNIT_ASSERT(p.getEqualities(d.v)==p.getEqualities(e.v));
        CPPUNIT_ASSERT(p.getEqualities(e.v)==p.getEqualities(f.v));
        CPPUNIT_ASSERT(p.getEqualities(a.v)->top()==a.v);
        CPPUNIT_ASSERT(p.getEqualities(a.v)->getConstraints().at(b.v).constant==0);
        CPPUNIT_ASSERT(p.getEqualities(a.v)->getConstraints().at(c.v).constant==0);
        CPPUNIT_ASSERT(p.getEqualities(a.v)->getConstraints().at(d.v).constant==0);
        CPPUNIT_ASSERT(p.getEqualities(a.v)->getConstraints().at(e.v).constant==0);
        CPPUNIT_ASSERT(p.getEqualities(a.v)->getConstraints().at(f.v).constant==0);
        
        CPPUNIT_ASSERT(p.getEqualities(a.v)->getConstraints().at(b.v).firstCoef==2);
        CPPUNIT_ASSERT(p.getEqualities(a.v)->getConstraints().at(b.v).secondCoef==1);
        
        CPPUNIT_ASSERT(p.getEqualities(a.v)->getConstraints().at(c.v).firstCoef==4);
        CPPUNIT_ASSERT(p.getEqualities(a.v)->getConstraints().at(c.v).secondCoef==1);
        
        CPPUNIT_ASSERT(p.getEqualities(a.v)->getConstraints().at(d.v).firstCoef==8);
        CPPUNIT_ASSERT(p.getEqualities(a.v)->getConstraints().at(d.v).secondCoef==1);
        
        CPPUNIT_ASSERT(p.getEqualities(a.v)->getConstraints().at(e.v).firstCoef==16);
        CPPUNIT_ASSERT(p.getEqualities(a.v)->getConstraints().at(e.v).secondCoef==1);
        
        CPPUNIT_ASSERT(p.getEqualities(a.v)->getConstraints().at(f.v).firstCoef==32);
        CPPUNIT_ASSERT(p.getEqualities(a.v)->getConstraints().at(f.v).secondCoef==1);
        
        for (auto& i : lc)
            p.substitute(i.l);
        
        const LinearConstraint& l = lc.back().l;
        CPPUNIT_ASSERT(l.getViews()[0].v==a);
        CPPUNIT_ASSERT(l.getViews()[0].a==101);
        CPPUNIT_ASSERT(l.getViews()[1].v==g);
        CPPUNIT_ASSERT(l.getViews()[1].a==32);
        CPPUNIT_ASSERT(l.getViews().size()==2);
        
        ReifiedAllDistinct rd({a*3,b+7,c,d,e,f,g},Literal(0,false),false);
        p.substitute(rd);
    }
};

CPPUNIT_TEST_SUITE_REGISTRATION (EqualityTest);




