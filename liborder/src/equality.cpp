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

    void EqualityClass::remove(Variable v)
    {
        assert(constraints_.find(v)!=constraints_.end());
        assert(top_ != v);
        constraints_.erase(v);
    }

    /// pre: l is unary equality
    /// sets the value for l and removes all equality classes containing this variable
    /// recursively set value for equal variables
    bool EqualityProcessor::unary(const LinearConstraint& l)
    {
        assert(l.normalized());
        assert(l.getConstViews().size()==1);
        View v = l.getConstViews().front();
        if (((int64)(l.getRhs())-(int64)(v.c))%v.a != 0)
            return false;
        int32 value = v.divide(l.getRhs());
        return unary(v.v,value);
    }

    /// pre: l is unary equality
    /// sets the value for l and removes all equality classes containing this variable
    /// recursively set value for equal variables
    bool EqualityProcessor::unary(Variable v, int32 value)
    {
        assert (unary_.find(v)==unary_.end()); // if it woudl already exist, it would have been already replaced
        unary_[v] = value;
        //std::cout << "Set v" << v << " to " << value << std::endl;

        if (hasEquality(v))
        {
            auto ec = getEqualities(v);
            if (ec->top() == v)
            {
                // we are top ourselfs, so add values for all other variables
                auto constraints = ec->getConstraints();
                for (auto i : constraints)
                {
                    Variable var = i.first;
                    assert(unary_.find(var)==unary_.end()); // i should not have a value AND an equality
                    EqualityClass::Edge e = i.second;

                    /// e.first*var = e.second*ec->top() +  e.constant
                    if ( ((int64(e.secondCoef) * int64(value)) + int64(e.constant)) % e.firstCoef != 0)
                        return false;
                    unary_[var] = ((int64(e.secondCoef) * int64(value)) + int64(e.constant)) / e.firstCoef;
                    //std::cout << "Set v" << var << " to " << unary_[var] << std::endl;
                }
                equalityClasses_.erase(ec->top());
            }
            else
            {
                auto constraint = ec->getConstraints().find(v);
                assert(constraint!= ec->getConstraints().end());
                EqualityClass::Edge e = constraint->second;
                /// e.first*v.v = e.second*ec->top() + e.constant
                if ( ((int64(e.firstCoef)*int64(value) - int64(e.constant)) % e.secondCoef)  != 0)
                    return false;
                int32 newval = ((int64(e.firstCoef)*int64(value) - int64(e.constant)) / e.secondCoef);
                ec->remove(v);
                return unary(ec->top(),newval);
            }
        }
        return true;
    }

    /// replaces all occurences in l with one of the two tops
    void EqualityProcessor::replace(LinearConstraint& l)
    {
        auto& views = l.getViews();

        /// convert all variables to one of the two tops
        for (auto it = views.begin(); it != views.end(); ++it)
        {
            auto ass = unary_.find(it->v);
            if (ass != unary_.end())
            {
                /// if we have an assignment for this variable, replace the value and shift it to the right hand side
                l.addRhs(-((ass->second*it->a)+it->c));
                it->c=0;
                it->a=0;
                continue;
            }
            if (hasEquality(it->v))
            {
                auto ec = getEqualities(it->v);
                if (it->v==ec->top())
                    continue;
                auto constraint = ec->getConstraints().find(it->v);
                assert(constraint != ec->getConstraints().end()); /// should always exist has we have hasEquality
                /// convert to top
                int32 myfactor = it->a;
                EqualityClass::Edge e = constraint->second;
                int32 oldfactor = e.firstCoef;
                int32 g = gcd(oldfactor,myfactor);
                e *= (myfactor/g);
                l.times(oldfactor/g);
                it->v = ec->top();
                it->a = e.secondCoef;
                l.addRhs(-e.constant);

            }
        }
        l.normalize();
    }

    ///pre: top() < ec.top();
    /// l has only these two top variables
    /// l has exactly 2 variables
    bool EqualityClass::merge(const EqualityClass& ec, const LinearConstraint& l)
    {
        assert(top() < ec.top());
        assert(l.getConstViews().size()==2);
        assert(l.normalized());
        ///im top=A, ec is top=B,
        /// convert l, s.t. l:= xA = yB + c
        auto& views = l.getConstViews();
        assert(views.size()==2);

        /// convert all variables to one of the two tops
        for (auto it = views.begin(); it != views.end(); ++it)
        {
            assert (it->v==top_ || it->v==ec.top_);
        }
        //l.normalize();
        //assert(l.getConstViews().size()==2);
        //views = l.getViews();
        View myviews[2];
        if (views.front().v == top_)
        {
            myviews[0] = views.front();
            myviews[1] = views.back();
        }
        else
        {
            myviews[1] = views.front();
            myviews[0] = views.back();
        }


        assert(myviews[0].v==top_);
        assert(myviews[1].v==ec.top_);

        /// also add the relation in the constraint to our equivalence class
        //std::cout << "added equality " << -myviews[1].a << " * v" << myviews[1].v << " = " <<
        //              myviews[0].a << " * v" << top_ << " + " << -l.getRhs() << std::endl;
        constraints_[views.back().v] = Edge(-myviews[1].a,myviews[0].a,-l.getRhs());

        int32 myfactor = myviews[1].a * -1;
        /// add all constraints from ec and convert the top variable
        for (auto e : ec.constraints_)
        {
            Edge convert(myviews[0].a, myfactor, l.getRhs());
            int oldfactor = e.second.secondCoef;
            int32 g = gcd(oldfactor,myfactor);
            convert *= (oldfactor/g);
            Edge newConstraint = e.second * (myfactor/g);
            newConstraint.secondCoef = convert.firstCoef;
            newConstraint.constant -= convert.constant;
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

            replace(l);
            /*if (l.size()==2)
            {
                auto first = (*ecv.begin());
                auto second = *(++ecv.begin());
                if (first->top() > second->top())
                    std::swap(first,second);
                first->replace(*second,l,vc_);
                ecv = getEqualityClasses(l);
            }*/

            auto size =  l.getConstViews().size();
            if (size<=2)
            {
                if (size==2)
                {
                    if (!merge(getEqualityClasses(l),l))
                        return false;
                }
                else if (size==1)
                {
                    if (!unary(l))
                        return false;
                }
                else if (size==0)
                {
                    if (l.getRhs()!=0)
                        return false;
                }
                /// remove this constraint and reiterate
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
            if (it==equalityClasses_.end())
            {
                auto found = unary_.find(i.v);
                if (found != unary_.end())
                {
                    l.addRhs(-(i.a*found->second)-i.c);
                    i.a=0;
                    i.c=0;
                }
            }
            else if (it->second->top()!=i.v)
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
            if (it==equalityClasses_.end())
            {
                auto found = unary_.find(i.v);
                if (found != unary_.end())
                {
                    i.c+=i.a*found->second;
                    i.a=0;
                }
            }
            else if (it->second->top()!=i.v)
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
        if (it==equalityClasses_.end())
        {
            auto found = unary_.find(i.v);
            if (found != unary_.end())
            {
                i.c+=i.a*found->second;
                i.a=0;
            }
        }
        else if (it->second->top()!=i.v)
        {
            EqualityClass::Edge e = it->second->getConstraints().find(i.v)->second;
            /// i.v * e.firstCoef = it->second->top() * e.secondCoef + e.constant
            int32 old = i.a;
            int32 g = gcd(old,e.firstCoef);
            i *= (e.firstCoef/g);
            i.v = it->second->top();
            i.a = (old/g)*e.secondCoef;
            i.c += (old/g)*e.constant;
            l.getDomain().inplace_times(e.firstCoef/g,Domain::max-Domain::min);
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
                if (it==equalityClasses_.end())
                {
                    auto found = unary_.find(i.v);
                    if (found != unary_.end())
                    {
                        i.c+=i.a*found->second;
                        i.a=0;
                    }
                }
                else if (it->second->top()!=i.v)
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
        if (it==equalityClasses_.end())
        {
            auto found = unary_.find(v.v);
            if (found != unary_.end())
            {
                v.c+=v.a*found->second;
                v.a=0;
            }
        }
        else if (it->second->top()!=v.v)
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

    bool EqualityProcessor::merge(EqualityProcessor::EqualityClassSet ecv, LinearConstraint& l)
    {
        assert(ecv.size()==2);

            auto first = (*ecv.begin());
            auto second = *(++ecv.begin());
            if (first->top() > second->top())
                std::swap(first,second);

            if (!first->merge(*second,l))
                return false;

            Variable top = second->top();
            for (EqualityClass::Constraints::const_iterator i = second->getConstraints().begin(); i != second->getConstraints().end(); ++i)
                equalityClasses_[i->first] = first;
            equalityClasses_[top] = first;
            return true;
    }

}
