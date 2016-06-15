#pragma once
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
    virtual bool simplify(Clasp::Solver& s, bool reinit = false) { return false; }

    ///TODO!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! DESTROY MUSS ÜBERLADEN WERDEN, und watches removed

    //! Returns an estimate of the constraint's complexity relative to a clause (complexity = 1).
    /// currently not used in clasp propagators
    ///virtual uint32 estimateComplexity(const Clasp::Solver& s) const { return 42; /* do some rought guessing by the number of constraints/size*/}


private:
    
    /*struct LitEdge
    {
        LitEdge(Clasp::Literal l, const difflogic::DLPropagator::InternalEdge& edge) : l(l), edge(edge) {}
        Clasp::Literal l;
        difflogic::DLPropagator::InternalEdge edge;
    };*/
    
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
/*
    /// for each Clasp Variable there is a vector of csp Variables with bounds
    /// For each CSP Variable there is an int x
    /// abs(x)-1: steps from the lowest element of the variable to the actual value that is referenced
    /// sign(x): positive if literal without sign means v <= y, negative if literal with sign means v <= y
    std::unordered_map<Clasp::Var, std::vector<std::pair<order::Variable, int32> > > propVar2cspVar_;/// Clasp Literals to csp variables+bound
    //const std::vector<std::unique_ptr<order::LitVec> >& var2OrderLits_; /// CSP variables to Clasp::order

    std::vector<std::size_t> dls_; /// every decision level that we are registered for
    */
};


}
