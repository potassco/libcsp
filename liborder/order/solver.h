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
#include <vector>
#include <ostream>
#include <cassert>
#include <order/platform.h>
#include <order/variable.h>

namespace order
{

//! A literal is a variable or its negation.
/*!
 * A literal is determined by two things: a sign and a variable index.
 * This class i copied and modified from clasp, to save conversion costs
 *
 * \par Implementation:
 * A literal's state is stored in a single 32-bit integer as follows:
 *  - 30-bits   : var-index
 *  - 1-bit     : sign, 1 if negative, 0 if positive
 *  - 1-bit     : called flag, keep track urself for whatever you want
 */
class Literal {
public:
    //! The default constructor creates the positive literal of the special sentinel var.
    //Literal() : rep_(0) { }

    //! Creates a literal of the variable var with sign s.
    /*!
     * \param var The literal's variable.
     * \param s true if new literal should be negative.
     */
    Literal(uint32_t var, bool sign) : rep_( (var<<2) + (uint32_t(sign)<<1) ) {
        assert( var < (1 << 30) );
    }

    //! Returns the unique index of this literal.
    /*!
     * \note The watch-flag is ignored and thus the index of a literal can be stored in 31-bits.
     */
    uint32_t index() const { return rep_ >> 1; }

    //! Creates a literal from an index.
    /*!
     * \pre idx < 2^31
     */
    static Literal fromIndex(uint32_t idx) {
        assert( idx < (uint32_t(1)<<31) );
        return Literal(idx<<1);
    }

    //! Creates a literal from an unsigned integer.
    static Literal fromRep(uint32_t rep) { return Literal(rep); }

    uint32_t& asUint()        { return rep_; }
    uint32_t  asUint() const  { return rep_; }

    //! Returns the variable of the literal.
    uint32_t var() const { return rep_ >> 2; }

    //! Returns the sign of the literal.
    /*!
     * \return true if the literal is negative. Otherwise false.
     */

    bool sign() const { return (rep_ & 2u) != 0; }

    void swap(Literal& other) { std::swap(rep_, other.rep_); }

    //! Sets the flag of this literal.
    void flag() { MyClasp::store_set_bit(rep_, 0); }

    //! Clears the flag of this literal.
    void clearFlag() { MyClasp::store_clear_bit(rep_, 0); }

    //! Returns true if the flag of this literal is set.
    bool flagged() const { return MyClasp::test_bit(rep_, 0); }

    //! Returns the complimentary literal of this literal.
    /*!
     *  The complementary Literal of a Literal is a Literal referring to the
     *  same variable but with inverted sign.
     */
    inline Literal operator~() const {
        return Literal( (rep_ ^ 2) & ~static_cast<uint32_t>(1u) );
    }

    //! Equality-Comparison for literals.
    /*!
     * Two Literals p and q are equal, iff
     * - they both refer to the same variable
     * - they have the same sign
     * .
     */
    inline bool operator==(const Literal& rhs) const {
        return index() == rhs.index();
    }
    inline bool operator!=(const Literal& rhs) const {
        return index() != rhs.index();
    }
    inline bool operator<(const Literal& rhs) const {
        return index() < rhs.index();
    }
private:
    Literal(uint32_t rep) : rep_(rep) {}
    uint32_t rep_;
};
/*class Literal
{
public:
    Literal(int i) : l(i) {}
    Literal(const Literal& l ) = default;
    bool operator==(const Literal& r) const { return l==r.l; }
    bool operator!=(const Literal& r) const { return l!=r.l; }
    bool operator<(const Literal& r) const { return l<r.l; }
    Literal operator~() const { return Literal(-l); }
    int getRep() const { return l; }
    void setRep(int i) { l=i; }
private:
    int l;
};*/

using LitVec = std::vector<Literal>;


class Solver
{
public:
    virtual bool isTrue(Literal) const = 0;
    virtual bool isFalse(Literal) const = 0;
    virtual bool isUnknown(Literal) const = 0;

    virtual Literal trueLit() const = 0;
    virtual Literal falseLit() const = 0;
};


class IncrementalSolver : public Solver
{
public:
    virtual Literal getNewLiteral() = 0;
};

class CreatingSolver : public Solver
{
public:
    
    /// preallocate a number of Literals
    /// for getNewLiteral to return
    virtual void createNewLiterals(uint64 num) = 0;

    /// returns a new literal
    /// pre: enough literals must have been created by createNewLiterals before
    virtual Literal getNewLiteral(bool frozen) = 0;

    virtual void freeze(Literal l) = 0;
    /// make all literals that have been created but not used false
    /// call this once preprocessing is finished
    virtual void makeRestFalse() = 0;

    virtual bool createClause(const LitVec &) = 0;

    /// create a simple cardinality constraint v =:= lb {lits}
    virtual bool createCardinality(Literal v, int lb, LitVec&& lits) = 0;

    /// called if the domain of var was not restricted
    virtual void unrestrictedDomainCallback(View var) const = 0;
    
    /// called if the domain of an intermediate var was not in Domain::min Domain::max (roughly int32)
    virtual void intermediateVariableOutOfRange() const = 0;
    

    virtual bool setEqual(const Literal& a, const Literal& b) = 0;
    
    
    virtual void addMinimize(order::Literal v, int32 weight, unsigned int level) = 0;
};



inline std::ostream& operator<< (std::ostream& stream, const Literal& l)
{
    stream << (l.sign() ? '-' : ' ') << l.var();
    return stream;
}

}
