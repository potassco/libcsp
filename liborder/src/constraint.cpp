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

#include <order/constraint.h>
#include <order/types.h>
#include <order/helper.h>

namespace order
{


void LinearConstraint::times(int32 x)
{
    for (auto& i : views_)
        i*=x;
    constant_*=x;
    if (x<0)
        invert();
}

Domain LinearConstraint::lhsDomain(const CreatingSolver& s, const VariableCreator& vc, const Config& conf)
{
    assert(views_.size()!=0);
    assert(normalized_);
    /// explicitly do not use getDomain of the view,
    /// as we are ok with an overapproximation
    /// and getDomain(view) gives accurate domain which can be expensive
    //Domain newDom(vc.getDomain(*views_.begin()));
    Domain newDom(Domain(vc.getDomain(views_.begin()->v)).times(views_.begin()->a, conf.domSize));
    if (newDom.overflow())
    {
        s.intermediateVariableOutOfRange();
        return newDom;
    }
    for(auto j = views_.begin()+1; j != views_.end(); ++j)
    {
        newDom+= Domain(vc.getDomain(j->v)).times(j->a, conf.domSize);
        if (newDom.overflow())
        {
            s.intermediateVariableOutOfRange();
            return newDom;
        }
    }
    return newDom;
}


bool LinearConstraint::productOfDomainsExceptLastLEx(const VariableCreator& vc, int64 x) const
{
    assert(views_.size()!=0);
    assert(normalized_);
    if (x<0)
        return true;
    uint64 ret = 1;
    for (std::size_t i = 0; i < views_.size()-1; ++i)
    {
        ret *= vc.getDomainSize(views_[i]);
        if (ret>uint64(x)) return false;
    }
    return true;
}

uint64 LinearConstraint::productOfDomainsExceptLast(const VariableCreator& vc) const
{
    assert(views_.size()!=0);
    assert(normalized_);
    uint64 ret = 1;
    for (std::size_t i = 0; i < views_.size()-1; ++i)
    {
        ret *= vc.getDomainSize(views_[i]);
    }
    return ret;
}

int LinearConstraint::normalize()
{
    if (normalized_) return 1;
    normalized_ = true;

    for (auto& i : views_)
    {
        constant_-=i.c;
        i.c=0;
    }

    switch(getRelation())
    {
        case LinearConstraint::Relation::LT: setRelation(LinearConstraint::Relation::LE); --constant_; break;
        case LinearConstraint::Relation::LE: break;
        case LinearConstraint::Relation::GT: ++constant_;
        case LinearConstraint::Relation::GE:
        {
            for (auto &i : views_)
                i.a *=-1;
            constant_*=-1;
            setRelation(LinearConstraint::Relation::LE);
            break;
        }
        case LinearConstraint::Relation::EQ:
        case LinearConstraint::Relation::NE:break;
        default: { assert(false); }
    }

    if (views_.size()==0) return 1;
    std::sort(views_.begin(), views_.end(), [](const View& x, const View& y) { return x.v < y.v; } );

    /// had a look at reference implementation of unique
    auto result = views_.begin();
    auto first = result;
    while (++first != views_.end())
    {
        if (result->a==0)
            *result=*first;
        else
            if (result->v != first->v)
                *(++result)=*first;
            else
                result->a+=first->a;
    }
    if (result->a==0)
        views_.erase(result,views_.end());
    else
        views_.erase(++result,views_.end());
    return factorize();
}

void LinearConstraint::reverse()
{
    switch(r_)
    {
    case LinearConstraint::Relation::EQ: r_ = LinearConstraint::Relation::NE; break;
    case LinearConstraint::Relation::NE: r_ = LinearConstraint::Relation::EQ; break;
    case LinearConstraint::Relation::LT: r_ = LinearConstraint::Relation::GE; break;
    case LinearConstraint::Relation::LE: r_ = LinearConstraint::Relation::GT; break;
    case LinearConstraint::Relation::GT: r_ = LinearConstraint::Relation::LE; break;
    case LinearConstraint::Relation::GE: r_ = LinearConstraint::Relation::LT; break;
    }
}


void LinearConstraint::invert()
{
    switch(r_)
    {
    case LinearConstraint::Relation::EQ: r_ = LinearConstraint::Relation::EQ; break;
    case LinearConstraint::Relation::NE: r_ = LinearConstraint::Relation::NE; break;
    case LinearConstraint::Relation::LT: r_ = LinearConstraint::Relation::GT; break;
    case LinearConstraint::Relation::LE: r_ = LinearConstraint::Relation::GE; break;
    case LinearConstraint::Relation::GT: r_ = LinearConstraint::Relation::LT; break;
    case LinearConstraint::Relation::GE: r_ = LinearConstraint::Relation::LE; break;
    }
}


/// BE AWARE, if you shange this you should adopt the estimate function for the alldifferent translation in the normalizer.cpp
std::vector<LinearConstraint> LinearConstraint::split(const CreatingSolver& s, VariableCreator& vc, const Config& conf, TruthValue t) const
{
    std::vector<LinearConstraint> ret;
    LinearConstraint l(*this);
    l.sort(vc,conf);
    if (conf.splitsize_maxClauseSize.first < 0 || ((int64)(l.views_.size()) <= conf.splitsize_maxClauseSize.first || l.productOfDomainsExceptLastLEx(vc,conf.splitsize_maxClauseSize.second)))
    {
        ret.emplace_back(l);
        return ret;
    }
    assert(r_== Relation::NE || r_== Relation::EQ || r_ == Relation::LE || r_ == Relation::GE);
    Relation r = Relation::EQ;
    if (conf.break_symmetries)
    {
        if (t==TruthValue::TRUE)
        {
            if (r_==Relation::LE || r_==Relation::GE)
                r = r_;
        }
        if (t==TruthValue::FALSE)
        {
                if (r_==Relation::LE)
                    r = Relation::GE;
                else
                if (r_==Relation::GE)
                    r = Relation::LE;
        }
    }
    ret.resize(conf.splitsize_maxClauseSize.first, LinearConstraint(r)); // creates symmetries
    std::size_t bucket = 0;
    for (auto i : l.views_)
    {
        ret[bucket].add(i);
        bucket=(bucket+1)%conf.splitsize_maxClauseSize.first;
    }

    std::vector<LinearConstraint> result;
    result.emplace_back(l.getRelation());
    result.back().addRhs(l.constant_);
    ///TODO: converting std::size_t to int sucks, but erasing reverse iterator is also not "simple"
    for (int64 i = conf.splitsize_maxClauseSize.first-1; i >=0; --i)
    {

        if (ret[i].views_.size()>=2)
        {
            int factor = ret[i].normalize();
            auto newVar = vc.createVariable(ret[i].lhsDomain(s, vc, conf));
            result.back().add(View(newVar,factor));
            ret[i].add(View(newVar,-1));
        }
        else
        {
            assert(ret[i].views_.size()==1);
            result.back().add(*ret[i].views_.begin());
            ret.erase(ret.begin()+i);
        }
    }

    for (auto i : ret)
    {
        i.normalize();
        auto v = i.split(s, vc, conf, TruthValue::TRUE);
        result.insert(result.end(),std::make_move_iterator(v.begin()), std::make_move_iterator(v.end()));
    }
    for (auto&i : result)
        i.normalize();
    return result;
}


int LinearConstraint::factorize()
{
    if (views_.size()==0) return 1;
    int div = std::abs(views_.front().a);
    for (auto i : views_)
    {
        assert(i.c==0);
        div = gcd(div,std::abs(i.a));
        if (div==1) break;
    }
    if (constant_!=0)
        div = gcd(div,constant_);

    if (div > 1)
    {
        for (auto& i : views_)
            i.a/=div;
        constant_/=div;
    }
    return div;
}


std::vector<ReifiedLinearConstraint> ReifiedLinearConstraint::split(VariableCreator& vc, CreatingSolver& s, const Config& conf) const
{
    std::vector<ReifiedLinearConstraint> ret;
    TruthValue t =TruthValue::UNKNOWN;
    if (s.isTrue(v))
        t = TruthValue::TRUE;
    else
    if (s.isFalse(v))
        t = TruthValue::FALSE;
    auto splitted(l.split(s, vc, conf,t));
    assert(splitted.size()>0);
    ret.reserve(splitted.size());
    ret.emplace_back(std::move(*splitted.begin()),v,impl);

    Literal lit = s.trueLit();
    if (impl != Direction::EQ)       /// in the case of an implication, we also have an implication on the splitted constraints,
        lit = v;    /// disabling them if not needed
    for (auto i = splitted.begin()+1; i != splitted.end(); ++i)
    {
        ret.emplace_back(std::move(*i),lit,Direction::FWD); // original
    }
    return ret;
}


void ReifiedLinearConstraint::normalize()
{
    switch(l.getRelation())
    {
    case LinearConstraint::Relation::EQ: break;
    case LinearConstraint::Relation::NE:
    {
        if (impl==Direction::EQ)
        {
            v = ~v;
            l.setRelation(LinearConstraint::Relation::EQ);
        }
        break;
    }
    case LinearConstraint::Relation::LT: l.setRelation(LinearConstraint::Relation::LE); --l.constant_; break;
    case LinearConstraint::Relation::LE: break;
    case LinearConstraint::Relation::GT: ++l.constant_;
    case LinearConstraint::Relation::GE:
    {
        for (auto &i : l.views_)
        {
            i.a *=-1;
            i.c *=-1;
        }
        l.constant_*=-1;
        l.setRelation(LinearConstraint::Relation::LE);
        break;
    }
    default: assert(false);
    }
    l.normalize();
}

uint64 ReifiedDNF::estimateVariables() const
{
    uint64 size = 0;
    if (dnf_.size()==0) return size;
    for (auto conj : dnf_)
    {
        if (conj.size()==0 || conj.size()==1)
            continue;
        ++size;
    }
    return size <= 1 ? size : ++size;
}



Literal ReifiedDNF::tseitin(CreatingSolver& s) const
{
    if (dnf_.size()==0)
        return s.falseLit();
    LitVec inter;
    for (auto conj : dnf_)
    {
        conj.erase(std::remove_if(conj.begin(), conj.end(),[&s](const Literal& l) { return s.isTrue(l); }), conj.end());
        if (std::any_of(conj.begin(),conj.end(),[&s](const Literal& l) { return s.isFalse(l); }))
            continue;
        if (conj.size()==0)
        {
            inter.emplace_back(s.trueLit());
            continue;
        }
        if (conj.size()==1)
        {
            inter.emplace_back(conj.front());
            continue;
        }
        Literal aux(s.getNewLiteral(false));
        LitVec lv{aux};

        for (auto i : conj)
        {
            s.createClause({~aux,i});
            lv.emplace_back(~i);
        }
        s.createClause(lv);
        inter.emplace_back(aux);
    }

    if (inter.size()==1)
        return inter.back();

    Literal v(s.getNewLiteral(true));
    for (auto i : inter)
    {
        s.createClause({~i,v});
    }

    inter.emplace_back(~v);
    s.createClause(inter);
    return v;
}

}
