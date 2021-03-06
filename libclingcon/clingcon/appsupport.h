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
#include <potassco/program_opts/program_options.h>
#include <potassco/program_opts/typed_value.h>

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

    bool first(const Clasp::Model& m, const char*& name, int32& value)
    {
        curr_ = names_.begin();
        currentSolverId_ = m.sId;
        return next(name,value);
    }

    bool next(const char*& name, int32& value)
    {
        while (curr_ != names_.end())
        {
            if (props_[currentSolverId_]->getValue(curr_->first,value))
            {
                name = curr_->second.first.c_str();
                ++curr_;
                return true;
            }
            ++curr_;
        }
        return false;
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
                to_.props_[i] = nullptr;
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
            to_.props_[s.id()] = nullptr;
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
    static void addOptions(Potassco::ProgramOptions::OptionContext& root, order::Config& conf);

    void postRead();
    bool postEnd();
    void postSolve();

    TheoryOutput* theoryOutput() { return &to_; }

private:

    /// checks if atoms occurs in some body of the logic program
    bool occursInBody(Clasp::Asp::LogicProgram& p, Potassco::Atom_t aId);
    void transformHeadConstraints(Clasp::Asp::PrgAtom *a);

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

    std::vector<order::Direction> tdinfo_;



};

}

#endif // APPSUPPORT_HH

