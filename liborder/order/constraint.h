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
#include <order/variable.h>
#include <order/storage.h>
#include <order/solver.h>
#include <order/types.h>
#include <order/config.h>

#include <vector>
#include <algorithm>

namespace order
{

enum class TruthValue {TRUE, FALSE, UNKNOWN};
struct ReifiedLinearConstraint;

class LinearConstraint
{
public:
    friend ReifiedLinearConstraint;

    enum class Relation : short {LT, LE, GT, GE, EQ, NE};
    LinearConstraint(Relation r) : constant_(0) , r_(r), flag_(false), normalized_(false) {}
    //LinearConstraint(const LinearConstraint& o) : vars_(o.vars_), constant_(o.constant_), r_(o.r_), flag_(o.flag_) {}
    //LinearConstraint(LinearConstraint&& o) : vars_(std::move(o.vars_)), constant_(o.constant_), r_(o.r_), flag_(o.flag_) {}
    bool operator==(const LinearConstraint& r) const { return r_==r.r_ && constant_==r.constant_ && views_==r.views_; }
    bool operator<(const LinearConstraint& r) const { return (int)r_<(int)r.r_ && constant_<r.constant_ && views_<r.views_; }

    Relation getRelation() const { return r_; }
    void setRelation(Relation r) { r_ = r; }
    std::vector<View>& getViews() { normalized_ = false; return views_; }
    const std::vector<View>& getViews() const { return views_; }
    const std::vector<View>& getConstViews() const { return views_; }
    void add(const View& v) { views_.emplace_back(v); normalized_ = false; }
    void addRhs(int constant) { constant_+=constant; normalized_ = false; }
    void times(int32 x);
    int getRhs() const { return constant_; }

    bool getFlag() const { return flag_; }
    void setFlag(bool b) { flag_ = b; }
    bool normalized() const {return normalized_; }


    friend std::ostream& operator<< (std::ostream& stream, const LinearConstraint& d);


    /// merge double variables, remove 0 coeffs
    /// performs and returns gcd
    int normalize();

    /// reverse the relation of the constraint
    void reverse();
    /// * -1 on both sides, inverts the relation is <,<=,>,=>
    void invert();

    /// sort the constraint according to |coefficient|, domainsize(var), biggest element first
    void sort(const VariableCreator& vc, const Config& conf)
    {
        assert(normalized_);
        std::sort(views_.begin(), views_.end(), [&vc,&conf](const View& x, const View& y)
        {
            if (conf.coefFirst)
            {
                if (abs(x.a) != abs(y.a))
                        return (conf.descendCoef ? abs(x.a) > abs(y.a) : abs(x.a) < abs(y.a));
                uint32 a = vc.getDomainSize(x);
                uint32 b = vc.getDomainSize(y);

                return conf.descendDom ? a > b : a < b;
            }
            else
            {
                uint32 a = vc.getDomainSize(x);
                uint32 b = vc.getDomainSize(y);
                if (a != b)
                    return conf.descendDom ? a > b : a < b;
                return (conf.descendCoef ? abs(x.a) > abs(y.a) : abs(x.a) < abs(y.a));
            }
         });
    }


    ///pre: constraint is not empty
    ///sum of (currentDomain * koeff)
    Domain lhsDomain(const CreatingSolver &s, const VariableCreator& vc, const Config& conf);


    ///pre: constraint is not empty
    /// product of size of all domains but last variable is less or equal x
    /// (x<0 is infinity)
    bool productOfDomainsExceptLastLEx(const VariableCreator& vc, int64 x) const;
    uint64 productOfDomainsExceptLast(const VariableCreator& vc) const;


    /// perform recursive sugarlike splitting
    /// pre: should be normalized
    /// the first constraint is the original one (with the original truthvalue)
    /// all others are simply true
    std::vector<LinearConstraint> split(const CreatingSolver &s, VariableCreator &vc, const Config &conf, TruthValue t) const;

    /// divide constraint by the gcd and return it
    /// pre: all views must have c==0
    int factorize();

private:
    


    std::vector<View> views_; /// should only contain views with b==1 and c==0
    int constant_; // rhs
    Relation r_;
    bool flag_;
    bool normalized_;
};


inline std::ostream& operator<< (std::ostream& stream, const LinearConstraint& l)
{
    assert(l.normalized_);
    for (auto i = l.views_.begin(); i < l.views_.end()-1; ++i)
    {
        stream << "v" << i->v << " * " << i->a  << "\t+\t";
    }
    stream << "v" << (l.views_.end()-1)->v << " * " << (l.views_.end()-1)->a << "\t";
    switch(l.r_)
    {
    case LinearConstraint::Relation::EQ: stream << "== "; break;
    case LinearConstraint::Relation::NE: stream << "!= "; break;
    case LinearConstraint::Relation::LT: stream << "< "; break;
    case LinearConstraint::Relation::LE: stream << "<= "; break;
    case LinearConstraint::Relation::GT: stream << "> "; break;
    case LinearConstraint::Relation::GE: stream << ">= "; break;
    default: assert(true);
    }
    stream << l.constant_;

    return stream;
}


struct ReifiedLinearConstraint
{
    ReifiedLinearConstraint(LinearConstraint&& ll, const Literal& vv, bool impl) : l(ll), v(vv), impl(impl) {}
    ReifiedLinearConstraint(const ReifiedLinearConstraint& ) = default;

    /// sort without impl
    static bool compareless(const ReifiedLinearConstraint& l, const ReifiedLinearConstraint& r) { return std::tie(l.v,l.l) < std::tie(r.v,r.l);/*l.v<r.v && l.l<r.l && l.impl < r.impl;*/ }
    /// ignore impl here
    static bool compareequal(const ReifiedLinearConstraint& l, const ReifiedLinearConstraint& r) { return l.v==r.v && l.l==r.l; }
    //bool operator==(const ReifiedLinearConstraint& r) const { return v==r.v && l==r.l; }
    //bool operator<(const ReifiedLinearConstraint& r) const { return /*std::tie(l,v) < std::tie(r.l,r.v);*/ v<r.v && l<r.l; }
    void sort(const VariableCreator& vc, const Config& c) { l.sort(vc,c); }
    LinearConstraint l;
    Literal v;
    bool impl;
    /// returns a list of reified constraints, the first one is the one that represents the original constraints
    /// the others describe the helper variables
    std::vector<ReifiedLinearConstraint> split(VariableCreator& vc, CreatingSolver &s, const Config &conf) const;
    /// reverse the literal
    void reverse() { l.reverse(); }
    /// this can change the literal as well as the constraint
    void normalize();
};


class ReifiedAllDistinct
{
public:
    ///TODO: sort variables, detect subset relations, reuse intermediate variables etc...
    ReifiedAllDistinct(std::vector<View>&& views, const Literal& l, bool impl) : views_(std::move(views)), v_(l), impl_(impl)
    {
        std::sort(views_.begin(), views_.end());
        views_.erase(std::unique(views_.begin(), views_.end()), views_.end());
    }
    bool isImpl() const { return impl_; }
    void add(const Variable& v) { views_.emplace_back(v); }
    const std::vector<View>& getViews() const { return views_; }
    std::vector<View>& getViews() { return views_; }
    void times(int32 x) /// multiply all views by x
    {
        for (auto& i : views_)
            i *= x;
    }

    Literal getLiteral() const { return v_; }
    void setLiteral(const Literal& l) { v_=l; }
private:

    std::vector<View> views_;
    Literal v_;
    bool impl_;
};


/// if the literal l is true, the view v has one value of the domain d
/// if false, it is unequal to the values in d
///
class ReifiedDomainConstraint
{
public:
    ReifiedDomainConstraint(View v, Domain&& d, const Literal& l, bool impl) : v_(v), d_(std::move(d)), l_(l), impl_(impl) {}
    ReifiedDomainConstraint(const ReifiedDomainConstraint& c) = default;
    ReifiedDomainConstraint(ReifiedDomainConstraint&& m) = default;
    ReifiedDomainConstraint& operator=(ReifiedDomainConstraint&& m) = default;
    ReifiedDomainConstraint& operator=(const ReifiedDomainConstraint& a) = default;
    bool isImpl() const { return impl_; }
    View getView() const { return v_; }
    View& getView() { return v_; }
    Literal getLiteral() const { return l_; }
    void setLiteral(const Literal& l) { l_=l; }
    const Domain& getDomain() const { return d_; }
    Domain& getDomain() { return d_; }
private:

    View v_;
    Domain d_;
    Literal l_;
    bool impl_;
};



class ReifiedDNF
{
public:
    /// input: a disjunction of conjunctions of literals
    ReifiedDNF(std::vector<std::vector<Literal>>&& dnf) : dnf_(std::move(dnf))
    {}

    /// overestimte the number of new variables needed
    uint64 estimateVariables() const;
    /// do introduce necessary variables for the reification and return reification literal l
    Literal tseitin(CreatingSolver &s) const;
private:

    std::vector<std::vector<Literal>> dnf_;
};



class ReifiedDisjoint
{
public:
    /// input: a set of viewlists
    /// the results of the variables of the first set are disjoint from the one from the other sets etc...
    /// a variable is only in the set if its condition is true
    ReifiedDisjoint(std::vector<std::vector<std::pair<View,ReifiedDNF>>>&& views, const Literal& l, bool impl) : views_(std::move(views)), v_(l), impl_(impl)
    {
    }
    bool isImpl() const { return impl_; }
    const std::vector<std::vector<std::pair<View,ReifiedDNF>>>& getViews() const {return views_; }
    std::vector<std::vector<std::pair<View,ReifiedDNF>>>& getViews() {return views_; }
    void times(int32 x)
    {
        for (auto& i : views_)
            for (auto& j : i)
                j.first *=x;
    }

    Literal getLiteral() const { return v_; }
    void setLiteral(const Literal& l) { v_=l; }
private:
    std::vector<std::vector<std::pair<View,ReifiedDNF>>> views_;
    Literal v_;
    bool impl_;
};


class ReifiedNormalizedDisjoint
{
public:
    /// input: a reified disjoint constraint
    /// does automatically convert to normalized form using tseitin
    ReifiedNormalizedDisjoint(ReifiedDisjoint&& rd, CreatingSolver& s) : v_(rd.getLiteral())
    {
        for (auto &i : rd.getViews())
        {
            views_.emplace_back(std::vector<std::pair<View,Literal>>());
            for (auto &j : i)
                views_.back().emplace_back(std::make_pair(j.first,j.second.tseitin(s)));
        }
    }

    const std::vector<std::vector<std::pair<View,Literal>>>& getViews() const { return views_; }

    Literal getLiteral() const { return v_; }
    void setLiteral(const Literal& l) { v_=l; }
private:
    std::vector<std::vector<std::pair<View,Literal>>> views_;
    Literal v_;
};






}
