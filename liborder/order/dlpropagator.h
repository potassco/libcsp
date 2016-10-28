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

#include <cassert>
#include <unordered_map>
#include <vector>
#include <algorithm>
#include <map>


template <class T>
inline void hash_combine(std::size_t & seed, const T & v)
{
  std::hash<T> hasher;
  seed ^= hasher(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
}

namespace difflogic
{

using uint64 = uint64_t;
using int64 = int64_t;

using uint32 = uint32_t;
using int32 = int32_t;




class DLPropagator
{
public:
    using Variable = unsigned int;
    using Weight = int;



    using EdgeId = int32;


    DLPropagator() : newVar_(0), currentLevel_(0) {}

    /// to create new indices,
    /// you are allowed to use your own indices,
    /// but do not mix them
    Variable newVar() { return newVar_++; }

    /// add a->b with weight, adds b->a with -weight-1
    /// returns a unique index for the first edge (index for the second is the negative one)
    /// for the variables, either use newVar to create them,
    /// or use your own indices (but keep the numbers low)
    //void addEdge(const Edge& e) { addEdge(e.in,e.weight,e.out); }
    EdgeId addEdge(Variable a, Weight weight, Variable b);

    /// pre: prepare must have been called
    /// pre: edge is unknown or already true (does nothing then)
    /// set's this edge to true, sets the opposite edge to false
    /// propagates all true/false edges
    /// returns a set of edges which have to be false
    std::vector<EdgeId> activate(EdgeId id);

    /// pre, prepare must have been called
    /// pre: edge is unknown or already false (does nothing then)
    /// set's this edge to false, sets the opposite edge to true
    /// (simply calls activate(b,-weight-1,a) )
    /// propagates all true/false edges
    /// returns a set of edges which have to be false
    std::vector<EdgeId> deactivate(EdgeId id);



    /// is not const because of intrinsic data structure
    std::vector<EdgeId> reason(EdgeId id);


    ///pre: number of activate/deactivate calls > undo calls
    /// undoes an activate/deactivate operation
    void undo();


    ///TODO: make it private
    int64 getPotential(Variable x) { return potential_[x]; }

    bool isTrue(EdgeId id) const;
    bool isFalse(EdgeId id) const;
    bool isUnknown(EdgeId e) const;


    /// debug function
    /// should always be false
    bool hasNegativeCycle() const;

private:


    struct InternalEdge
    {
        InternalEdge(Variable in, Weight weight, Variable out, int32 id) : in(in), weight(weight), out(out), id(id) {}
        Variable in;
        Weight weight;
        Variable out;
        int32 id;

        bool operator==(const InternalEdge& b) const
        {
            return in==b.in && out==b.out && weight==b.weight;
        }

        bool operator!=(const InternalEdge& b) const
        {
            return in!=b.in || out!=b.out || weight!=b.weight;
        }
    };

    struct Edge
    {
          Edge(Variable in, Weight weight, Variable out) : in(in), weight(weight), out(out) {}
          Variable in;
          Weight weight;
          Variable out;
    };

    struct HalfEdge
    {
        HalfEdge(Variable vertex, int weight, int32 id) : vertex(vertex), weight(weight), id (id) {}
        Variable vertex;
        Weight weight;
        int32 id;
    };

    Edge getEdge(EdgeId id) const { return id>0 ? edges_[id-1] : Edge(edges_[-id-1].out, -edges_[-id-1].weight-1, edges_[-id-1].in); }


    /// pre: prepare must have been called
    /// pre: edge is !unknown
    /// sets the edge to the value unknown again
    void undo(EdgeId id);

    void nonrec_activate(EdgeId id);

    unsigned int level(EdgeId id);


    /// returns a set if edges which is true
    std::vector<EdgeId> propagate(EdgeId id);

    using PosWeight = unsigned int;
    /// returns positive weight, adjusted by potential function
    PosWeight weight(Variable a, Weight weight, Variable b) const { assert(potential_[a]+(int64)weight-potential_[b] >= 0); return potential_[a]+(int64)weight-potential_[b]; }

    Variable newVar_; ///counter for creating new variables
public:
    struct Intrusive
    {
        Intrusive() : data1(0), data2(0), heapIndex(std::numeric_limits<unsigned int>::max()), visited(false) {}
        std::vector<HalfEdge> out;
        int64 data1;
        union
        {
        int64 data2; /// data can be used for anything
        };
        //int64 data2; /// data can be used for anything
        unsigned int heapIndex; /// primarily used as lookup where the Variable is stored in the heap, if it is stored (need to check again)
        bool visited;
    };
private:
    std::vector<Intrusive> outgoing_; /// outgoing edges of a variable (can be used as incoming with weight: -weight-1)
                                      /// after each function call, visited should be 0 and heapIndex=maxInt
    //std::vector<std::vector<HalfEdge>> incoming_; /// incoming edges of a variable

    //std::unordered_map<InternalEdge, int, Edgehash> truthTable_; /// stores the truthvalue of edges with weight >= 0 (truthvalue of edges with negative weight can be requested by asking for the opposing edge with weight: -weight-1)
                                                         /// a value > 0 is true, a value < 0 is false
                                                         /// abs(value) indicates some order of assignment, so Edge a was inserted before Edge b if |value(a)| < |value(b)|
                                                         /// an edge always depends only on other edges with strictly smaller level
                                                         /// there are always 2 levels, odd number for assigned from outside, even number for propagated by ourself
    std::vector<int> truthTable_;                        /// stores the truthvalue of edges with id (for edges with negative id, use positive id and invert result)
                                                         /// abs(value) indicates some order of assignment, so Edge a was inserted before Edge b if |value(a)| < |value(b)|
                                                         /// an edge always depends only on other edges with strictly smaller level
                                                         /// there are always 2 levels, odd number for assigned from outside, even number for propagated by ourself

    unsigned int currentLevel_;                          /// counter of truth levels
    std::vector<int64> potential_; /// for each Variable, this is the distance from a new Variable Q, where we have for all other Variables q->x with weight 0, should always be <=0
    std::vector<EdgeId> activityQueue_; /// all activated edges are added here, if adding edge A and propagating B,C,D, this queue contains A,B,C,D,0, last edge is seperator


    std::vector<Edge> edges_;


};


}

*/
