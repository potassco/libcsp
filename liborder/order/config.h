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
#include <order/types.h>


namespace order
{

struct Config
{
public:
    Config() : strict(false) {}//= default;

    Config(bool redundantClauseCheck,
           unsigned int domSize, bool break_symmetries,
           std::pair<int64,int64> splitsize_maxClauseSize,
           bool pidgeon, bool permutation, bool disjoint2distinct,
           bool alldistinctCard, bool explicitBinaryOrderClauses,
           bool learnClauses, unsigned int dlprop,
           int64 translateConstraints, int64 minLitsPerVar,
           bool equalityProcessing, bool optimizeOptimize,
           bool coefFirst, bool descendCoef, bool descendDom,
           unsigned int propStrength, bool sortQueue,
           std::pair<unsigned int,bool> convertLazy) :
        redundantClauseCheck(redundantClauseCheck),
        domSize(domSize), break_symmetries(break_symmetries),
        splitsize_maxClauseSize(splitsize_maxClauseSize),
        pidgeon(pidgeon), permutation(permutation), disjoint2distinct(disjoint2distinct),
        alldistinctCard(alldistinctCard), explicitBinaryOrderClausesIfPossible(explicitBinaryOrderClauses),
        learnClauses(learnClauses), dlprop(dlprop),
        translateConstraints(translateConstraints),
        minLitsPerVar(minLitsPerVar), equalityProcessing(equalityProcessing),
        optimizeOptimize(optimizeOptimize),
        coefFirst(coefFirst), descendCoef(descendCoef), descendDom(descendDom),
        propStrength(propStrength), sortQueue(sortQueue),
        convertLazy(convertLazy), strict(false)
    {
        if (this->splitsize_maxClauseSize.first>=0)
            this->splitsize_maxClauseSize.first = std::max((int64)(3),this->splitsize_maxClauseSize.first);

    }
    //Config() : hallsize(0), redundantClauseCheck(true), domSize(10000), break_symmetries(false), splitsize_maxClauseSize{3,1024}, pidgeon(true), permutation(true),
    //           disjoint2distinct(true), alldistinctCard(false), explicitBinaryOrderClauses(true) {}

    bool redundantClauseCheck; /// activate check for redundant clauses while translating linear constraints
    int64 domSize; /// the maximum number of chunks a domain can have when multiplied (if avoidable)
    bool break_symmetries; /// necessary to avoid double solutions, can be set to false when only computing 1 solution or using projection
                     /// on the visible variables
    std::pair<int64,int64> splitsize_maxClauseSize; /// constraints are splitted into this size, if the expected number of clauses is larger then maxClauseSize
                            /// "minimum should be 3!
    //unsigned int maxClauseSize; /// see splitsize
    bool pidgeon; /// apply pidgeon hole optimization
    bool permutation; /// apply permutation constraint to alldifferent cosntraints
    bool disjoint2distinct; /// try to convert a disjoint constraint to an alldistinct constraint
    bool alldistinctCard; /// translate alldistinct with cardinality constraints
    bool explicitBinaryOrderClausesIfPossible; /// have the order clauses explicit or in a propagator
    /// be careful with explicitBinaryOrderClauses as it is not compatible with bool translate which has to be implemented yet
    bool learnClauses; /// learn clauses while propagating, default true
    unsigned int dlprop; /// 0 = no difference logic propagator, 1 =dl prop comes before linear order prop, 2 =dl prop comes after
    int64 translateConstraints; // translate constraint if expected number of clauses is less than this number (-1 = all)
    int64 minLitsPerVar; // precreate at least this number of literals per variable (-1 = all)
    bool equalityProcessing; // enable equality processing
    bool optimizeOptimize; /// replace single variables in optimize statement with their sum
    bool coefFirst; /// sort constraints by coefficient first
    bool descendCoef; /// sort constraints by decreasing coefficients
    bool descendDom; /// sort constraints by decreasing domain size
    unsigned int propStrength; /// propagation strength for lazy constraints 1..4
    bool sortQueue; /// sort the lazy propagation queue by constraint size (makes sense without splitting)
    std::pair<unsigned int,bool> convertLazy;
    bool strict; /// hidde option for testing strict/vs fwd/back inferences only
};


}
