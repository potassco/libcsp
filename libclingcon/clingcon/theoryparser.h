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
#include <clingcon/solver.h>
#include <order/normalizer.h>
#include <potassco/theory_data.h>
#include <order/constraint.h>
#include <unordered_map>
#include <sstream>



namespace clingcon
{

template <class T>
inline void hash_combine(std::size_t & seed, const T & v)
{
  std::hash<T> hasher;
  seed ^= hasher(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
}


using NameList = std::unordered_map<order::Variable,std::pair<std::string,Clasp::LitVec>>;


class TheoryParser
{
public:

    enum CType {SUM, DOM, DISTINCT, SHOW, MINIMIZE};
    using mytuple = std::vector<Potassco::Id_t>;   /// a tuple identifier
    using tuple2View = std::map<mytuple, order::View>; // could be unordered
    

    TheoryParser(order::Normalizer& n, Potassco::TheoryData& td, Clasp::Asp::LogicProgram* lp, order::Literal trueLit) :
        n_(n), td_(td), lp_(lp), trueLit_(trueLit)
    {}
    bool isClingconConstraint(Potassco::TheoryData::atom_iterator& i);
    bool isUnarySum(Potassco::TheoryData::atom_iterator& i);

    /// returns false, if not a constraint of this theory
    /// throws string with error if error occurs
    /// save constraint as strict or nonstrict
    bool readConstraint(Potassco::TheoryData::atom_iterator& i, order::Direction dir);
    /// turn show predicates to variables
    NameList& postProcess();
    const std::vector<tuple2View>& minimize() const;
    
    void reset();

private:

    void error(const std::string& s);
    void error(const std::string& s, Potassco::Id_t id);


    bool getConstraintType(Potassco::Id_t id, CType& t);
    bool getGuard(Potassco::Id_t id, order::LinearConstraint::Relation& rel);


    std::string toString(const Potassco::TheoryData::Term& t)
    {
        std::stringstream ss;
        return toString(ss,t).str();
    }

    std::stringstream& toString(std::stringstream& ss, const Potassco::TheoryData::Term& t);

    bool isNumber(Potassco::Id_t id);

    int getNumber(Potassco::Id_t id);
    
    void add2Shown(order::Variable v, uint32 tid, Clasp::Literal l);

    ///
    /// \brief getView
    /// \param id
    /// \return
    /// either number -> create new var
    /// string -> create var
    /// tuple -> not allowed
    /// function
    ///        named function -> eval and create var
    ///        operator + unary ->getView of Rest
    ///        operator - unary ->getView of Rest
    ///        operator + binary -> one is number, other getView or both is number -> create Var
    ///        operator - binary -> one is number, other getView or both is number -> create Var
    ///        operator * binary -> one is number, other getView or both is number -> create Var
    ///
    bool getView(Potassco::Id_t id, order::View& v);

    order::View createVar(Potassco::Id_t id);

    order::View createVar(Potassco::Id_t id, int32 val);

    
private:
    
    bool check(Potassco::Id_t id);
    bool isVariable(Potassco::Id_t id);
    

    std::unordered_map<Potassco::Id_t, std::pair<CType,bool>>  termId2constraint_;
    std::unordered_map<Potassco::Id_t, order::LinearConstraint::Relation>  termId2guard_;
    std::vector<std::pair<Potassco::Id_t,Clasp::Literal>> shown_; /// order::Variable to TermId + condition literal
    std::vector<std::pair<Potassco::Id_t,Clasp::Literal>> shownPred_; /// a list of p/3 predicates to be shown + condition literal
    std::vector<tuple2View> minimize_;                /// for each level
    
    std::vector<order::View>  termId2View_;
    order::Normalizer& n_;
    Potassco::TheoryData& td_;
    Clasp::Asp::LogicProgram* lp_;
    order::Literal trueLit_;
    const Potassco::Id_t MAXID = std::numeric_limits<Potassco::Id_t>::max();
    std::unordered_map<std::string, order::View> string2view_;
    using Predicate = std::pair<std::string,unsigned int>;

    struct PredicateHasher
    {
        std::size_t operator()(Predicate const& p) const
        {
        std::size_t seed = 0;
        hash_combine(seed, p.first);
        hash_combine(seed, p.second);
        return seed;
        }
    };

    std::unordered_map<Predicate,std::set<order::Variable>,PredicateHasher> pred2Variables_;
    std::unordered_map<order::Variable,std::pair<std::string,Clasp::LitVec>> orderVar2nameAndConditions_;
    std::unordered_map<Predicate,Clasp::LitVec,PredicateHasher> shownPredPerm_;
};

}
