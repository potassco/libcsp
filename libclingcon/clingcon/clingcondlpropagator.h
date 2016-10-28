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

/*#pragma once
#include <clasp/constraint.h>
#include <order/dlpropagator.h>
#include <order/constraint.h>
#include <order/config.h>


//#include <memory>
//#include <cstdint>
#include <unordered_map>


namespace clingcon
{

class ClingconDLPropagator : public Clasp::PostPropagator
{
public:

    ClingconDLPropagator(Clasp::Solver& s, order::Config conf) :
        s_(s), conf_(conf), propagated_pointer_(0)
    {
        activationsPerDl_.emplace_back(0);
        dls_.emplace_back(0);
    }

    virtual ~ClingconDLPropagator() {}

    /// should be called on a normalized constraint to increase chances of being valid
    bool isValidConstraint(const order::ReifiedLinearConstraint &rl) const;
    /// pre, is valid constraint==true
    void addValidConstraint(const order::ReifiedLinearConstraint &rl);

    /// propagator interface
    virtual uint32 priority() const { return Clasp::PostPropagator::priority_reserved_ufs+1; }
    virtual bool   init(Clasp::Solver& s);
    virtual bool   propagateFixpoint(Clasp::Solver& , Clasp::PostPropagator* );
    virtual void   reset();
    virtual bool   isModel(Clasp::Solver& s);

    /// constraint interface
    virtual PropResult propagate(Clasp::Solver& s, Clasp::Literal p, uint32& data);
    virtual void reason(Clasp::Solver& s, Clasp::Literal p, Clasp::LitVec& lits);
    virtual void undoLevel(Clasp::Solver& s);
    virtual bool simplify(Clasp::Solver& , bool) { return false; }

    ///TODO!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! DESTROY MUSS ÜBERLADEN WERDEN, und watches removed

    //! Returns an estimate of the constraint's complexity relative to a clause (complexity = 1).
    /// currently not used in clasp propagators



private:



    Clasp::Literal edgeid2lit(difflogic::DLPropagator::EdgeId id) const;
    //ClingconDLPropagator::LitEdge litIndex2ledge(const Clasp::Literal& l) const; /// warning, this does not take a literal, but a data blob



    Clasp::Solver& s_;
    order::Config conf_;
    Clasp::LitVec literals_; // EdgeId>0 -> literals_[id-1], EdgeId<0 -> ~(literals_[-id-1])
    Clasp::LitVec propagated_; /// i propagated these myself
    unsigned int propagated_pointer_; /// a pointer to the yet unchecked part of the propagated_ list
    /// all these things should be on the watch stack in the same order
    /// but may be interleaved with some others
    std::vector<difflogic::DLPropagator::EdgeId> pending_; /// a list of EdgeId's that are pending to be propagated by us

    std::vector<unsigned int> activationsPerDl_; /// the number of activations per decision level
    std::vector<unsigned int> dls_;
    //std::unordered_map<difflogic::DLPropagator::InternalEdge, Clasp::Literal, difflogic::DLPropagator::Edgehash> edge2Lit_;
    //std::unique_ptr<order::IncrementalSolver> ms_;
    difflogic::DLPropagator p_;

};


}
*/
