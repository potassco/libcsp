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

#pragma once
#include <order/types.h>
#include <order/constraint.h>

namespace order
{

class EqualityClass
{
public:
    struct Edge
    {
        Edge() = default;
        Edge(int32 firstCoef, int32 secondCoef, int32 constant) :
            firstCoef(firstCoef), secondCoef(secondCoef), constant(constant)
        {}
        int32 firstCoef;
        int32 secondCoef;
        int32 constant;
        Edge operator *(int32 x) { Edge e(*this); e.firstCoef*=x; e.secondCoef*=x; e.constant*=x; return e;}
        Edge& operator *=(int32 x) { firstCoef*=x; secondCoef*=x; constant*=x; return *this;}
    };
    using Constraints = std::unordered_map<Variable,Edge>;

    EqualityClass(Variable top) : top_(top) {}

    Variable top() const { return top_; }

    bool add(LinearConstraint& l, VariableCreator& vc);

    ///pre: top() < ec.top();
    /// l has something from both classes
    /// l has at least 2 variables
    bool merge(const EqualityClass& ec, LinearConstraint& l, VariableCreator& vc);

    const Constraints& getConstraints() const { return constraints_; }

private:

    std::unordered_map<Variable,Edge> constraints_; /// binary constraints, all containing the top_ variable
                                                    /// Var -> Edge(first,second,constant) represents equality first*Var = second*top_ + constant
                                                    /// there can only be one such edge, if there is more, the variable
                                                    /// has either -> no value
                                                    /// or exactly one value
    Variable top_; /// the variable all other elements are equal to
};

class EqualityProcessor
{
private:
    using EqualityClassSet = std::set<std::shared_ptr<EqualityClass>>;
public:
    using EqualityClassMap = std::unordered_map<Variable,std::shared_ptr<EqualityClass>>;
    EqualityProcessor(CreatingSolver& s, VariableCreator& vc) : s_(s), vc_(vc) {}

    const EqualityClassMap& equalities() const { return equalityClasses_; }

    bool process(std::vector<ReifiedLinearConstraint>& linearConstraints);

    EqualityClassSet getEqualityClasses(const LinearConstraint& l);

    bool merge(EqualityClassSet& ecv, LinearConstraint& l);

    bool hasEquality(Variable v) const { return equalityClasses_.find(v) != equalityClasses_.end(); }
    bool isValid(Variable v) const { return (!hasEquality(v)) || getEqualities(v)->top()==v; }
    std::shared_ptr<EqualityClass> getEqualities(Variable v) const { assert(hasEquality(v)); return equalityClasses_.find(v)->second; }

    bool substitute(LinearConstraint& l) const;
    bool substitute(ReifiedAllDistinct& l) const;
    bool substitute(ReifiedDomainConstraint& l) const;
    bool substitute(ReifiedDisjoint& l) const;
    bool substitute(View& v) const;

private:
    EqualityClassMap equalityClasses_;
    CreatingSolver& s_;
    VariableCreator& vc_;


};

}
