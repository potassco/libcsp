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

#include <order/equality.h>
#include <order/helper.h>


namespace order
{


    bool EqualityClass::add(LinearConstraint& l, VariableCreator& vc)
    {
        assert(l.normalized());
        for (auto& i : l.getConstViews())
        {
            (void)(i);
            assert(constraints_.find(i.v)!=constraints_.end() || i.v==top_);
        }
        //for each variable != top, replace it in the linear constraint with top
        //.... this can result in returning false, if we have a = b, a = b + 1 (a=b,a=2b works for a=0 :) )
        for (auto& i : l.getViews())
            if (i.v != top_)
            {
                int32 myfactor = i.a;
                auto it = constraints_.find(i.v);
                assert(it != constraints_.end());
                int32 oldfactor = it->second.firstCoef;
                int32 g = gcd(oldfactor,myfactor);
                Edge e = it->second * (myfactor/g);
                l.times(oldfactor/g);
                assert(e.firstCoef==i.a);
                /// now replace variable in l with top
                i.v = top_;
                i.a = e.secondCoef;
                l.addRhs(-e.constant);
                //l.normalize();/// destroys the views iterator, factorize is not ok, as some coefficients could be 0
            }
        l.normalize();
        assert(l.getConstViews().size()<=1);
        if (l.getConstViews().size()==0)
            return l.getRhs()==0;
        assert(l.getConstViews().size()==1);
        /// variable has either 1 or 0 values
        /// degenerate equality class
        if (!vc.constrainView(l.getViews().front(),l.getRhs(),l.getRhs()))
            return false;
        /// top variable has exactly one value
        assert(vc.getDomainSize(top_)==1);
        int32 val = *(vc.getRestrictor(View(top_)).begin());
        /// all other variables have also at most one value
        for (auto e = constraints_.begin(); e != constraints_.end(); ++e)
        {
            int64 res = e->second.secondCoef*val + e->second.constant;
            if (!vc.constrainView(View(e->first,e->second.firstCoef), res,res))
                return false;
            e->second.secondCoef=0;
            e->second.constant=res;
        }
        return true;
    }
    
    ///pre: top() < ec.top();
    /// l has something from both classes
    /// l has at least 2 variables
    bool EqualityClass::merge(const EqualityClass& ec, LinearConstraint& l, VariableCreator& )
    {
        assert(top() < ec.top());
        assert(l.getConstViews().size()>=2);
        ///im top=A, ec is top=B,
        /// convert l, s.t. l:= xA = yB + c
        auto& views = l.getViews();

        /// convert all variables to one of the two tops
        for (auto it = views.begin(); it != views.end(); ++it)
        {
            if (it->v==top_ || it->v==ec.top_)
                continue;

            std::unordered_map<Variable,Edge>::const_iterator edgeit = constraints_.find(it->v);
            if (edgeit != constraints_.end())
            {
                /// convert to top
                int32 myfactor = it->a;
                Edge e = edgeit->second;
                int32 oldfactor = e.firstCoef;
                int32 g = gcd(oldfactor,myfactor);
                e *= (myfactor/g);
                l.times(oldfactor/g);
                it->v = top_;
                it->a = e.secondCoef;
                l.addRhs(-e.constant);
                //l.normalize();/// destroys the views iterator, factorize is not ok, as some coefficients could be 0
            }
            else
            {
                edgeit = ec.constraints_.find(it->v);
                assert(edgeit != ec.constraints_.end());
                /// convert to ec.top
                int32 myfactor = it->a;
                Edge e = edgeit->second;
                int32 oldfactor = e.firstCoef;
                int32 g = gcd(oldfactor,myfactor);
                e *= (myfactor/g);
                l.times(oldfactor/g);
                it->v = ec.top_;
                it->a = e.secondCoef;
                l.addRhs(-e.constant);
                //l.normalize();
            }
        }
        l.normalize();
        assert(l.getViews().size()==2);
        views = l.getViews();


        if (views.front().v != top_)
            std::swap(views.front(),views.back());

        assert(views.front().v==top_);
        assert(views.back().v==ec.top_);

        /// also add the relation in the constraint to our equivalence class
        //std::cout << "added equality " << -views.back().a << " * v" << views.back().v << " = " <<
        //              views.front().a << " * v" << top_ << " + " << -l.getRhs() << std::endl;
        constraints_[views.back().v] = Edge(-views.back().a,views.front().a,-l.getRhs());

        int32 myfactor = views.back().a * -1;
        /// add all constraints from ec and convert the top variable
        for (auto e : ec.constraints_)
        {
            Edge convert(views.front().a, myfactor, l.getRhs());
            int oldfactor = e.second.secondCoef;
            int32 g = gcd(oldfactor,myfactor);
            convert *= (oldfactor/g);
            Edge newConstraint = e.second * (myfactor/g);
            newConstraint.secondCoef = convert.firstCoef;
            newConstraint.constant += convert.constant;
            assert(constraints_.find(e.first)==constraints_.end());
            constraints_[e.first] = newConstraint;
        }

        return true;
    }

    

    bool EqualityProcessor::process(std::vector<ReifiedLinearConstraint>& linearConstraints)
    {
        std::vector<LinearConstraint> equals;
        for (unsigned int i = 0; i < linearConstraints.size();)
        {
            auto& rc = linearConstraints[i];
            if ((s_.isTrue(rc.v) && rc.l.getRelation()==LinearConstraint::Relation::EQ && (rc.impl & Direction::FWD)) || (s_.isFalse(rc.v) && rc.l.getRelation()==LinearConstraint::Relation::NE && (rc.impl & Direction::BACK)))
            {
                rc.l.setRelation(LinearConstraint::Relation::EQ);
                equals.emplace_back(std::move(rc.l));
                std::swap(linearConstraints[i],linearConstraints.back());
                linearConstraints.pop_back();
            }
            else
                ++i;
        }
        

        unsigned int current = 0;
        unsigned int last = 0;
        if (current < equals.size())
        while (true)
        {
            LinearConstraint& l = equals[current];
            l.normalize();
            EqualityClassSet ecv = getEqualityClasses(l);
            if (ecv.size()<=2)
            {
                if (!merge(ecv,l))
                    return false;
                std::swap(equals[current],equals.back());
                equals.pop_back(); 
                last = current;
                if (last == equals.size())
                    last = 0;
                if (current == equals.size())
                    current = 0;
                if (current >= equals.size())
                    break;
            }
            else
            {
                ++current;
                if (current == equals.size())
                    current = 0;
                if (current==last)
                    break;
            }


        }    
        
        for (auto& i : equals)
            linearConstraints.emplace_back(std::move(i),s_.trueLit(),Direction::FWD);
                
        return true;    
    }
    
    bool EqualityProcessor::substitute(LinearConstraint& l) const
    {
        for (auto& i : l.getViews())
        {
            auto it = equalityClasses_.find(i.v);
            if (it != equalityClasses_.end() && it->second->top()!=i.v)
            {
                EqualityClass::Edge e = it->second->getConstraints().find(i.v)->second;
                /// i.v * e.firstCoef = it->second->top() * e.secondCoef + e.constant
                int32 old = i.a;
                int32 g = gcd(old,e.firstCoef);
                l.times(e.firstCoef/g);
                i.v = it->second->top();
                i.a = (old/g)*e.secondCoef;
                i.c += (old/g)*e.constant;
            }
        }
        l.normalize();
        return true;
    }
    
    bool EqualityProcessor::substitute(ReifiedAllDistinct& l) const
    {
        for (auto& i : l.getViews())
        {
            auto it = equalityClasses_.find(i.v);
            if (it != equalityClasses_.end() && it->second->top()!=i.v)
            {
                EqualityClass::Edge e = it->second->getConstraints().find(i.v)->second;
                /// i.v * e.firstCoef = it->second->top() * e.secondCoef + e.constant
                int32 old = i.a;
                int32 g = gcd(old,e.firstCoef);
                l.times(e.firstCoef/g);
                i.v = it->second->top();
                i.a = (old/g)*e.secondCoef;
                i.c += (old/g)*e.constant;                
            }
        }
        return true;
    }
    
    bool EqualityProcessor::substitute(ReifiedDomainConstraint& l) const
    {
        auto& i = l.getView();
        auto it = equalityClasses_.find(i.v);
        if (it != equalityClasses_.end() && it->second->top()!=i.v)
        {
            EqualityClass::Edge e = it->second->getConstraints().find(i.v)->second;
            /// i.v * e.firstCoef = it->second->top() * e.secondCoef + e.constant
            int32 old = i.a;
            int32 g = gcd(old,e.firstCoef);
            i *= (e.firstCoef/g);
            i.v = it->second->top();
            i.a = (old/g)*e.secondCoef;
            i.c += (old/g)*e.constant;
            l.getDomain().times(e.firstCoef/g,Domain::max-Domain::min);
        }
        return true;
    }
    
    bool EqualityProcessor::substitute(ReifiedDisjoint& l) const
    {
        for (auto& j : l.getViews())
        {
            for (auto& k : j)
            {
                auto& i = k.first;
                auto it = equalityClasses_.find(i.v);
                if (it != equalityClasses_.end() && it->second->top()!=i.v)
                {
                    EqualityClass::Edge e = it->second->getConstraints().find(i.v)->second;
                    /// i.v * e.firstCoef = it->second->top() * e.secondCoef + e.constant
                    int32 old = i.a;
                    int32 g = gcd(old,e.firstCoef);
                    l.times(e.firstCoef/g);
                    i.v = it->second->top();
                    i.a = (old/g)*e.secondCoef;
                    i.c += (old/g)*e.constant;                
                }
            }
        }
        return true;
    }

    bool EqualityProcessor::substitute(View& v) const
    {
        auto it = equalityClasses_.find(v.v);
        if (it != equalityClasses_.end() && it->second->top()!=v.v)
        {
            EqualityClass::Edge e = it->second->getConstraints().find(v.v)->second;
            /// i.v * e.firstCoef = it->second->top() * e.secondCoef + e.constant
            int32 old = v.a;
            int32 g = gcd(old,e.firstCoef);
            v.v = it->second->top();
            v.a = (old/g)*e.secondCoef;
            v.c += (old/g)*e.constant;
        }
        return true;
    }
    
    EqualityProcessor::EqualityClassSet EqualityProcessor::getEqualityClasses(const LinearConstraint& l)
    {
        EqualityClassSet ecv;
        for (auto& i : l.getViews())
        {
            auto it = equalityClasses_.find(i.v);
            if (it != equalityClasses_.end())
            {
                ecv.emplace(it->second);
            }
            else
            {
                std::shared_ptr<EqualityClass> sp(new EqualityClass(i.v));
                equalityClasses_[i.v]=sp;
                ecv.emplace(sp);
            }
        }
        return ecv;        
    }
    
    bool EqualityProcessor::merge(EqualityProcessor::EqualityClassSet& ecv, LinearConstraint& l)
    {
        assert(ecv.size()<=2);
        if (ecv.size()==2)
        {
            auto first = (*ecv.begin());
            auto second = *(++ecv.begin());
            if (first->top() > second->top())
                std::swap(first,second);
            Variable top = second->top();
            if (!first->merge(*second,l,vc_))
                return false;
            for (EqualityClass::Constraints::const_iterator i = second->getConstraints().begin(); i != second->getConstraints().end(); ++i)
                equalityClasses_[i->first] = first;
            equalityClasses_[top] = first;
            return true;
        }
        else
        if (ecv.size()==1)
        {
            auto first = (*ecv.begin());
            return first->add(l,vc_);
        }
        /// special case, no variables
        return l.getRhs()==0;
    }

}
