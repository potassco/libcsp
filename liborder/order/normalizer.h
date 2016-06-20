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
#include <order/storage.h>
#include <order/constraint.h>
#include <order/linearpropagator.h>
#include <order/config.h>
#include <order/equality.h>
#include <map>

namespace order
{

/// contains the orderLiterals, either need to be moved or Normalizer to be kept
class Normalizer
{
public:
    Normalizer(CreatingSolver& s, Config conf) : s_(s), vc_(s, conf), conf_(conf), ep_(s_,vc_), firstRun_(true) {}

    /// can be made const, only changed for unit tests
    Config& getConfig()
    {
        return conf_;
    }

    /// to create view with an already existing variable, just use View manually
    View createView() { return View(vc_.createVariable()); }
    View createView(const Domain& d) { return View(vc_.createVariable(d)); }

    /// adds a constraint to the propagator l.v <-> l.l
    /// returns false on unsat
    void addConstraint(ReifiedLinearConstraint&& l);
    void addConstraint(ReifiedDomainConstraint&& d);
    void addConstraint(ReifiedAllDistinct&& l);
    void addConstraint(ReifiedDisjoint&& l);
    void addMinimize(View& v, unsigned int level);

    /// do initial propagation
    bool prepare();
    
    /// gives an overapproximation of the number of boolean variables needed
    uint64 estimateVariables();

        
    bool propagate();
    
    bool atFixPoint() const;
    
    bool finalize();
    
    /// converts some of the aux literals in the VVS's into normal once
    /// numVars must be the biggest+1 boolean variable used in all order::Literals so far,
    /// sucht that the test l.var() < numVars is true for all "normal" variables
    /// and false for all aux variables
    void convertAuxLiterals(std::vector<const order::VolatileVariableStorage*>& vvs, unsigned int numVars);

    /// a reference to all linear implications
    /// pre: prepare and createClauses must have been called
    std::vector<ReifiedLinearConstraint>& constraints()
    {
        for (auto& i : linearConstraints_)
            assert(i.impl);
        return linearConstraints_;
    }

    VariableCreator& getVariableCreator() { return vc_; }
    const VariableCreator& getVariableCreator() const { return vc_; }

    
    /// pre: l.normalized()
    /// pre: l.getViews().size()==1
    Literal getLitFromUnary(const LinearConstraint& l)
    {
        assert(l.getViews().size()==1);
        assert(l.normalized());
        View v = *l.getViews().begin();
        Restrictor r = vc_.getRestrictor(v);

        auto it = order::wrap_lower_bound(r.begin(), r.end(), l.getRhs());
        switch(l.getRelation())
        {
            case LinearConstraint::Relation::EQ:
            {
                it = (it == r.end() || *it != l.getRhs()) ? r.end() : it;
                return vc_.getEqualLit(it);
            }
            case LinearConstraint::Relation::NE:
            {
                it = (it == r.end() || *it != l.getRhs()) ? r.end() : it;
                return ~vc_.getEqualLit(it);
            }
            case LinearConstraint::Relation::LE: return vc_.getLELiteral(it);
            case LinearConstraint::Relation::LT:
            case LinearConstraint::Relation::GT:
            case LinearConstraint::Relation::GE:
            default: assert(false);
        }
        assert(false);
        return Literal(0,false);
    }
    
    Literal getEqualLit(View v, int i)
    {
        LinearConstraint l(LinearConstraint::Relation::EQ);
        l.add(v);
        l.addRhs(i);
        l.normalize();
        return getLitFromUnary(l);
    }
    
    const EqualityProcessor::EqualityClassMap& getEqualities() const { return ep_.equalities(); }

//private:
    
    /// pre: prepare()
    bool auxprepare();

    /// uses ReifiedDomainConstraints and ReifiedLinearConstraints to calculate initial variable domain
    /// can remove elements/reorder domainConstraints_/linearConstraints_
    /// return false if domain gets empty
    bool calculateDomains();
    /// throw exception (currently only std::string) if a domain was not restricted
    /// on one of the sides
    ///TODO: need better specification
    /// return sum{domain(i).size()-1} for all domains
    /// pre: prepare() must have been called

    uint64 estimateVariables(const ReifiedDomainConstraint& c);
    uint64 estimateVariables(ReifiedLinearConstraint &c);
    uint64 estimateVariables(const ReifiedAllDistinct& c);
    uint64 estimateVariables(const ReifiedDisjoint& c);
    /// add constraint l as implications to vector insert
    bool convertLinear(ReifiedLinearConstraint&& l, std::vector<ReifiedLinearConstraint> &insert);
    bool addDomainConstraint(ReifiedDomainConstraint&& l);
    bool addDistinct(ReifiedAllDistinct&& l);
    bool addPidgeonConstraint(ReifiedAllDistinct& l);
    bool addPermutationConstraint(ReifiedAllDistinct& l);
    bool addDistinctPairwiseUnequal(ReifiedAllDistinct&& l);
    //bool addDistinctHallIntervals(ReifiedAllDistinct&& l);
    bool addDistinctCardinality(ReifiedAllDistinct&& l);
    bool addDisjoint(ReifiedDisjoint &&l);
    
    void addMinimize();
    /// if constraint is true/false and (0-1 ary), retrict the domain and return true on first parameter(can be simplified away),
    ///  else false
    /// second parameter is false if domain gets empty or UNSAT
    std::pair<bool,bool> deriveSimpleDomain(ReifiedDomainConstraint& d);
    /// pre: constraint must be normalized
    std::pair<bool,bool> deriveSimpleDomain(const ReifiedLinearConstraint &l);

    void addClause(LitVec v);
    /// exactly one value can be true
    bool createOrderClauses();
    /// mapping of order to direct variables
    bool createEqualClauses();
    /// do replace equalities on variables
    /// and introduce "views", so that they share literals
    bool equalityPreprocessing();

    /// a list of all constraints
    //std::vector<ReifiedLinearConstraint> linearImplications_;  /// normalized LE implications

    std::vector<ReifiedLinearConstraint> linearConstraints_;
    std::vector<ReifiedAllDistinct> allDistincts_;
    std::vector<ReifiedDomainConstraint> domainConstraints_;
    std::vector<ReifiedDisjoint> disjoints_;
    std::vector<std::pair<View,unsigned int> > minimize_; /// Views on a level to minimize
    std::vector<uint64>  estimateLE_; // for each variable, number of estimated literals (order)
    std::vector<uint64>  estimateEQ_; // for each variable, number of estimated literals (equal)


    CreatingSolver& s_;
    VariableCreator vc_;
    Config conf_;
    EqualityProcessor ep_;
    
    std::unique_ptr<LinearPropagator> propagator_;
    bool firstRun_;
};

}
