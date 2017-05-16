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

#include <order/normalizer.h>
#include <order/types.h>
#include <order/translator.h>

#include <map>
#include <unordered_map>
#include <memory>


namespace order
{


void Normalizer::addConstraint(ReifiedLinearConstraint&& l)
{
    linearConstraints_.emplace_back(std::move(l));
}

void Normalizer::addConstraint(ReifiedDomainConstraint&& d)
{
    domainConstraints_.emplace_back(std::move(d));
}


/// hallsize=0 should be equal to quadratic amount of unequal ?
void Normalizer::addConstraint(ReifiedAllDistinct&& l)
{
    allDistincts_.emplace_back(std::move(l));
}

void Normalizer::addConstraint(ReifiedDisjoint&& l)
{
    disjoints_.emplace_back(std::move(l));
}

void Normalizer::addMinimize(View& v, unsigned int level)
{
    minimize_.emplace_back(v,level);
}


bool Normalizer::convertLinear(ReifiedLinearConstraint&& l, std::vector<ReifiedLinearConstraint>& insert)
{
    Direction impl = l.impl;
    if ((impl==Direction::FWD && s_.isFalse(l.v)) || (impl==Direction::BACK && s_.isTrue(l.v)))
        return true;
    s_.freeze(l.v);
    l.normalize();
    l.impl = Direction::FWD;
    assert(l.l.getRelation()==LinearConstraint::Relation::LE || l.l.getRelation()==LinearConstraint::Relation::EQ || l.l.getRelation()==LinearConstraint::Relation::NE);

    if (l.l.getRelation()==LinearConstraint::Relation::LE)
    {
        if (l.l.getConstViews().size()==1 && impl==Direction::EQ) // In this case we sometimes can make an orderLiteral out of it, if it was not yet otheriwse created
        {
            //ReifiedLinearConstraint copy(l);
            View v = *l.l.getConstViews().begin();
            int rhs = l.l.getRhs();
            //rhs = v.divide(rhs);
            Restrictor r = vc_.getRestrictor(v);
            auto it = order::wrap_upper_bound(r.begin(), r.end(), rhs);
            if (it != r.begin())
                --it;
            else
                it = r.end();
            return vc_.setLELit(it,l.v);
        }
        else
        {
            ReifiedLinearConstraint t(l);
            if (!s_.isFalse(l.v) && (impl & Direction::FWD))
                insert.emplace_back(std::move(l));
            if (!s_.isTrue(t.v) && (impl & Direction::BACK))
            {
                t.reverse();
                t.v = ~t.v;
                insert.emplace_back(std::move(t));
            }
        }
    }
    else
    if (l.l.getRelation()==LinearConstraint::Relation::EQ)
    {
        assert(l.l.getRelation()==LinearConstraint::Relation::EQ);
        if (l.l.getConstViews().size()==1 && impl==Direction::EQ) // In this case we sometimes can make an orderLiteral out of it, if it was not yet otheriwse created
        {
            View v = *(l.l.getConstViews().begin());
            Restrictor r = vc_.getRestrictor(v);
            auto it = order::wrap_lower_bound(r.begin(), r.end(), l.l.getRhs());
            it = (it==r.end() || *it != l.l.getRhs()) ? r.end() : it;
            return vc_.setEqualLit(it,l.v); // otherwise, an equality will be created
        }
        Literal orig = l.v;
        ReifiedLinearConstraint u(l);
        ReifiedLinearConstraint less(l);
        ReifiedLinearConstraint more(l);
        if (!s_.isFalse(l.v) && (impl & Direction::FWD))
        {
            l.l.setRelation(LinearConstraint::Relation::LE);
            insert.emplace_back(std::move(l));
            u.l.setRelation(LinearConstraint::Relation::GE);
            insert.emplace_back(std::move(u));
        }
        if (!s_.isTrue(orig) && (impl & Direction::BACK))
        {
            Literal x = s_.getNewLiteral(true);
            less.v = x;
            less.l.setRelation(LinearConstraint::Relation::LT);
            insert.emplace_back(std::move(less));
            Literal y = s_.getNewLiteral(true);
            more.v = y;
            more.l.setRelation(LinearConstraint::Relation::GT);
            insert.emplace_back(std::move(more));
            if (!s_.createClause(LitVec{~x,~orig})) /// having orig implies not x (having x implies not orig)
                return false;
            if (!s_.createClause(LitVec{~y,~orig})) /// having orig implies not y (having y implies not orig)
                return false;
            if (!s_.createClause(LitVec{~x,~y})) /// cant be less and more at same time
                return false;
            if (!s_.createClause(LitVec{x,y,orig})) /// either equal or less or more
                return false;
        }
    }
    else
    if (l.l.getRelation()==LinearConstraint::Relation::NE)
    {
        if (l.l.getConstViews().size()==1 && impl==Direction::EQ) // In this case we sometimes can make an orderLiteral out of it, if it was not yet otheriwse created
        {
            View v = *(l.l.getConstViews().begin());
            Restrictor r = vc_.getRestrictor(v);
            auto it = order::wrap_lower_bound(r.begin(), r.end(), l.l.getRhs());
            it = (it==r.end() || *it != l.l.getRhs()) ? r.end() : it;
            return vc_.setEqualLit(it,~l.v);
        }
        Literal orig = l.v;
        ReifiedLinearConstraint u(l);
        ReifiedLinearConstraint less(l);
        ReifiedLinearConstraint more(l);
        if (!s_.isFalse(l.v) && (impl & Direction::FWD))
        {
            Literal x = s_.getNewLiteral(true);
            less.v = x;
            less.l.setRelation(LinearConstraint::Relation::LT);
            insert.emplace_back(std::move(less));
            Literal y = s_.getNewLiteral(true);
            more.v = y;
            more.l.setRelation(LinearConstraint::Relation::GT);
            insert.emplace_back(std::move(more));
            if (!s_.createClause(LitVec{~x,orig})) /// having not orig implies not x (having x implies orig)
                return false;
            if (!s_.createClause(LitVec{~y,orig})) /// having not orig implies not y (having y implies orig)
                return false;
            if (!s_.createClause(LitVec{~x,~y})) /// cant be less and more at same time
                return false;
            if (!s_.createClause(LitVec{x,y,~orig})) /// orig -> x or y
                return false;
        }
        if (!s_.isTrue(orig) && (impl & Direction::BACK))
        {
            l.v = ~l.v;
            l.l.setRelation(LinearConstraint::Relation::LE);
            insert.emplace_back(std::move(l));
            u.v = ~u.v;
            u.l.setRelation(LinearConstraint::Relation::GE);
            insert.emplace_back(std::move(u));
        }
    }
    return true;
}


std::pair<bool,bool> Normalizer::deriveSimpleDomain(ReifiedDomainConstraint& d)
{
    View v = d.getView();
    if ((s_.isFalse(d.getLiteral()) && d.getDirection() == Direction::FWD) || (s_.isTrue(d.getLiteral()) && d.getDirection() == Direction::BACK))
        return std::make_pair(true,true);
    if (v.a==0)
    {
        if (d.getDomain().in(v.c))
        {
            if (d.getDirection() == Direction::EQ)
                return std::make_pair(true,s_.setEqual(d.getLiteral(),s_.trueLit()));
            else
                return std::make_pair(true,true);
        }
        else
            return std::make_pair(true,s_.setEqual(d.getLiteral(),s_.falseLit()));
    }
    if (s_.isTrue(d.getLiteral()) && d.getDirection() & Direction::FWD)
    {
        if (!vc_.intersectView(d.getView(),d.getDomain()))
            return std::make_pair(true, false);
        return std::make_pair(true, true);
    }
    else
        if (s_.isFalse(d.getLiteral()) && d.getDirection() & Direction::BACK)
        {
            Domain all;
            all.remove(d.getDomain());
            if (!vc_.intersectView(d.getView(),all))
                return std::make_pair(true, false);
            return std::make_pair(true, true);
        }
    return std::make_pair(false, true);
}

std::pair<bool,bool> Normalizer::deriveSimpleDomain(const ReifiedLinearConstraint& l)
{
    if ((s_.isFalse(l.v) && l.impl==Direction::FWD) || (s_.isTrue(l.v) && l.impl==Direction::BACK))
        return std::make_pair(true,true);

    if (l.l.getViews().size()> 1)
        return std::make_pair(false, true);
    if (l.l.getViews().size()==0)
    {
        bool failed = ((l.l.getRelation()==LinearConstraint::Relation::LE && 0 > l.l.getRhs())
                ||
            (l.l.getRelation()==LinearConstraint::Relation::EQ && 0 != l.l.getRhs())
                ||
            (l.l.getRelation()==LinearConstraint::Relation::NE && 0 == l.l.getRhs()));
        if (failed && (l.impl & Direction::FWD))
            return std::make_pair(true,s_.createClause(LitVec{~l.v}));
        if (!failed && (l.impl & Direction::BACK))
            return std::make_pair(true,s_.createClause(LitVec{l.v}));
        else
            return std::make_pair(true,true);
    }
    auto& view = l.l.getViews().front();
    if (l.l.getRelation()==LinearConstraint::Relation::LE)
    {
        if (s_.isTrue(l.v) && l.impl & Direction::FWD)
        {
            return std::make_pair(true, vc_.constrainUpperBound(view,l.l.getRhs()));
        }
        else
            if (s_.isFalse(l.v) && l.impl & Direction::BACK)
            {
                return std::make_pair(true, vc_.constrainLowerBound(view,l.l.getRhs()+1));
            }
    }
    else
    {
        if(l.l.getRelation()==LinearConstraint::Relation::EQ)
        {
            if (s_.isTrue(l.v))
                return std::make_pair(true, vc_.constrainView(view,l.l.getRhs(), l.l.getRhs()));
            if (s_.isFalse(l.v)) /// not impl
                return std::make_pair(true, vc_.removeFromView(view,l.l.getRhs()));
        }
        else if(l.l.getRelation()==LinearConstraint::Relation::NE)
        {
            if (s_.isTrue(l.v))
                return std::make_pair(true,vc_.removeFromView(view,l.l.getRhs()));
            if (s_.isFalse(l.v)) /// not impl
                return std::make_pair(true, vc_.constrainView(view,l.l.getRhs(), l.l.getRhs()));
        }
    }
    return std::make_pair(false,true);
}


bool Normalizer::addDistinct(ReifiedAllDistinct&& l)
{
    if (conf_.pidgeon)
        if (!addPidgeonConstraint(l))
            return false;
    if (conf_.permutation)
        if (!addPermutationConstraint(l))
            return false;
    if (conf_.alldistinctCard)
        return addDistinctCardinality(std::move(l));
    //if (conf_.hallsize==0)
        return addDistinctPairwiseUnequal(std::move(l));
    //else
    //    return addDistinctHallIntervals(std::move(l));
}

/// size is sum of sizes of views
ViewDomain unify(const std::vector<View>& views, const VariableCreator& vc, uint64& size)
{
    auto it = views.begin();
    //createOrderLiterals(*it);
    ViewDomain d(vc.getViewDomain(*(it++))); // yes, explicitly use domain of the view, not of the variable
    size += d.size();
    for (;it!=views.end(); ++it)
    {
        //createOrderLiterals(*it);
        ViewDomain dd(vc.getViewDomain(*it));
        size+=dd.size();
        d.unify(dd);
    }
    return d;
}

bool Normalizer::addPidgeonConstraint(ReifiedAllDistinct& l)
{
    auto& views = l.getViews();
    if (views.size()==0) return true;
    uint64 size = 0;
    ViewDomain d = unify(views,vc_, size);

    if (views.size()>d.size())
    {
        if (!s_.createClause(LitVec{~l.getLiteral()}))
            return false;
        return true;
    }
    if (size==d.size()) // no overlap between variables
    {
        if ((l.getDirection() & Direction::BACK) && !s_.createClause(LitVec{l.getLiteral()}))
            return false;
        return true;
    }

    int lower = *(d.begin()+(views.size()-1));
    int upper = *(d.end()-(views.size()+1-1));
    LitVec lowerbound{~l.getLiteral()};
    LitVec upperbound{~l.getLiteral()};
    for (const auto& i : views)
    {
        {
        LinearConstraint c(LinearConstraint::Relation::GE);
        c.add(i);
        c.addRhs(lower);
        c.normalize();
        lowerbound.emplace_back(getLitFromUnary(c));
        }

        {
        LinearConstraint c(LinearConstraint::Relation::LE);
        c.add(i);
        c.addRhs(upper);
        c.normalize();
        upperbound.emplace_back(getLitFromUnary(c));
        }
    }
    if(!s_.createClause(lowerbound))
        return false;
    if(!s_.createClause(upperbound))
        return false;
    return true;
}

/// currently only for true distinct constraint
bool Normalizer::addPermutationConstraint(ReifiedAllDistinct& l)
{
    if (!s_.isTrue(l.getLiteral()))
        return true;
    auto& views = l.getViews();
    if (views.size()==0) return true;

    uint64 size = 0;
    ViewDomain d = unify(views,vc_,size);

    if (views.size()>d.size())
    {
        if (!s_.createClause(LitVec{~l.getLiteral()}))
            return false;
        return true;
    }
    if (size==d.size()) // no overlap between variables
    {
        if ((l.getDirection() & Direction::BACK) && !s_.createClause(LitVec{l.getLiteral()}))
            return false;
        return true;
    }

    if (views.size()==d.size())
    {
        for (auto i : d)
        {
            LitVec cond;
            for (auto v : views)
            {
                cond.emplace_back(vc_.getEqualLit(v,i));
                //if (!s_.createClause(LitVec{x,~cond.back(),~l.getLiteral()}))
                //    return false;
            }
            if (!s_.createClause(cond))
                    return false;
            /*
            CHECK THIS AGAIN -> for true either x must be true or better -> just have clause {x_1=v, ... x_n=v} for all values
                    if unknown or false, just dont do it
            Literal x = s_.getNewLiteral(false);
            LitVec cond;
            for (auto v : views)
            {
                cond.emplace_back(vc_.getEqualLit(v,i));
                if (!s_.createClause(LitVec{x,~cond.back(),~l.getLiteral()}))
                    return false;
            }
            cond.emplace_back(~x);
            cond.emplace_back(~l.getLiteral());
            if (!s_.createClause(cond))
                    return false;
            ///in case constraint is false, new free variables are not allowed to vary
            if (!s_.createClause(LitVec{l.getLiteral(),x}))
                return false;
            */
        }
    }
    return true;
}


bool Normalizer::addDistinctCardinality(ReifiedAllDistinct&& l)
{
    auto& views = l.getViews();
    if (views.size()==0) return true;

    uint64 size = 0;
    ViewDomain d = unify(views,vc_,size);

    if (views.size()>d.size())
    {
        if (!s_.createClause(LitVec{~l.getLiteral()}))
            return false;
        return true;
    }
    if (size==d.size()) // no overlap between variables
    {
        if ((l.getDirection() & Direction::BACK) && !s_.createClause(LitVec{l.getLiteral()}))
            return false;
        return true;
    }


    LitVec conditions;
    for (auto i : d)
    {
        LitVec lits;

        for (auto v : views)
            lits.emplace_back(vc_.getEqualLit(v,i));

        Literal x(Literal::fromIndex(0));
        if (s_.isTrue(l.getLiteral()))
            x = s_.falseLit();
        else
            x = s_.getNewLiteral(false);

        conditions.emplace_back(x);
        if (!s_.createCardinality(conditions.back(),2,std::move(lits)))
            return false;
        if (l.getDirection() & Direction::FWD)
        if (!s_.createClause(LitVec{~conditions.back(),~l.getLiteral()}))
            return false;
    }

    conditions.emplace_back(l.getLiteral());
    if (l.getDirection() == Direction::FWD)
        return true;
    return s_.createClause(conditions);
}

bool Normalizer::addDistinctPairwiseUnequal(ReifiedAllDistinct&& l)
{
    auto& views = l.getViews();
    if (views.size()==1) return true;

    uint64 size = 0;
    ViewDomain d = unify(views,vc_,size);

    if (views.size()>d.size())
    {
        if (!s_.createClause(LitVec{~l.getLiteral()}))
            return false;
        return true;
    }
    if (size==d.size()) // no overlap between variables
    {
        if ((l.getDirection() & Direction::BACK) && !s_.createClause(LitVec{l.getLiteral()}))
            return false;
        return true;
    }

    std::vector<LinearConstraint> inequalities;
    for (auto i = views.begin(); i != views.end()-1; ++i)
        for (auto j = i+1; j != views.end(); ++j)
        {
            LinearConstraint temp(LinearConstraint::Relation::NE);
            temp.add(*i);
            temp.add((*j)*-1);///reverse view
            inequalities.emplace_back(temp);
        }

    LitVec lits;
    for (auto& i : inequalities)
    {
        /// in case of an implication, l can be used for all inequalities
        Literal x(0, false);
        if (s_.isTrue(l.getLiteral()))
            x = s_.trueLit();
        else
        {
            if (l.getDirection() == Direction::FWD)
                x = l.getLiteral();
            else
                x = s_.getNewLiteral(true);
        }
        std::vector<ReifiedLinearConstraint> tempv;
        if (!convertLinear(ReifiedLinearConstraint(LinearConstraint(i),x,l.getDirection()),tempv))
            return false;
        else
        {
            linearConstraints_.insert(linearConstraints_.end(), tempv.begin(), tempv.end());
        }

        if (l.getDirection() & Direction::BACK)
        {
            lits.emplace_back(~x);
            if (!s_.createClause(LitVec{~l.getLiteral(),x}))
                return false;
        }
    }


    if (l.getDirection() & Direction::BACK)
    {
        /// if l.v is false, then at least one of the inequalities is also false
        lits.emplace_back(l.getLiteral());
        return s_.createClause(lits);
    }
    return true;
}


bool Normalizer::addDomainConstraint(ReifiedDomainConstraint&& d)
{
    LitVec longc;
    assert(!s_.isFalse(d.getLiteral()) && !s_.isTrue(d.getLiteral()));
    for (const auto &i : d.getDomain().getRanges())
    {
        if (i.l == i.u)
        {
            Literal u = vc_.getEqualLit(d.getView(),i.l);
            longc.emplace_back(u);
        }
        else
        {
            Literal u = s_.getNewLiteral(false);
            longc.emplace_back(u);

            Restrictor r = vc_.getRestrictor(d.getView());
            auto it = order::wrap_lower_bound(r.begin(),r.end(),i.l);
            Literal x = vc_.getGELiteral(it);
            it = order::wrap_upper_bound(it,r.end(),i.u);
            Literal y = vc_.getLELiteral(it-1);

            if (!s_.createClause(LitVec{~u,x})) return false;
            if (!s_.createClause(LitVec{~u,y})) return false;
            if (!s_.createClause(LitVec{u,~x, ~y})) return false;
        }
    }
    if (d.getDirection() & Direction::BACK)
    {
    for (const auto& i : longc)
        if (!s_.createClause(LitVec{~i,d.getLiteral()})) return false;
    }

    if (d.getDirection() & Direction::FWD)
    {
        longc.emplace_back(~d.getLiteral());
        if (!s_.createClause(longc)) return false;
    }

    return true;
}

bool Normalizer::addDisjoint(ReifiedDisjoint&& l)
{
    Direction impl = l.getDirection();
    ReifiedNormalizedDisjoint d(std::move(l),s_);

    if (conf_.disjoint2distinct)
    {
        std::vector<View> views;
        bool allDiff=true;
        /// test if disjoint is an alldifferent
        for (auto& i : d.getViews())
        {
            if (i.size()!=1)
            {
                allDiff=false;
                break;
            }
            auto pair = i.back();
            if (!s_.isTrue(pair.second))
            {
                allDiff=false;
                break;
            }
            views.emplace_back(pair.first);
        }

        if (allDiff)
        {
            addConstraint(ReifiedAllDistinct(std::move(views),l.getLiteral(),l.getDirection()));
            return true;
        }
    }

    /// first vector stores for every tuple
    /// a map having key is the value the variable should take
    /// and DNF as two vectors
    std::vector<std::map<int,std::vector<std::vector<Literal>>>> DNF;
    for (auto &i : d.getViews())
    {
        DNF.emplace_back(std::map<int,std::vector<std::vector<Literal>>>());
        auto &map = DNF.back();
        for (auto &viewcondpair : i)
        {
            View v = viewcondpair.first;
            Literal cond = viewcondpair.second;
            auto dom = vc_.getViewDomain(v);
            for (auto val : dom)
            {
                auto it = map.lower_bound(val);
                if (it == map.end() || (it->first != val))
                    it = map.insert(it,std::make_pair(val,std::vector<std::vector<Literal>>()));
                it->second.emplace_back(std::vector<Literal>{vc_.getEqualLit(v,val), cond});
            }
        }
    }


    Domain dom(1,-1);
    std::vector<std::map<int,Literal>> conditions;
    for (auto &tuple : DNF)
    {
        conditions.emplace_back(std::map<int,Literal>());
        auto &map = conditions.back();
        for (auto &value : tuple)
        {
            ReifiedDNF d(std::move(value.second));
            Literal aux_x_y(d.tseitin(s_));             /// is true if one of the variables for the tuple has been set to "value"
            assert(map.find(value.first)==map.end()); // should only occur once
            map.insert(std::make_pair(value.first,aux_x_y));
//            auto it = map.lower_bound(value.first);
//            if (it == map.end() || (it->first != value.first))
//                it = map.insert(it, std::make_pair(value.first,aux_x_y));
//            else
//                it->second.emplace_back(aux_x_y);
            dom.unify(value.first,value.first);
        }
    }


    LitVec auxs;
    ///1. add a new literal which is reified with a cardinality constraint
    ///2. make reification with original d
    for (const auto &value : dom)
    {

        LitVec v;
        for (auto &tuple : conditions)
        {
            const auto& it = tuple.find(value);
            if (it != tuple.end())
            {
                v.emplace_back(it->second);
            }
        }
        Literal aux = s_.getNewLiteral(false);
        if (!s_.createCardinality(aux,2,std::move(v)))
            return false;
        if (!s_.createClause(LitVec{~aux,~d.getLiteral()}))
            return false;
        auxs.emplace_back(aux);
    }

    auxs.emplace_back(d.getLiteral());
    if ((impl & Direction::BACK) && !s_.createClause(auxs))
        return false;

    return true;
}

bool Normalizer::calculateDomains()
{
    size_t removed = 0;
    for (size_t i = 0; i < domainConstraints_.size()-removed;)
    {
        auto& d = domainConstraints_[i];
        auto p = deriveSimpleDomain(d);
        if (!p.second) return false; // empty domain
        if (p.first) // simplified away
        {
            ++removed;
            domainConstraints_[i] = std::move(domainConstraints_[domainConstraints_.size()-removed]);
        }
        else
            ++i;
    }
    domainConstraints_.erase(domainConstraints_.begin()+(domainConstraints_.size()-removed), domainConstraints_.end());


    removed = 0;
    for (size_t i = 0; i < linearConstraints_.size()-removed;)
    {
        auto& d = linearConstraints_[i];
        d.normalize();
        auto p = deriveSimpleDomain(d);
        if (!p.second) return false; // empty domain
        if (p.first) // simplified away
        {
            ++removed;
            linearConstraints_[i] = std::move(linearConstraints_[linearConstraints_.size()-removed]);
        }
        else
            ++i;
    }
    linearConstraints_.erase(linearConstraints_.begin()+(linearConstraints_.size()-removed), linearConstraints_.end());


    removed = 0;
    for (size_t i = 0; i < this->disjoints_.size()-removed;)
    {
        if ((disjoints_[i].getDirection()==Direction::FWD && s_.isFalse(disjoints_[i].getLiteral())) || (disjoints_[i].getDirection()==Direction::BACK && s_.isTrue(disjoints_[i].getLiteral())))
        {
            ++removed;
            disjoints_[i] = std::move(disjoints_[disjoints_.size()-removed]);
        }
        else
            ++i;
    }
    disjoints_.erase(disjoints_.begin()+(disjoints_.size()-removed), disjoints_.end());

    removed = 0;
    for (size_t i = 0; i < this->allDistincts_.size()-removed;)
    {
        if ((allDistincts_[i].getDirection()==Direction::FWD && s_.isFalse(allDistincts_[i].getLiteral())) || (allDistincts_[i].getDirection()==Direction::BACK && s_.isTrue(allDistincts_[i].getLiteral())))
        {
            ++removed;
            allDistincts_[i] = std::move(allDistincts_[allDistincts_.size()-removed]);
        }
        else
            ++i;
    }
    allDistincts_.erase(allDistincts_.begin()+(allDistincts_.size()-removed), allDistincts_.end());

    /// restrict the domains according to the found equalities
    if (firstRun_)
    for (auto ec : ep_.equalities())
    {
        if (ec.first==ec.second->top()) /// original variable
        {
            for (auto it : ec.second->getConstraints())
            {
                //Variable v = it.first;
                EqualityClass::Edge e = it.second;
                getVariableCreator().constrainDomain(ec.second->top(),e.secondCoef,e.constant,e.firstCoef);
            }
        }
    }

    return true;
}

namespace
{
uint64 allLiterals(Variable v, const VariableCreator& vc)
{
    return std::max(vc.getDomainSize(v),1u)-1;
}

}

uint64 Normalizer::estimateVariables()
{
    estimateLE_.resize(std::max(estimateLE_.size(),getVariableCreator().numVariables()),0);
    estimateEQ_.resize(std::max(estimateLE_.size(),getVariableCreator().numVariables()),0);
    std::fill(estimateLE_.begin(), estimateLE_.end(),0); /// reset vector
    std::fill(estimateEQ_.begin(), estimateEQ_.end(),0); /// reset vector
    uint64 sum = 0;

    for (auto& i : minimize_)
        estimateLE_[i.first.v]=allLiterals(i.first.v,getVariableCreator());
    for (auto& i : linearConstraints_)
        sum += estimateVariables(i);
    for (auto& i : domainConstraints_)
        sum += estimateVariables(i);
    for (auto& i : allDistincts_)
        sum += estimateVariables(i);
    for (auto& i : disjoints_)
        sum += estimateVariables(i);

    for (Variable i = 0; i <= getVariableCreator().numVariables(); ++i)
        if (getVariableCreator().isValid(i))
        {
            // only order lits
            uint64 min = conf_.minLitsPerVar == -1 ? allLiterals(i,getVariableCreator()) : std::min((uint64)(conf_.minLitsPerVar),allLiterals(i,getVariableCreator()));

            min += estimateLE_[i]; /// estimation plus number added by minLitsPerVar
            min = std::min(min,uint64(getVariableCreator().getDomainSize(View(i))-getVariableCreator().numOrderLits(i))); // either min or less if i cant add so many
            sum += min;
            sum += estimateEQ_[i];
            /// ???
//            THIS IS ALL TO COMPLICATED !!!!, estimate should be excact, unordered map or std::set ? for new literals ?
//            do estimate for order and equal lits seperately
//            if (estimateLE_[i]< allOrderLiterals(i,getVariableCreator())+min )
//            // order and equality lits
//                sum += std::min(estimateLE_[i]+min,allLiterals(i,getVariableCreator()));
//            sum -= getVariableCreator().numOrderLits(i);

        }
    //sum -= getVariableCreator().numEqualLits();
    return sum;
}

uint64 Normalizer::estimateVariables(const ReifiedDomainConstraint& d)
{
    uint64 ret = 0;
    Variable v = d.getView().v;
    for (const auto &i : d.getDomain().getRanges())
    {
        if (i.l == i.u)
            estimateEQ_[v] = std::min(estimateEQ_[v]+1,allLiterals(v,getVariableCreator()));
        else
            ++ret;
        estimateLE_[v] = std::min(estimateLE_[v]+2,allLiterals(v,getVariableCreator()));
    }
    return ret;
}

uint64 Normalizer::estimateVariables(ReifiedLinearConstraint& l)
{
    uint64 ret = 0;
    if ((l.impl == Direction::FWD && s_.isFalse(l.v)) || (l.impl == Direction::BACK && s_.isTrue(l.v)))
        return ret;
    l.normalize();
    if (l.l.getRelation()==LinearConstraint::Relation::LE)
    {
        if (l.l.getConstViews().size()==1 && l.impl == Direction::EQ) // In this case we sometimes can make an orderLiteral out of it, if it was not yet otheriwse created
        {
            Variable v = l.l.getViews().begin()->v;
            estimateLE_[v] = std::min(estimateLE_[v]+1,allLiterals(v,getVariableCreator()));
            return ret;
        }
    }
    else
    if (l.l.getRelation()==LinearConstraint::Relation::EQ)
    {
        if (l.l.getConstViews().size()==1 && l.impl == Direction::EQ) // In this case we sometimes can make an orderLiteral out of it, if it was not yet otheriwse created
        {
            Variable v = l.l.getViews().begin()->v;
            estimateEQ_[v] = std::min(estimateEQ_[v]+1,allLiterals(v,getVariableCreator()));
            estimateLE_[v] = std::min(estimateLE_[v]+2,allLiterals(v,getVariableCreator()));
            return ret;
        }

        if (!s_.isTrue(l.v) && l.impl & Direction::BACK)
            ret +=2;
    }
    else
    if (l.l.getRelation()==LinearConstraint::Relation::NE)
    {
        if (l.l.getConstViews().size()==1 && l.impl == Direction::EQ) // In this case we sometimes can make an orderLiteral out of it, if it was not yet otheriwse created
        {
            Variable v = l.l.getViews().begin()->v;
            estimateEQ_[v] = std::min(estimateEQ_[v]+1,allLiterals(v,getVariableCreator()));
            estimateLE_[v] = std::min(estimateLE_[v]+2,allLiterals(v,getVariableCreator()));
            return ret;
        }

        if (!s_.isFalse(l.v) && l.impl & Direction::FWD)
            ret+=2;
    }


    int factor = 1;
    if (l.l.getRelation()==LinearConstraint::Relation::EQ || l.l.getRelation()==LinearConstraint::Relation::NE)
        factor = 2; // we have to consider both directions
    uint64 size = conf_.translateConstraints == -1 ? std::numeric_limits<uint64>::max() : conf_.translateConstraints;
    uint64 product = l.l.productOfDomainsExceptLast(getVariableCreator());
    if (product<=size)
    {
        for (const auto& view : l.l.getViews())
            estimateLE_[view.v] = std::min(estimateLE_[view.v]+std::min(product*factor,allLiterals(view.v,getVariableCreator())),allLiterals(view.v,getVariableCreator()));
    }


    return ret;
}


uint64 Normalizer::estimateVariables(const ReifiedAllDistinct& c)
{
    auto& views = c.getViews();
    if (views.size()<=1) return 0;

    uint64 size = 0;
    ViewDomain d = unify(views,vc_,size);

    if (views.size()>d.size())
        return 0;

    if (size==d.size()) // no overlap between variables
        return 0;

    if (conf_.pidgeon)
    {
        for (auto& i : views)
            if (getVariableCreator().isValid(i.v))
                estimateLE_[i.v]= std::min(allLiterals(i.v,getVariableCreator()), 2 + estimateLE_[i.v]);
    }

    if (conf_.permutation)
    {
        if (views.size()==d.size())
        {
            for (auto& i : views)
                if (getVariableCreator().isValid(i.v))
                {
                    estimateLE_[i.v]= allLiterals(i.v,getVariableCreator());
                    estimateEQ_[i.v]= allLiterals(i.v,getVariableCreator());
                }
        }
    }

    if (conf_.alldistinctCard)
    {
        for (auto& i : views)
        {
            estimateLE_[i.v]= std::min(uint64(estimateLE_[i.v]) + uint64(d.size()-1)*2,allLiterals(i.v,getVariableCreator()));
            estimateEQ_[i.v]= std::min(uint64(estimateLE_[i.v]) + uint64(d.size()-1)*1,allLiterals(i.v,getVariableCreator()));
        }
        return (s_.isTrue(c.getLiteral()) ? 0 : d.size());
    }
    else// pairwise inequality
    {
        uint64 size = conf_.translateConstraints == -1 ? std::numeric_limits<uint64>::max() : conf_.translateConstraints;
                /// if we translate the inequalities, we need some order lits
        uint64 max = 0;
        for (auto i : views)
        {
            uint64 n = allLiterals(i.v,getVariableCreator());
            if (n <= size)
                max = std::max(max,n);
        }
        for (auto i : views)
        {
            uint64 n = allLiterals(i.v,getVariableCreator());
            if (n <= size)
                estimateLE_[i.v] = std::min(estimateLE_[i.v] + n,allLiterals(i.v,getVariableCreator()));
            else
                estimateLE_[i.v] = std::min(estimateLE_[i.v] + max,allLiterals(i.v,getVariableCreator()));
        }
        return ((views.size()*views.size() + 1)/2)*2 + ((s_.isTrue(c.getLiteral()) || c.getDirection() == Direction::FWD) ? 0 : ((views.size()*views.size() + 1) / 2));
    }
    //return d.size()*conf_.hallsize*views.size() +  d.size()*conf_.hallsize*splitHallConstraintEstimate(s_, views.size(),conf_) + permutation;
}



uint64 Normalizer::estimateVariables(const ReifiedDisjoint& d)
{
    uint64 sum = 0;
    ///tseitin of conditions
    for (const auto &i : d.getViews())
        for (const auto &j : i)
            sum += j.second.estimateVariables();

    std::vector<View> vars;
    bool allDiff=true;
    /// test if reified is an alldifferent
    for (const auto& i : d.getViews())
    {
        if (i.size()!=1)
        {
            allDiff=false;
            break;
        }
        auto pair = i.back();
        vars.emplace_back(pair.first);
    }

    if (allDiff && conf_.disjoint2distinct)
    {
        return sum + estimateVariables(ReifiedAllDistinct(std::move(vars),d.getLiteral(),d.getDirection()));
    }

    ViewDomain dom(1,-1);
    for (const auto &i : d.getViews())
    {
        for (auto &varcondpair : i)
        {
            View v = varcondpair.first;
            auto d = vc_.getViewDomain(v);
            estimateLE_[v.v] = std::max(estimateLE_[v.v] + (uint64)(std::max(d.size(),(uint64)1u)-1)*2-1,allLiterals(v.v,getVariableCreator()));
            estimateEQ_[v.v] = std::max(estimateEQ_[v.v] + (uint64)(std::max(d.size(),(uint64)1u)-1)*1-1,allLiterals(v.v,getVariableCreator()));
            dom.unify(d);
        }
    }
    return sum += dom.size();
}






bool Normalizer::prepare()
{
    if (firstRun_)
    {
        varsBefore_ = 0;
        varsAfter_ = vc_.numVariables();
    }
    else
    {
        varsBefore_ = varsAfterFinalize_;
        varsAfter_ = vc_.numVariables();
    }
    if (conf_.equalityProcessing)
        if (!equalityPreprocessing(firstRun_))
            return false;

    /// calculate very first domains for easy constraints and remove them
    if (!calculateDomains())
        return false;

    /// split constraints
    std::size_t csize = linearConstraints_.size();
    for (std::size_t i = 0; i < csize; ++i)
    {
        ///TODO: sugar does propagation while splitting, this can change order/everything
        linearConstraints_[i].normalize();
        std::vector<ReifiedLinearConstraint> splitted = linearConstraints_[i].split(vc_, s_, conf_);
        //linearImplications_.reserve(splitted.size()+linearConstraints_.size()-1);// necessary? insert could do this
        assert(splitted.size()>0);
        linearConstraints_[i] = std::move(*splitted.begin());

        for (auto j = splitted.begin()+1; j < splitted.end(); ++j)
            addConstraint(std::move(*j));
    }

    /// do propagation on true/false literals
    LinearPropagator p(s_, vc_, conf_);
    for (auto& i : linearConstraints_)
    {
        i.l.normalize();
        if (!s_.isTrue(i.v) && !s_.isFalse(i.v))
            continue;
        if ( i.l.getRelation()==LinearConstraint::Relation::LE )
        {
            ReifiedLinearConstraint l(i); // make a copy

            if (s_.isTrue(l.v) && l.impl & Direction::FWD)
            {
                p.addImp(std::move(l));
            }
            else
                if (s_.isFalse(l.v) && l.impl & Direction::BACK)
                {
                    l.reverse();
                    l.v = ~l.v;
                    p.addImp(std::move(l));
                }
        }
        if (i.l.getRelation()==LinearConstraint::Relation::EQ && s_.isTrue(i.v) && i.impl==Direction::FWD)
        {
            ReifiedLinearConstraint l(i); // make a copy
            l.l.setRelation(LinearConstraint::Relation::LE);
            p.addImp(std::move(l));
            ReifiedLinearConstraint u(i); // make a copy
            u.l.setRelation(LinearConstraint::Relation::GE);
            p.addImp(std::move(u));
        }
    }
    if(!p.propagate())
        return false;

    ///update the domain
    for (std::size_t i = 0; i < vc_.numVariables(); ++i)
    {
        if (getVariableCreator().isValid(i))
        {
            auto r = p.getVariableStorage().getCurrentRestrictor(i);
            vc_.constrainView(View(i),r.lower(), r.upper()); /// should be the same as intersect (but cheaper),
        }
        /// as i only do bound propagation
    }

    firstRun_ = false;

    return auxprepare();
}

void Normalizer::addMinimize()
{
    for (auto p : minimize_)
    {
        View v = p.first;
        unsigned int level = p.second;
        auto& vc = getVariableCreator();
        auto res = vc.getRestrictor(v);
        order::uint64 before = 0;
        for (auto it = res.begin(); it != res.end(); ++it)
        {
            int32 w = ((*it) - before);
            before = *it;
            s_.addMinimize(vc.getGELiteral(it),w,level);
        }
    }
    minimize_.clear();
}


bool Normalizer::auxprepare()
{
//    std::cout << "prepare" << std::endl;
//    for (auto& i : linearConstraints_)
//        std::cout << "Constraint " << i.l << " with rep " << i.v.asUint() << " is " << s_.isFalse(i.v) << " " << s_.isTrue(i.v) << std::endl;

    //for (std::size_t i = 0; i < linearConstraints_.size(); ++i)
    //    std::cout << ":\t" << linearConstraints_[i].l << " << " << linearConstraints_[i].v.asUint() << " true/false " << s_.isTrue(linearConstraints_[i].v) << "/" << s_.isFalse(linearConstraints_[i].v) <<std::endl;

    s_.createNewLiterals(estimateVariables());

    /// 1st: add all linear constraints
    std::vector<ReifiedLinearConstraint> tempv;
    unsigned int end = linearConstraints_.size();
    for (unsigned int i = 0; i < end;)
    {
        if (!convertLinear(std::move(linearConstraints_[i]),tempv)) /// adds it to the constraints
            return false;
        else
        {
            if (tempv.size())
            {
                tempv.front().normalize();
                tempv.front().l.sort(vc_,conf_);
                linearConstraints_[i] = std::move(tempv.front());
                for (auto j = tempv.begin()+1; j != tempv.end(); ++j)
                {
                    j->normalize();
                    j->l.sort(vc_,conf_);
                    linearConstraints_.emplace_back(std::move(*j));
                }
                tempv.clear();
                ++i;
            }
            else
            {
                linearConstraints_[i] = std::move(linearConstraints_.back());
                linearConstraints_.pop_back();
                if (end<linearConstraints_.size()+1)
                    ++i;
                else
                    --end;
            }

        }
    }
    for (auto& i : domainConstraints_)
    {
        if (!addDomainConstraint(std::move(i)))
            return false;
    }
    domainConstraints_.clear();
    //for (std::size_t i = 0; i < linearImplications_.size(); ++i)
    //    std::cout << i << ":\t" << linearImplications_[i].l << " << " << linearImplications_[i].v.asUint() << std::endl;

    for (auto& i : disjoints_)
        if (!addDisjoint(std::move(i)))
            return false;
    disjoints_.clear();

    /// add even more constraints
    for (auto& i : allDistincts_)
        if (!addDistinct(std::move(i)))
            return false;
    allDistincts_.clear();

    /// remove 0sized linear constraints
    auto size = linearConstraints_.size();
    for (unsigned int i = 0; i < size;)
    {
        auto& l = linearConstraints_[i];
        l.normalize();
        if (l.l.getConstViews().size()==0)
        {
            if (0 <= l.l.getRhs()) /// linear constraint  is satisfied
            {
                if (l.impl & Direction::BACK)
                    if (!s_.setEqual(l.v,s_.trueLit()))
                        return false;
            }
            else  /// linear constraint cant be satisfied
            {
                if (l.impl & Direction::FWD)
                {
                    if (!s_.setEqual(l.v,s_.falseLit()))
                        return false;
                }
            }
            /// always remove
            --size;
            std::swap(l,linearConstraints_[size]);
        }
        else
            ++i;
    }

    linearConstraints_.erase(linearConstraints_.end()-(linearConstraints_.size()-size), linearConstraints_.end());

    /// remove duplicates
    sort( linearConstraints_.begin(), linearConstraints_.end(), ReifiedLinearConstraint::compareless );
    linearConstraints_.erase( unique( linearConstraints_.begin(), linearConstraints_.end(), ReifiedLinearConstraint::compareequal ), linearConstraints_.end() );


    // TODO do propagate only if added allDistinct constraints
    ///Cant do propagation here, as it does not propagate to clasp -> can restrict domains
    /// -> this can result in unused orderLits -> make them false in createOrderLits
    ///So either not propagate -> more order lits
    /// propagate and add clauses for clasp manually that apply after creating order literals
    /// cant use propagateWithReason as it uses orderLiterals

    ///TODO: do restrict variables if equality preprocessing restricts integer variables
    if (!vc_.restrictDomainsAccordingToLiterals())
        return false;

    propagator_.reset(new LinearPropagator(s_,vc_,conf_));
    propagator_->addImp(std::move(linearConstraints_));
    linearConstraints_.clear();

    return propagate();

}

bool Normalizer::propagate()
{
    if (!vc_.restrictDomainsAccordingToLiterals())
        return false;
    auto temp = std::move(propagator_->removeConstraints());
    propagator_.reset(new LinearPropagator(s_,vc_,conf_));
    propagator_->addImp(std::move(temp));

    // do propagate all original constraints
    if(!propagator_->propagate())
    {
        return false;
    }
    ///update the domain
    for (std::size_t i = 0; i < vc_.numVariables(); ++i)
    {
        if (getVariableCreator().isValid(i))
        {
            const auto& r = propagator_->getVariableStorage().getCurrentRestrictor(i);
            if (!vc_.constrainView(View(i), r.lower(), r.upper()))
                return false;
        }
    }
    return true;
}


bool Normalizer::atFixPoint() const
{
    assert(propagator_);
    return !propagator_->propagated();
}

bool Normalizer::finalize()
{
    linearConstraints_ = propagator_->removeConstraints();
    propagator_.reset();

//    std::cout << "finalize" << std::endl;
//    for (auto& i : linearConstraints_)
//        std::cout << "Constraint " << i.l << " with rep " << i.v.asUint() << " is " << s_.isFalse(i.v) << " " << s_.isTrue(i.v) << std::endl;

    vc_.prepareOrderLitMemory();

    if (!createEqualClauses())
        return false;

    if (!vc_.createOrderLiterals())
        return false;

    if (!translate(s_,getVariableCreator(),constraints(),conf_))
        return false;

    if (conf_.explicitBinaryOrderClausesIfPossible && !createOrderClauses())
        return false;

    addMinimize();

    //for (std::size_t i = 0; i < constraints_.size(); ++i)
    //    std::cout << i << ":\t" << constraints_[i].l << " << " << constraints_[i].v.asUint() << std::endl;

    /// make all unecessary ones false
    s_.makeRestFalse();

    assert(allDistincts_.size()==0);
    assert(disjoints_.size()==0);
    assert(domainConstraints_.size()==0);
    assert(minimize_.size()==0);

    varsAfterFinalize_ = vc_.numVariables();

    return true;
}


void Normalizer::variablesWithoutBounds(std::vector<order::Variable>& lb, std::vector<order::Variable>& ub)
{
    for (unsigned int i = varsBefore_; i < varsAfter_;++i)
    {
        if (vc_.isValid(i))
        {
        if (vc_.getDomain(i).lower()==order::Domain::min)
            lb.push_back(i);
        if (vc_.getDomain(i).upper()==order::Domain::max)
            ub.push_back(i);
        }
    }
}


void Normalizer::convertAuxLiterals(std::vector<const order::VolatileVariableStorage*>& vvss, unsigned int numVars)
{
    if (vvss.size()==0)
        return;
    if (conf_.convertLazy.second) /// union
    {
        for (const auto& vvs : vvss)
        {
            auto& vc = vvs->getVariableStorage();
            for (order::Variable var = 0; var != vc.numVariables(); ++var)
            {
                unsigned int before = getVariableCreator().numOrderLits(var);
                unsigned int after = vvs->getStorage(var).numLits()-1;
                if (after-before==0)
                    continue;
                //std::cout << "creating " << after-before << " new literals" << std::endl;
                s_.createNewLiterals(after-before);
                order::pure_LELiteral_iterator it(vc.getRestrictor(order::View(var)).begin(),vvs->getStorage(var),true);

                while(it.isValid())
                {
                    Literal l = (*it);
                    if (l.var() > numVars) /// actually comparing clasp variables with order::Literals
                    {
                        //std::cout << l.var() << " is an aux var" << std::endl;

                        auto newlit = s_.getNewLiteral(true);
                        getVariableCreator().setLELit(vc.getRestrictor(order::View(var)).begin()+it.numElement(),newlit);
                    }
                    ++it;
                }
            }
        }
    }
    else
    {
        /// intersection
        /// for each variable
        for (order::Variable var = 0; var != vc_.numVariables(); ++var)
        {
            unsigned int before = getVariableCreator().numOrderLits(var);

            ///compute the thread with the fewest literals
            unsigned int smallest = 0;

            unsigned int size=std::numeric_limits<unsigned int>::max();
            unsigned int minimumLits = std::numeric_limits<unsigned int>::max(); /// the minimum number of lits to be added
            for (unsigned int i = 0; i != vvss.size(); ++i)
            {
                unsigned int nl = vvss[i]->getStorage(var).numLits()-1;
                if (vvss[i]->getStorage(var).numLits()<size)
                {
                    smallest = i;
                    size = nl;

                }
                minimumLits = std::min(minimumLits,nl-before);
            }
            if (!minimumLits)
                continue;

            const VolatileVariableStorage* smallestVVS= vvss[smallest];
            auto& vc = smallestVVS->getVariableStorage();

            s_.createNewLiterals(minimumLits);

            order::pure_LELiteral_iterator it(vc.getRestrictor(order::View(var)).begin(),smallestVVS->getStorage(var),true);

            while(it.isValid())
            {
                if (smallestVVS->getStorage(var).getLiteral(it.numElement()).var()>numVars) // aux var
                {
                    bool found = true;
                    /// check for all other threads if there is a literal at it position
                    for (unsigned int i = 0; i!=smallest && i != vvss.size(); ++i)
                    {
                        if (vvss[i]->getStorage(var).hasNoLiteral(it.numElement()))
                        {
                            found = false;
                            break;
                        }
                    }

                    assert(getVariableCreator().getStorage(var).hasNoLiteral(it.numElement()));

                    if (found)
                    {
                        //std::cout << "\033[1;31m" << "added " << var << " on pos " << it.numElement() << "\033[0m" << std::endl;
                        auto newlit = s_.getNewLiteral(true);
                        getVariableCreator().setLELit(vc.getRestrictor(order::View(var)).begin()+it.numElement(),newlit);
                    }
                }
                ++it;
            }
            /// loop through the literals
            ///
            ///
            s_.makeRestFalse();
        }
    }
}

bool Normalizer::createOrderClauses()
{
    /// how to decide this incrementally?  recreate per var ?
    for (Variable var = 0; var <vc_.numVariables(); ++var)
    {
        if (getVariableCreator().isValid(var))
        {
            const auto& lr = vc_.getRestrictor(View(var));
            if (lr.size()>=3)
            {
                auto start = pure_LELiteral_iterator(lr.begin(), vc_.getStorage(var), true);
                auto end =   pure_LELiteral_iterator(lr.end()-2, vc_.getStorage(var), false);
                if (end.isValid())
                for (auto next = start; next != end;)
                {
                    auto old = next;
                    ++next;
                    if (old.isValid() && next.isValid() && old.numElement()+1 == next.numElement())
                        if (!s_.createClause(LitVec{~(*old),*next}))
                            return false;
                    //if (vc_.hasLELiteral(v) && vc_.hasLELiteral(v+1))
                    //if (!s_.createClause(LitVec{~(vc_.getLELiteral(v)),vc_.getLELiteral(v+1)}))// could use GELiteral instead
                    //    return false;
                    //++v;
                }
            }
        }
    }
    return true;
}

bool Normalizer::createEqualClauses()
{
    return vc_.createEqualClauses();
}


bool Normalizer::equalityPreprocessing(bool firstRun)
{
    if (firstRun)
    {
        if (!ep_.process(linearConstraints_))
            return false;
        auto unary = ep_.getUnaries();
        for (auto i : unary)
        {
            if (!vc_.constrainView(View(i.first,1,0),i.second,i.second))
                return false;
        }
    }
    for (auto& i : linearConstraints_)
        if (!ep_.substitute(i.l))
            return false;
    for (auto& i : allDistincts_)
        if (!ep_.substitute(i))
            return false;
    for (auto& i : domainConstraints_)
        if (!ep_.substitute(i))
            return false;
    for (auto& i : disjoints_)
        if (!ep_.substitute(i))
            return false;
    for (auto& i : minimize_)
        if (!ep_.substitute(i.first))
            return false;
    if (firstRun)
    {
        for (Variable v = 0; v != getVariableCreator().numVariables(); ++v)
            if (!ep_.isValid(v))
                getVariableCreator().removeVar(v);
    }
    return true;
}

}
