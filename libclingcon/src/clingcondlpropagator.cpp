#include <clingcon/clingcondlpropagator.h>
#include <clasp/solver.h>
#include <clingcon/solver.h>




namespace clingcon
{


bool ClingconDLPropagator::isValidConstraint(const order::ReifiedLinearConstraint& rl) const
{
    //rl.normalize();
    //rl.factorize();
    return rl.l.getRelation()==order::LinearConstraint::Relation::LE && rl.l.getViews().size()==2 && ((rl.l.getViews().begin()->a==1 && ((rl.l.getViews().begin()+1)->a==-1)) || 
                                             (rl.l.getViews().begin()->a==-1 && ((rl.l.getViews().begin()+1)->a==1)) );

}


void ClingconDLPropagator::addValidConstraint(const order::ReifiedLinearConstraint &rl)
{
    //x -y <= c becomes  x-c->y
    assert(isValidConstraint(rl));
    
    /// interface changed, ask BENNI
    //s_.requestData(rl.v.var()); /// have an extra 32bit space for the reason
    difflogic::DLPropagator::EdgeId id;
    if (rl.l.getViews()[0].a==-1)
        id = p_.addEdge(rl.l.getViews()[1].v, rl.l.getRhs() - rl.l.getViews()[0].c - rl.l.getViews()[1].c, rl.l.getViews()[0].v);
    else
        id = p_.addEdge(rl.l.getViews()[0].v, rl.l.getRhs() - rl.l.getViews()[0].c - rl.l.getViews()[1].c, rl.l.getViews()[1].v);
    (void)(id);
    assert(id==(difflogic::DLPropagator::EdgeId)(literals_.size()+1)); /// should be consecutive
//    literals_.resize(std::max(literals_.size(),(unsigned int)(id)));
//    literals_[id-1]=toClaspFormat(rl.v);
    literals_.push_back(toClaspFormat(rl.v));
}


Clasp::Literal ClingconDLPropagator::edgeid2lit(difflogic::DLPropagator::EdgeId id) const
{
    if (id>0)
        return literals_[id-1];
    else
        return ~literals_[-id-1];
}


Clasp::Constraint::PropResult ClingconDLPropagator::propagate(Clasp::Solver& s, Clasp::Literal p, uint32& data)
{
    /// only called if p is gets true (otherwise ~p gets true)
    assert(s_.isTrue(p));
    Clasp::Literal l(Clasp::Literal::fromRep(data));
    difflogic::DLPropagator::EdgeId id = l.var();
    if (l.sign())
        id *=-1;
    pending_.emplace_back(id);
    //std::cout << "add " << p.var() << "," << p.sign() << " to pending: " << id << " <- id on level " << s_.level(p.var()) << std::endl;
    
    return PropResult(true, true);
}


void ClingconDLPropagator::reason(Clasp::Solver& s, Clasp::Literal p, Clasp::LitVec& lits)
{

    auto it = lits.size();
    (void)(it);
    Clasp::Literal lid = Clasp::Literal::fromRep(s_.reasonData(p));
    difflogic::DLPropagator::EdgeId id = lid.var();
    if (lid.sign())
        id *=-1;
    //std::cout << "Need reason for " << p.var() << "," << p.sign() << " which has data " << id << " at level " << s_.level(p.var()) << std::endl;
    auto reason = p_.reason(id);
    for (auto i : reason)
        lits.push_back(edgeid2lit(i));
//    std::cout << "Reason: ";
//    for (auto i : reason)
//    {
//        Clasp::Literal l = edgeid2lit(i);
//        std::cout << "Data: " << i << " Literal: " << l.var() << "," << l.sign() << " @" << s_.level(l.var()) << " ";
//        std::cout << (s_.isFalse(l) ? "false" : (s_.isTrue(l) ? "true" : "unknown"));
//    }
//    std::cout << std::endl;



    assert(std::count_if(lits.begin()+it, lits.end(), [&](Clasp::Literal i){ return s_.isTrue(i); } )==lits.size()-it);
    //assert(std::count_if(lits.begin()+it, lits.end(), [&](Clasp::Literal i){ return s_.isTrue(i) && (s_.level(i.var()) == s_.decisionLevel()); } )>=1);
    // i case of conflict minimization, it can happen that the decision literal is higher
    assert(std::count_if(lits.begin()+it, lits.end(), [&](Clasp::Literal i){ return s_.isTrue(i) && (s_.level(i.var()) == s_.level(p.var())); } )>=1);
}


bool ClingconDLPropagator::init(Clasp::Solver& s)
{
    for (unsigned int i = 0; i != literals_.size(); ++i)
    {
        if (!s.isFalse(literals_[i]) || !s.isTrue(literals_[i]))
        {
            s.addWatch(literals_[i],this,Clasp::Literal(i+1,false).rep());
            s.addWatch(~literals_[i],this,Clasp::Literal(i+1,true).rep());
        }
    }
    for (unsigned int i = 0; i != literals_.size(); ++i)
    {
        std::vector<difflogic::DLPropagator::EdgeId> consequences;
        if (s.isFalse(literals_[i]))
            consequences = p_.deactivate(i+1);
        else
        if (s.isTrue(literals_[i]))
            consequences = p_.activate(i+1);
            
        for (const auto& j : consequences)
        {
            Clasp::Literal l = edgeid2lit(j);
            if (!s_.isTrue(l))
            {
                if (!s.force(l))
                    return false;
                //std::cout << "initially propagated " << l.var() << "," << l.sign() << " to pending: " << j << std::endl;
                propagated_.push_back(l);
            }
        }
    }
    return true;
}


bool ClingconDLPropagator::propagateFixpoint(Clasp::Solver& , PostPropagator*)
{
    if (pending_.size() && dls_.back()<s_.decisionLevel())
    {
        dls_.emplace_back(s_.decisionLevel());
        s_.addUndoWatch(s_.decisionLevel(),this);
        assert(propagated_.size()==propagated_pointer_);// i do not get called if non of the other propagators has something new
        propagated_.clear();
        propagated_pointer_ = 0;
        activationsPerDl_.emplace_back(0);  
        //std::cout << "#activationperDL list " << activationsPerDl_.size() << " vs. dl level " << s_.decisionLevel() << std::endl;
    }
    
    while(pending_.size())
    {
        for (const auto& i : pending_)
        {
             Clasp::Literal l = edgeid2lit(i);
             if (propagated_pointer_<propagated_.size() && propagated_[propagated_pointer_]==l)
             {  // it was us that made this literal true
                 ++propagated_pointer_;
                 continue;
             }
             if (!p_.isTrue(i)) /// a literal may actually already be true by some other inference in the solver, so i dont have to activate it again
             {
                 //std::cout << "propagate " << l.var() << "," << l.sign() << " @" << s_.level(l.var()) << std::endl;
                 ++(activationsPerDl_.back());
                 auto conseq = p_.activate(i);
                 for (const auto& j : conseq)
                 {
                     Clasp::Literal l = edgeid2lit(j);
                     if (!s_.isTrue(l))
                     {
                         //std::cout << "force" << l.var() << "," << l.sign() << " with data "  << j << " at level " << s_.decisionLevel() << std::endl;
                         if (!s_.force(l,this,Clasp::Literal(abs(j),j<0).rep()))
                             return false;
                         propagated_.push_back(l);
                     }
                 }
             }
        }
        pending_.clear();
        if (!s_.propagateUntil(this))         { return false; } /// only call if propagated_> pointer is not empty
    }
    return true;
}


void ClingconDLPropagator::reset()
{
    if (dls_.back()<=s_.decisionLevel()) /// maybe this can an assert instead of if
    {
        //std::cout << "reset " << s_.decisionLevel() << std::endl;
        propagated_.clear();///
        propagated_pointer_ = 0;
        pending_.clear();
        // i can not undo things here, since reasons may be requested
        //for (unsigned int i = 0; i != activationsPerDl_.back(); ++i)
        //    p_.undo();
        //activationsPerDl_.back()=0;
    }
    else
    {
        //std::cout << "noreset " << s_.decisionLevel() << std::endl;
    }
}


void ClingconDLPropagator::undoLevel(Clasp::Solver& s)
{
    assert(dls_.back()==s_.decisionLevel());
    //std::cout << "undo " << s_.decisionLevel() << std::endl;
    propagated_.clear();
    propagated_pointer_ = 0;
    pending_.clear();
    for (unsigned int i = 0; i != activationsPerDl_.back(); ++i)
        p_.undo();
    activationsPerDl_.pop_back();
    dls_.pop_back();
}

bool ClingconDLPropagator::isModel(Clasp::Solver& s)
{
    return true;
}

}

