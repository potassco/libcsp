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

#ifndef APPSUPPORT_HH
#define APPSUPPORT_HH

#include <clasp/constraint.h>
#include <order/linearpropagator.h>
#include <clingcon/solver.h>
#include <order/equality.h>
#include <clingcon/theoryparser.h>
#include <clingcon/clingconorderpropagator.h>
#include <clingcon/clingcondlpropagator.h>
#include <clingcon/theoryparser.h>
#include <clasp/enumerator.h>
#include <clasp/cli/clasp_options.h>

#include <memory>
#include <cstdint>
#include <unordered_map>

namespace clingcon
{

class TheoryOutput : public Clasp::OutputTable::Theory
{
public:
    TheoryOutput() { for (auto& i : props_) i = nullptr; }
    //! Called once on new model m. Shall return 0 to indicate no output.
    virtual const char* first(const Clasp::Model& m)
    {
        curr_ = names_.begin();
        currentSolverId_ = m.sId;
        return next();
    }

    //! Shall return 0 to indicate no output.
    virtual const char* next()
    {
        while (curr_ != names_.end())
        {
            const char* ret = props_[currentSolverId_]->printModel(curr_->first,curr_->second.first);
            if (ret)
            {
            ++curr_;
            return ret;
            }
            ++curr_;
        }
        return 0;        
    }
    clingcon::NameList::iterator curr_;
    unsigned int currentSolverId_;
    clingcon::NameList names_; /// order::Variable to name + condition
    static const int numThreads = 64;
    clingcon::ClingconOrderPropagator* props_[numThreads];

};

class Configurator : public Clasp::Cli::ClaspCliConfig::Configurator
{
public:
    
    Configurator(order::Config conf, order::Normalizer& n, TheoryOutput& to) : conf_(conf), n_(n), to_(to), cp_(0)
    {}
    
    ~Configurator()
    { 
        for (unsigned int i = 0; i < to_.numThreads; ++i)
            if (to_.props_[i] != nullptr)
            {
                to_.props_[i]->solver().removePost(to_.props_[i]);
                delete to_.props_[i];
                to_.props_[i] = 0;
            }
    }
    
    virtual bool addPost(Clasp::Solver& s)
    {
        /// there can be some propagation in clasp::prepare
        /// here is the latest point to get the actual view
        if (!n_.getVariableCreator().restrictDomainsAccordingToLiterals())
            return false;
        //if (conf_.dlprop==2)
        //    if (!addDLProp(s,n_.constraints()))
        //        return false;

        
        if (to_.props_[s.id()])
        {
            s.removePost(to_.props_[s.id()]);
            delete to_.props_[s.id()];
            to_.props_[s.id()] = 0;
        }
        
        
        //std::vector<order::ReifiedLinearConstraint> constraints;
        //if (conf_.dlprop==1)
        //{
        //    constraints = n_.constraints();
        //}
        
        ///solver takes ownership of propagator
        clingcon::ClingconOrderPropagator* test = new clingcon::ClingconOrderPropagator(s, n_.getVariableCreator(), conf_,
                                                                                      n_.constraints(),n_.getEqualities(),
                                                                                      &(to_.names_));
        to_.props_[s.id()] = test;
        if (!s.addPost(to_.props_[s.id()]))
           return false;
        
//        if (conf_.dlprop==1)
//            if (!addDLProp(s, constraints))
//                return false;
        return true;
    }
    
private:
//    bool addDLProp(Clasp::Solver& s, const std::vector<order::ReifiedLinearConstraint>& constraints)
//    {
//        clingcon::ClingconDLPropagator* dlp = new clingcon::ClingconDLPropagator(s, conf_);
//        for (const auto&i : constraints)
//        {
//            if (dlp->isValidConstraint(i))
//                dlp->addValidConstraint(i);
//        }
//        if (!s.addPost(dlp))
//            return false;
//        return true;
//    }

    order::Config conf_;
    order::Normalizer& n_;
    TheoryOutput& to_;
    clingcon::ClingconOrderPropagator* cp_;
};


class Helper
{
public:
    Helper(Clasp::SharedContext& ctx, Clasp::Cli::ClaspCliConfig& claspConfig, Clasp::Asp::LogicProgram* lp, order::Config& conf);
    static void addOptions(ProgramOptions::OptionContext& root, order::Config& conf);

    void postRead();
    bool postEnd();
    void postSolve();

private:

    void simplifyMinimize();

    Clasp::SharedContext& ctx_;
    Potassco::TheoryData& td_;
    Clasp::Asp::LogicProgram* lp_;
    MySharedContext mctx_;
    std::unique_ptr<order::Normalizer> n_;
    order::Config conf_;
    TheoryOutput to_;
    Configurator configurator_;
    clingcon::TheoryParser tp_;



};

}

#endif // APPSUPPORT_HH

