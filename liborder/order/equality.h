// {{{ MIT License

// Copyright 2017 Max Ostrowski

// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.

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

    ///pre: top() < ec.top();
    /// l has something from both classes
    /// l has exactly 2 variables
    bool merge(const EqualityClass& ec, const LinearConstraint &l);
    /// simply remove constraint handling v
    /// has a constraint handling v
    /// v != top
    void remove(Variable v);

    const Constraints& getConstraints() const { return constraints_; }

private:

    std::unordered_map<Variable,Edge> constraints_; /// binary constraints, all containing the top_ variable
                                                    /// Var -> Edge(first,second,constant) represents equality first*Var = second*top_ + constant
                                                    /// there can only be one such edge, if there is more, the variable
                                                    /// has either -> no value
                                                    /// or exactly one value
    Variable top_; /// the variable all other elements are equal to
};

/// it is important that no domain is set yet
class EqualityProcessor
{
private:
    using EqualityClassSet = std::set<std::shared_ptr<EqualityClass>>;
public:
    using EqualityClassMap = std::unordered_map<Variable,std::shared_ptr<EqualityClass>>;
    EqualityProcessor(CreatingSolver& s, VariableCreator& vc) : s_(s), vc_(vc) {}

    const EqualityClassMap& equalities() const { return equalityClasses_; }

    bool process(std::vector<ReifiedLinearConstraint>& linearConstraints);

    EqualityClassSet getEqualityClasses(const LinearConstraint &l);

    /// replace all variables in l with one of the tops
    void replace(LinearConstraint& l);
    bool unary(const LinearConstraint& l);
    bool unary(Variable v, int32 value);
    bool merge(EqualityClassSet ecv, LinearConstraint& l);

    bool hasEquality(Variable v) const { return equalityClasses_.find(v) != equalityClasses_.end(); }
    bool isUnary(Variable v) const { return unary_.find(v) != unary_.end(); }
    bool isValid(Variable v) const { return (!hasEquality(v)) || getEqualities(v)->top()==v; }
    std::shared_ptr<EqualityClass> getEqualities(Variable v) const { assert(hasEquality(v)); return equalityClasses_.find(v)->second; }
    const std::unordered_map<Variable,int32> getUnaries() const { return unary_; }
    int32 getUnary(Variable v) const { return unary_.at(v); }

    bool substitute(LinearConstraint& l) const;
    bool substitute(ReifiedAllDistinct& l) const;
    bool substitute(ReifiedDomainConstraint& l) const;
    bool substitute(ReifiedDisjoint& l) const;
    bool substitute(View& v) const;

private:
    EqualityClassMap equalityClasses_;
    std::unordered_map<Variable,int32> unary_; // Variable = int32
    CreatingSolver& s_;
    VariableCreator& vc_;


};

}
