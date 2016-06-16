#include <clingcon/appsupport.h>
#include <program_opts/program_options.h>
#include <program_opts/typed_value.h>


namespace clingcon
{

Helper::Helper(Clasp::SharedContext& ctx, Clasp::Cli::ClaspCliConfig& claspConfig, Clasp::Asp::LogicProgram* lp, order::Config& conf) : ctx_(ctx), td_(lp->theoryData()),
                                                                          lp_(lp), mctx_(ctx), n_(new order::Normalizer(mctx_,conf_)),
                                                                          conf_(conf), configurator_(conf_,*n_.get(),to_),
                                                                          tp_(*n_.get(),td_,lp,mctx_.trueLit())
{
    claspConfig.addConfigurator(&configurator_,Clasp::Ownership_t::Type::Retain, false);

}

void Helper::addOptions(ProgramOptions::OptionContext& root, order::Config& conf)
{
    ProgramOptions::OptionGroup cspconf("Constraint Processing Options");
    cspconf.addOptions()
            ("redundant-nogood-check", ProgramOptions::storeTo(conf.redundantClauseCheck = true), "Check translated nogoods for redundancies (default: true)")
            ("domain-size", ProgramOptions::storeTo(conf.domSize = 10000), "Restrict the number of ranges a domain can have when multiplied (-1=all) (default: 10000)")
            ("break-symmetries", ProgramOptions::storeTo(conf.break_symmetries = true), "Break symmetries (necessary for enumeration) (default: true)")
            ("split-size", ProgramOptions::storeTo(conf.splitsize_maxClauseSize.first = 3)->arg("<n>"), "Split constraints into size %A (minimum: 3, -1=all) (default: 3)")
            ("max-nogoods-size", ProgramOptions::storeTo(conf.splitsize_maxClauseSize.second = 1024)->arg("<n>"), "Constraints are only split if they would produce more then %A nogoods (default: 1024)")
            ("pidgeon-optimization", ProgramOptions::storeTo(conf.pidgeon = true), "Add pidgeon-hole constraints for distinct (default: true)")
            ("permutation-optimization", ProgramOptions::storeTo(conf.permutation = false), "Add permutation constraints for distinct (default: false)")
            ("distinct-to-card", ProgramOptions::storeTo(conf.alldistinctCard = true), "Translate distinct constraint using cardinality constraints (default: true)")
            ("explicit-binary-order", ProgramOptions::storeTo(conf.explicitBinaryOrderClausesIfPossible = false), "Create binary order nogoods if possible (default: false)")
            ("learn-nogoods", ProgramOptions::storeTo(conf.learnClauses = true), "Learn nogoods while propagating (default: true)")
            ("translate-constraints", ProgramOptions::storeTo(conf.translateConstraints = 1000)->arg("<n>"), "Translate constraints with an estimated number of nogoods less than %A (-1=all) (default: 1000)")
            ("min-lits-per-var", ProgramOptions::storeTo(conf.minLitsPerVar = 1000)->arg("<n>"), "Creates at least %A literals per variable (-1=all) (default: 1000)")
            ("equality-processing", ProgramOptions::storeTo(conf.equalityProcessing = true), "Replace equal variable views (default: true)")
            ("flatten-optimization", ProgramOptions::storeTo(conf.optimizeOptimize = false), "Flatten the optimization statement (default: false)")
            ("sort-coefficient", ProgramOptions::storeTo(conf.coefFirst = false), "Sort constraints by coefficient first (otherwise domain size) (default: false)")
            ("sort-descend-coef", ProgramOptions::storeTo(conf.descendCoef = true), "Sort constraints by descending coefficients (otherwise ascending) (default: true)")
            ("sort-descend-dom", ProgramOptions::storeTo(conf.descendDom = false), "Sort constraints by descending domain size (otherwise ascending) (default: false)")
            ("prop-strength", ProgramOptions::storeTo(conf.propStrength = 4)->arg("<n>"), "Propagation strength %A {1=weak .. 4=strong} (default: 4)")
            ("sort-queue", ProgramOptions::storeTo(conf.sortQueue = false), "Sort lazy propagation queue by constraint size (default: false)")
            ("convert-lazy-variables", ProgramOptions::storeTo(conf.convertLazy = std::make_pair(0,false))->arg("<n,b>"), "Add the union(b=true)/intersection(b=false) of the lazy variables of the first n threads (default: 0,false)")
            ;
    root.add(cspconf);

    ProgramOptions::OptionGroup hidden("Constraint Processing Options 2",ProgramOptions::desc_level_hidden);
    cspconf.addOptions()
            //("difference-logic", ProgramOptions::storeTo(conf.dlprop = 0), "0: no difference logic propagator, 1 early, 2 late  (default: 0)")
            ("disjoint-to-distinct", ProgramOptions::storeTo(conf.disjoint2distinct = false), "Translate disjoint to distinct constraint if possible (default: false)")
            ;
    root.add(hidden);
}


void Helper::postRead()
{
    for (auto i = td_.currBegin(); i != td_.end(); ++i)
        if ((*i)->atom()!=0)
            lp_->startChoiceRule().addHead((*i)->atom()).endRule();
}

bool Helper::postEnd()
{
    bool conflict = false;
    conflict = !ctx_.master()->propagate();
    if (!conflict)
    {

        for (auto i = td_.currBegin(); i != td_.end(); ++i)
        {
            if (!tp_.readConstraint(i))
                throw std::runtime_error("Unknown theory atom detected, cowardly refusing to continue");
        }
        to_.names_ = tp_.postProcess();
        ctx_.output.theory = &to_;
        simplifyMinimize();
        conflict = !n_->prepare();
    }


    if (!conflict)
    {
        do
        {
            conflict = !ctx_.master()->propagate();
            if (!conflict)
                conflict = !n_->propagate();
        }while(!conflict && !n_->atFixPoint());
    }

    if (!conflict)
        conflict = !n_->finalize();

    if (conflict && !ctx_.master()->hasConflict())
        ctx_.master()->force(Clasp::Literal(0,true));

     tp_.reset();
     return conflict;
}

void Helper::postSolve()
{
    std::vector<const order::VolatileVariableStorage*> vvs;
    for (unsigned int thread = 0; thread < std::min(conf_.convertLazy.first,64u); ++thread)
    {
        if (to_.props_[thread]==nullptr)
            break;
        vvs.emplace_back(&to_.props_[thread]->getVVS());
    }

    n_->convertAuxLiterals(vvs, ctx_.numVars());
}


void Helper::simplifyMinimize()
{
    for (unsigned int level = 0; level < tp_.minimize().size(); ++level)
        for (auto i : tp_.minimize()[level])
        {
            std::vector<order::View> mini;
            mini.emplace_back(i.second);
            if (n_->getConfig().optimizeOptimize) // optimize away equality and minimize variable
            {
                if (abs(i.second.a)==1)
                for (auto rlin = n_->linearConstraints_.begin(); rlin != n_->linearConstraints_.end(); ++rlin)
                {
                    if (mctx_.isTrue(rlin->v) && rlin->l.getRelation()==order::LinearConstraint::Relation::EQ && rlin->l.getConstViews().size()>1)
                    {
                        auto& views = rlin->l.getConstViews();
                        bool hit = false;
                        for (unsigned int j = 0; j <= views.size(); ++j)
                        {
                            if (views[j].v == i.second.v && views[j].a == -1)
                            {
                                hit = true;

                                order::LinearConstraint lc = rlin->l;
                                auto& newviews = lc.getViews();
                                newviews.erase(newviews.begin()+j);
                                lc.times(i.second.a);
                                mini.clear();
                                mini = std::move(newviews);
                                mini.front().c -= rlin->l.getRhs();
                                break;
                            }
                        }
                        if (hit)
                        {
                            //n.linearConstraints_.erase(rlin);
                            break;
                        }
                    }
                }

            }
            for (auto& j : mini)
                n_->addMinimize(j,level);
        }
}

}
