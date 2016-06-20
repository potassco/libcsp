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

/*#include <order/dlpropagator.h>

#include <set>
#include <unordered_set>
#include <iostream>
#include <queue>
#include <limits>


namespace difflogic
{

template <class F>
class MinBinaryHeap
{
public:
    /// outgoing[index].data1 is used for comparison
    MinBinaryHeap(const F& cmp) : compare_(cmp) {}
    
    /// adds a node index to the heap, returns heap index
    void add(unsigned int index)    {
        vec.push_back(index);
        moveUp(vec.size()-1);
        assert(debug_integrity());
    }
    
    bool empty() { return vec.size()==0; }

    unsigned int getSmallest() const
    {
        assert(vec.size());
        return vec[0];
        assert(debug_integrity());
    }
    
    void popSmallest()
    {
        assert(vec.size());
        std::swap(vec[0],vec.back());
        vec.pop_back();
        moveDown(0);
        assert(debug_integrity());
    }
    
    
//    void popSmallest()
//    {
//        assert(vec.size());
    //DOES NOT WORK
//        vec[0] = std::numeric_limits<unsigned int>::max();
//        moveDown(0);
//        vec.pop_back();
//    }
    
    void moveDown(unsigned int heapIndex)
    {
        while(true)
        {
            int leftChild = 2*heapIndex + 1;
            int rightChild = 2*heapIndex + 2;
            
            if (leftChild>=vec.size())
                return;
            
            unsigned int newIndex = heapIndex;
            if (compare_(vec[leftChild],vec[heapIndex]))
                newIndex = leftChild;
            if (rightChild<vec.size() && compare_(vec[rightChild],vec[newIndex]))
                newIndex = rightChild;
            if (newIndex != heapIndex)
            {
                std::swap(vec[newIndex],vec[heapIndex]);
                heapIndex = newIndex;
            }
            else
            {
                assert(debug_integrity());
                return;
            }
         }
        assert(debug_integrity());
    }

    /// input is a heapindex, returns heapindex
    unsigned int moveUp(unsigned int heapIndex)
    {
        if (heapIndex==0)
            return 0;
        
        unsigned int parent = (heapIndex-1)/2;
        
        while(true)
        {
            if (heapIndex!=0 && !compare_(vec[parent],vec[heapIndex]))
            {
                std::swap(vec[parent],vec[heapIndex]);
                heapIndex = parent;
                parent = (heapIndex-1)/2;
            }
            else
                break;
        }
        assert(debug_integrity());
        return heapIndex;
    }

    /// should always return true
    bool debug_integrity() const
    {
        for (unsigned int i = 0; i < vec.size(); ++i)
        {
            if (i==0)
                return true;
            unsigned int parent = (i-1)/2;
            if (!compare_(vec[parent],vec[i]))
                return false;
        }
        return true;

    }
    
    std::vector<unsigned int> vec; /// indices to outgoing_
    const F& compare_;
};



///extended by isInside function which returns heapIndex for bubbling up
template <class F>
class ExtendedMinBinaryHeap
{
public:
    /// outgoing[index].data1 is used for comparison
    ExtendedMinBinaryHeap(const F& cmp, std::vector<DLPropagator::Intrusive>& outgoing) : compare_(cmp), outgoing_(outgoing) {}
    
    /// adds a node index to the heap, returns heap index
    void add(unsigned int index)
    {
        vec.push_back(index);
        assert(outgoing_[index].heapIndex==std::numeric_limits<unsigned int>::max());
        outgoing_[index].heapIndex = vec.size()-1;
        moveUp(vec.size()-1);
        assert(debug_integrity());
    }
    
    bool empty() { return vec.size()==0; }

    unsigned int getSmallest() const
    {
        assert(vec.size());
        return vec[0];
        assert(debug_integrity());
    }
    void popSmallest()
    {
        assert(vec.size());
        std::swap(outgoing_[vec[0]].heapIndex, outgoing_[vec.back()].heapIndex);
        std::swap(vec[0],vec.back());
        outgoing_[vec.back()].heapIndex=std::numeric_limits<unsigned int>::max();
        vec.pop_back();
        moveDown(0);
        assert(debug_integrity());
    }
    
    /// negative if not inside
    int isInside(unsigned int index)
    {
        if (outgoing_[index].heapIndex==std::numeric_limits<unsigned int>::max())
            return -1;
        else
        {   
            assert(outgoing_[index].heapIndex<vec.size() && vec[outgoing_[index].heapIndex]==index);
            return outgoing_[index].heapIndex;
        }
        assert(debug_integrity());
    }
    
    
    void moveDown(unsigned int heapIndex)
    {
        while(true)
        {
            int leftChild = 2*heapIndex + 1;
            int rightChild = 2*heapIndex + 2;
            
            if (leftChild>=vec.size())
                return;
            
            unsigned int newIndex = heapIndex;
            if (compare_(vec[leftChild],vec[heapIndex]))
                newIndex = leftChild;
            if (rightChild<vec.size() && compare_(vec[rightChild],vec[newIndex]))
                newIndex = rightChild;
            if (newIndex != heapIndex)
            {
                std::swap(outgoing_[vec[newIndex]].heapIndex, outgoing_[vec[heapIndex]].heapIndex);
                std::swap(vec[newIndex],vec[heapIndex]);
                heapIndex = newIndex;
            }
            else
            {
                assert(debug_integrity());
                return;
            }
         }
        assert(debug_integrity());
    }

    /// input is a heapindex, returns heapindex
    unsigned int moveUp(unsigned int heapIndex)
    {
        if (heapIndex==0)
            return 0;
        
        unsigned int parent = (heapIndex-1)/2;
        
        while(true)
        {
            if (heapIndex!=0 && !compare_(vec[parent],vec[heapIndex]))
            {
                std::swap(outgoing_[vec[parent]].heapIndex, outgoing_[vec[heapIndex]].heapIndex);
                std::swap(vec[parent],vec[heapIndex]);
                heapIndex = parent;
                parent = (heapIndex-1)/2;
            }
            else
                break;
        }
        assert(debug_integrity());
        return heapIndex;
    }

    /// should always return true
    bool debug_integrity() const
    {
        for (unsigned int i = 0; i < vec.size(); ++i)
        {
            if (i==0)
                return true;
            unsigned int parent = (i-1)/2;
            if (!compare_(vec[parent],vec[i]))
                return false;
            if (outgoing_[vec[i]].heapIndex!=i)
                return false;
        }
        return true;

    }
    
    std::vector<unsigned int> vec; /// indices to outgoing_
    const F& compare_;
    std::vector<DLPropagator::Intrusive>& outgoing_;
};



DLPropagator::EdgeId DLPropagator::addEdge(Variable a, Weight weight, Variable b)
{
    potential_.resize(std::max((unsigned int)(potential_.size()), std::max(a+1,b+1)));
    outgoing_.resize(std::max(b+1, std::max((Variable)(outgoing_.size()), a+1)));
    edges_.emplace_back(a,weight,b);
    outgoing_[a].out.emplace_back(b, weight,edges_.size());
    outgoing_[b].out.emplace_back(a, -weight-1, -edges_.size());
    truthTable_.resize(edges_.size()+1,0);
    return edges_.size();
}

std::vector<DLPropagator::EdgeId> DLPropagator::activate(EdgeId id)
{
    if (isTrue(id))
        return std::vector<DLPropagator::EdgeId>();
    activityQueue_.emplace_back(id);
    //assert(!hasNegativeCycle());
    ++currentLevel_;
    nonrec_activate(id);
    ++currentLevel_;
    auto r = propagate(id);
    for (const auto& i : r)
        activityQueue_.emplace_back(i);
    activityQueue_.emplace_back(0);
    //assert(!hasNegativeCycle());
    return r;
}

void DLPropagator::nonrec_activate(EdgeId id)
{
    assert(isUnknown(id));    

    if (id>0)
        truthTable_[id] = currentLevel_;
    else
        truthTable_[-id] = -(int32)(currentLevel_);
    
    Edge e = getEdge(id);
    unsigned int a = e.in;
    unsigned int b = e.out;
    Weight weight = e.weight;
    
    auto mycomp = [this](const uint64&a, const uint64& b) { return outgoing_[a].data1 < outgoing_[b].data1; };
    ExtendedMinBinaryHeap<decltype(mycomp)> changes(mycomp, outgoing_);
    int64 change = potential_[a]+(int64)weight - potential_[b];
    if (change >= 0)
        return;
    //changes.emplace(-change,std::make_pair(b,-(-int(potential_[a])+weight)));
    outgoing_[b].data1=change;
    outgoing_[b].data2=potential_[a]+int64(weight);
    changes.add(b);
    //changes.insert(std::make_pair(-change,std::make_pair(b,-(-int64(potential_[a])+int64(weight)))));
    
    while(!changes.empty())
    {    
        Variable var = changes.getSmallest();
        int64 newPot = outgoing_[var].data2;
        //std::cout << "doing smallest var " << var << std::endl;
        changes.popSmallest();
        
        if (potential_[var]> newPot)
        {
            potential_[var] = newPot;

            for (const auto& i : outgoing_[var].out)
            {
                if (isTrue(i.id))
                {
                    int newChange = potential_[var] + int64(i.weight) - potential_[i.vertex];
                    if (newChange<0)
                    {
                    //changes.emplace(-newChange, std::make_pair(i.vertex,-(-int(potential_[var]) + i.weight)));
                        //changes.insert(std::make_pair(-newChange, std::make_pair(i.vertex,-(-int64(potential_[var]) + int64(i.weight)))));
                        int heapIndex = changes.isInside(i.vertex);
                        if (heapIndex < 0)
                        {
                            outgoing_[i.vertex].data1=newChange;
                            outgoing_[i.vertex].data2=potential_[var] + int64(i.weight);
                            //std::cout << "adding var " << i.vertex << " with newchange " << newChange << " with edgeid " << i.id << " and weight " << i.weight << std::endl;
                            changes.add(i.vertex);
                        }
                        else
                        {
                            /// already inside, only change if less
                            if (outgoing_[i.vertex].data1 > newChange)
                            {
                                outgoing_[i.vertex].data1=newChange;
                                outgoing_[i.vertex].data2=potential_[var] + int64(i.weight);
                                //std::cout << "modifying var " << i.vertex << " with newchange " << newChange << " with edgeid " << i.id << " and weight " << i.weight<< std::endl;
                                changes.moveUp(heapIndex);
                            }
                        }


                    }
                        
                }
            }
        }
    }  
}

unsigned int DLPropagator::level(EdgeId id)
{
    assert(!isUnknown(id));
    return std::abs(truthTable_[abs(id)]);
}

std::vector<DLPropagator::EdgeId> DLPropagator::deactivate(EdgeId id)
{
    return activate(-id);
}


 std::vector<DLPropagator::EdgeId> DLPropagator::reason(EdgeId id)
 {
     std::vector<EdgeId> ret;
     std::vector<Variable> seen;
     auto mycomp = [this](const uint64&a, const uint64& b) { return outgoing_[a].data1 < outgoing_[b].data1; };
     ExtendedMinBinaryHeap<decltype(mycomp)> heap(mycomp, outgoing_);
     /// compute shortest path from a to b with only using true edges c
     /// level(c) < level(e)
     unsigned int l = level(id);
     Edge e = getEdge(id);

     outgoing_[e.in].data1=0;
     heap.add(e.in);
     seen.emplace_back(e.in);
     outgoing_[e.in].visited = true;
     
     while(!heap.empty())
     {
         auto x = heap.getSmallest();
         if (x==e.out)
         {
             while(x!=e.in)
             {

                 Edge e = getEdge(outgoing_[x].data2);
                 ret.emplace_back(outgoing_[x].data2);
                 x = e.in;
//                ret.emplace_back(InternalEdge(outgoing_[x].data2_2,outgoing_[x].data2_1,x));
//                x = outgoing_[x].data2_2;
                //ret.emplace_back(Edge(outgoing_[x].out[outgoing_[x].data2].vertex,-outgoing_[x].out[outgoing_[x].data2].weight-1,x));
                //x = outgoing_[x].out[outgoing_[x].data2].vertex;
             }
             break;
         }
         heap.popSmallest();
         for (unsigned int i = 0; i != outgoing_[x].out.size(); ++i)
         {
             
             const auto& out = outgoing_[x].out[i];
             if (isTrue(out.id) && level(out.id) < l)
             {
                 uint64 newWeight = outgoing_[x].data1+weight(x,out.weight,out.vertex);
                 int heapIndex = heap.isInside(out.vertex);
                 if (heapIndex < 0) /// not in heap
                 {
                     if (!outgoing_[out.vertex].visited)
                     {
                        outgoing_[out.vertex].data1=newWeight;
                        heap.add(out.vertex);
                        //unsigned int back = 0;
                        //REPLACE WITH UNION ?
                        //while(outgoing_[out.vertex].out[back].vertex!=x || outgoing_[out.vertex].out[back].weight!=-out.weight-1)
                        //    ++back;
                        //outgoing_[out.vertex].data2_1=back;// i do not need to put the i, but the way back
//                        outgoing_[out.vertex].data2_1=out.weight;
//                        outgoing_[out.vertex].data2_2=x;
                        outgoing_[out.vertex].data2=out.id;
                        seen.emplace_back(out.vertex);
                        outgoing_[out.vertex].visited = true;
                     }
                     ///else: it could have been there before -> im not allowed to overwrite the prev pointer
                 }
                 else
                 {
                     if (outgoing_[out.vertex].data1 > newWeight)
                     {
                        outgoing_[out.vertex].data1 = newWeight;
//                        outgoing_[out.vertex].data2_1=out.weight;
//                        outgoing_[out.vertex].data2_2=x;
                        outgoing_[out.vertex].data2=out.id;
                        heap.moveUp(heapIndex);
                     }
                 }   
             }
         }
     }
     
     for (const auto& i : seen)
     {
         outgoing_[i].heapIndex = std::numeric_limits<unsigned int>::max();
         outgoing_[i].visited = false;
     }
     
     return ret;
}



void DLPropagator::undo()
{
    assert(activityQueue_.back()==0);
    activityQueue_.pop_back();
    while(activityQueue_.back()!=0)
    {
        undo(activityQueue_.back());
        activityQueue_.pop_back();
    }
    --currentLevel_;
    --currentLevel_;
}

void DLPropagator::undo(EdgeId id)
{
    Edge e = getEdge(id);
    const Variable& b = e.out;
    
    assert(!isUnknown(id));
    truthTable_[abs(id)]=0;
    
    //auto mycomp = [](const uint64&a, const uint64& b) { return a > b; };
    //std::multimap<uint64,std::pair<Variable,uint64>, decltype(mycomp)> changes(mycomp); /// positive potential change -> (var/negative final potential)
    auto mycomp = [this](const uint64&a, const uint64& b) { return outgoing_[a].data1 < outgoing_[b].data1; };
    MinBinaryHeap<decltype(mycomp)> changes(mycomp);
    
    const auto& newShortest = [&](const Variable& b)
    {
        int64 ret = 0;
        for (const auto& i : outgoing_[b].out) /// incoming
        {
            if (isTrue(-i.id))
                ret = std::min(ret,potential_[i.vertex]+int64(-i.weight-1));
        }
        return ret;
    };
    
    
    /// shortest path's can only increase or stay the same
    int64 newPot = newShortest(b);
    
    if (potential_[b]>=newPot)
        return;
    int64 change = potential_[b]-newPot; /// increase by change
      
    outgoing_[b].data1 = change;
    outgoing_[b].data2 = newPot;
    changes.add(b);
    
    while(!changes.empty())
    {
        Variable var = changes.getSmallest();
        int64 newPot = outgoing_[var].data2;
        changes.popSmallest();
        
        if (potential_[var] <= newPot)
        {
            potential_[var] = newPot;
        
            for (const auto& i : outgoing_[var].out)
            {
                if (isTrue(i.id))
                {
                    int64 newPot = newShortest(i.vertex);
                    if (potential_[i.vertex] <= newPot) /// new distance is bigger
                    {
                        int64 newChange = newPot-potential_[i.vertex];
                        if (newChange>0)
                        {
                            outgoing_[i.vertex].data1=newChange;
                            outgoing_[i.vertex].data2=newPot;
                            changes.add(i.vertex);
                        }
                    }
                }
            }
        }   
    }
}


bool DLPropagator::isTrue(EdgeId id) const
{
    return (truthTable_[abs(id)] > 0 && id > 0) || (truthTable_[abs(id)] < 0 && id < 0);
//    if (e.weight>=0)
//        return truthTable_.find(e) != truthTable_.end() &&  (truthTable_.find(e)->second > 0);
//    else
//        return isFalse(InternalEdge(e.out, -e.weight-1, e.in));
}

bool DLPropagator::isFalse(EdgeId id) const
{
    return (truthTable_[abs(id)] < 0 && id > 0) || (truthTable_[abs(id)] > 0 && id < 0);
//    if (e.weight>=0)
//        return truthTable_.find(e) != truthTable_.end() && (truthTable_.find(e)->second < 0);
//    else
//        return isTrue(InternalEdge(e.out, -e.weight-1, e.in));  
}

bool DLPropagator::isUnknown(EdgeId id) const
{
    return truthTable_[abs(id)]==0;
}


std::vector<DLPropagator::EdgeId> DLPropagator::propagate(EdgeId id)
{
    ///TODO: think about a rewrite using the new EdgeId thingy
    Edge e = getEdge(id);
    Variable a = e.in;
    Variable b = e.out;
    Weight w = e.weight;
    
   // REWRITE using HEAP
   // REUSE pos/negRelevany, store posPotentialDistance in data1, posRealDistance in data2,
   //         afterwards negPotentialDistance in data1
    
    std::vector<Variable> seen;
    
    
    /// compute all "relevant" shortest path from a->
    std::unordered_set<Variable> posRelevancy; /// relevant variables in the positive graph
    //auto myposcomp = [&posRelevancy](const uint64&a, const uint64& b) { return a < b || (a==b && posRelevancy.find(a)==posRelevancy.end() && posRelevancy.find(b)!=posRelevancy.end()); };
    //std::multimap<uint64,Variable, decltype(myposcomp)> posQueue(myposcomp); /// potential distance from a to value == key, this is the queue
    //std::unordered_map<Variable,std::pair<uint64,int64>> posDistance; /// distance from a to Key, first: potential distance, second: real distance
    auto myposcomp = [this, &posRelevancy](const uint64&a, const uint64& b)
    {
        if (outgoing_[a].data1 < outgoing_[b].data1)
            return true;
        if (outgoing_[a].data1 == outgoing_[b].data1)
            return (posRelevancy.find(a)==posRelevancy.end() && posRelevancy.find(b)!=posRelevancy.end());
        return false;
    };
    ExtendedMinBinaryHeap<decltype(myposcomp)> posQueue(myposcomp, outgoing_);
    
    std::vector<EdgeId> ret;
    
    unsigned int posChecks = 0;
    outgoing_[a].data1=0;
    outgoing_[a].data2=0;
    outgoing_[a].visited=true;
    posQueue.add(a);
    //posQueue.emplace(0,a);
    //posDistance.emplace(a,std::make_pair(0,0));
    
    outgoing_[b].data1=weight(a,w,b);
    outgoing_[b].data2=w;
    outgoing_[b].visited=true;
    posQueue.add(b);
    //posQueue.emplace(weight(a,w,b),b);
    //posDistance.emplace(b,std::make_pair(weight(a,w,b),w));
    
    posRelevancy.insert(b);
    seen.emplace_back(a);
    seen.emplace_back(b);
    
    unsigned int numRelevantInQueue=1;
    while(numRelevantInQueue>0)
    {
        assert(!posQueue.empty());
        Variable x = posQueue.getSmallest();
        posQueue.popSmallest();
        
        int relevant = posRelevancy.count(x);
        
        if (relevant)
            --numRelevantInQueue;
            
        for (const auto&out : outgoing_[x].out)
        {
            Variable next = out.vertex;
            if (isTrue(out.id))
            {
                PosWeight w = weight(x,out.weight,next);
                int currentDistIndex = posQueue.isInside(next);
                //if (currentDist == posDistance.end() || currentDist->second.first>posDistance[x].first+w)
                if (currentDistIndex < 0)
                {
                    if (!outgoing_[next].visited)
                    {
                        /// add to heap
                        outgoing_[next].data1 = outgoing_[x].data1+w;
                        outgoing_[next].data2 = outgoing_[x].data2+out.weight;
                        posQueue.add(next);
                        outgoing_[next].visited = true;
                        seen.emplace_back(next);
                        /// propagate relevancy
                        if (posRelevancy.count(x))
                        {
                            if (posRelevancy.count(next)==0)
                            {
                                ++numRelevantInQueue;
                                posRelevancy.insert(next);
                            }
                        }
                        else
                            numRelevantInQueue-=posRelevancy.erase(next);
                    }
                    
                }
                else
                if (outgoing_[next].data1 > outgoing_[x].data1+w)
                {
                    // add to heap, this time only update heap
                    outgoing_[next].data1 = outgoing_[x].data1+w;
                    outgoing_[next].data2 = outgoing_[x].data2+out.weight;
                    posQueue.moveUp(currentDistIndex);
                    /// propagate relevancy
                    if (posRelevancy.count(x))
                    {
                        if (posRelevancy.count(next)==0)
                        {
                            ++numRelevantInQueue;
                            posRelevancy.insert(next);
                        }
                    }
                    else
                        numRelevantInQueue-=posRelevancy.erase(next);
                }
            }
            else
            {
                if (relevant && isUnknown(out.id))
                    ++posChecks;
            }
        }
    }

//    for (const auto& i : posRelevancy)
//    {
//        std::cout << "Reachable " << i << " with weight " << outgoing_[i].data2 << std::endl;
//    }

    std::vector<int64> tempPosWeights;  ///vector to store real weights of pos in order of posRelevancy
    for (const auto& i : posRelevancy)
        tempPosWeights.push_back(outgoing_[i].data2);
    
    for (const auto& i : seen)
    {
        /// cleaning up has to be done manually, as we do not necessarily clear the heap
        outgoing_[i].heapIndex=std::numeric_limits<unsigned int>::max();
        outgoing_[i].visited=false;
    }
    seen.clear();

    
    /// compute all relevant shortest path from <-b backwards
    std::unordered_set<Variable> negRelevancy; /// relevant variables in the positive graph
//    auto mynegcomp = [&negRelevancy](const uint64&a, const uint64& b) { return a < b || (a==b && negRelevancy.find(a)==negRelevancy.end() && negRelevancy.find(b)!=negRelevancy.end()); };
//    std::multimap<uint64,Variable, decltype(mynegcomp)> negQueue(mynegcomp); /// distance from a to value == key, this is the queue
//    std::unordered_map<Variable,std::pair<uint64,int64>> negDistance; /// distance from Key to b by going backwards through the edges, first: potential distance, second: real distance
    auto mynegcomp = [this, &negRelevancy](const uint64&a, const uint64& b)
    {
        if (outgoing_[a].data1 < outgoing_[b].data1)
            return true;
        if (outgoing_[a].data1 == outgoing_[b].data1)
           return (negRelevancy.find(a)==negRelevancy.end() && negRelevancy.find(b)!=negRelevancy.end());
        return false;
    };
    ExtendedMinBinaryHeap<decltype(mynegcomp)> negQueue(mynegcomp, outgoing_);
    
    
    unsigned int negChecks = 0;
    
    outgoing_[b].data1 = 0;
    outgoing_[b].data2 = 0;
    outgoing_[b].visited = true;
    negQueue.add(b);
    
    outgoing_[a].data1 = weight(a,w,b);
    outgoing_[a].data2 = w;
    outgoing_[a].visited = true;
    negQueue.add(a);
    
    seen.emplace_back(b);
    seen.emplace_back(a);
    
    
    negRelevancy.insert(a);
    
    numRelevantInQueue=1;
    while(numRelevantInQueue>0)
    {
        assert(!negQueue.empty());
        Variable x = negQueue.getSmallest();
        negQueue.popSmallest();
        
        int relevant = negRelevancy.count(x);
        if (relevant)
            --numRelevantInQueue;
            
        for (const auto&out : outgoing_[x].out) /// for all incoming arcs
        {
            Variable next = out.vertex;
            if (isTrue(-out.id))
            {
                PosWeight w = weight(next,-out.weight-1,x);
                int currentDistIndex = negQueue.isInside(next);
                if (currentDistIndex < 0)
                {
                    if (!outgoing_[next].visited)
                    {
                        /// add to heap
                        outgoing_[next].data1 = outgoing_[x].data1+w;
                        outgoing_[next].data2 = outgoing_[x].data2-out.weight-1;
                        outgoing_[next].visited = true;
                        negQueue.add(next);
                        seen.emplace_back(next);
                        /// propagate relevancy
                        if (negRelevancy.count(x))
                        {
                            if (negRelevancy.count(next)==0)
                            {
                                ++numRelevantInQueue;
                                negRelevancy.insert(next);
                            }
                        }
                        else
                            numRelevantInQueue-=negRelevancy.erase(next);
                    }
                    
                }
                else
                if (outgoing_[next].data1 > outgoing_[x].data1+w)
                {
                    // add to heap, this time only update heap
                    outgoing_[next].data1 = outgoing_[x].data1+w;
                    outgoing_[next].data2 = outgoing_[x].data2-out.weight-1;
                    negQueue.moveUp(currentDistIndex);
                    /// propagate relevancy
                    if (negRelevancy.count(x))
                    {
                        if (negRelevancy.count(next)==0)
                        {
                            ++numRelevantInQueue;
                            negRelevancy.insert(next);
                        }
                    }
                    else
                        numRelevantInQueue-=negRelevancy.erase(next);
                }
            }
            else
            {
                if (relevant && isUnknown(-out.id))
                    ++negChecks;
            }
        }
    }
    
    unsigned int k = 0;
    for (const auto& i : posRelevancy)
        outgoing_[i].data1 = tempPosWeights[k++];

//    for (const auto& i : negRelevancy)
//    {
//        std::cout << "Reachable " << i << " with weight " << outgoing_[i].data2 << std::endl;
//    }
    
    for (const auto& i : seen)
    {
        outgoing_[i].heapIndex = std::numeric_limits<unsigned int>::max();
        outgoing_[i].visited = false;
    }
    
    /// data1 is posRealDistance and data2 is negRealDistance now
    
        
    if (posChecks<negChecks)
    {
        for (const auto& i : posRelevancy)
        {
            for (const auto& out : outgoing_[i].out)
            {
                if (isUnknown(out.id))
                {
                    //std::cout << negDistance[out.vertex].second << " + " << posDistance[i].second << " - " << w << " + " <<   out.weight << " < " <<  0 << std::endl;
                    if (negRelevancy.find(out.vertex)!=negRelevancy.end() && outgoing_[i].data1 + outgoing_[out.vertex].data2 - w + out.weight < 0 )
                    {
                        //FOUND
                        nonrec_activate(-out.id);
                        ret.emplace_back(-out.id);
                    }
                }
            }
        }
    }
    else
    {
        for (const auto& i : negRelevancy)
        {
            for (const auto& out : outgoing_[i].out) /// incoming
            {
                if (isUnknown(-out.id))
                {
                    //std::cout << negDistance[i].second << " + " << posDistance[out.vertex].second << " - " << w << " + " << -out.weight-1 << " < " <<  0 << std::endl;
                    if (posRelevancy.find(out.vertex)!=negRelevancy.end() && (int64)(outgoing_[i].data2 + outgoing_[out.vertex].data1 - w + (-out.weight-1) < 0 ))
                    {
                        //FOUND
                        nonrec_activate(out.id);
                        ret.emplace_back(out.id);
                    }
                }
            }
        }
    }
    
    return ret;
}


bool DLPropagator::hasNegativeCycle() const
{
    if (edges_.size()==0)
        return false;

    std::set<Variable> variables;
    for (unsigned int index_e = 0; index_e != edges_.size(); ++index_e)
    {
        if (isTrue(index_e))
        {
            Edge e = getEdge(index_e);
            variables.insert(e.in);
            variables.insert(e.out);
        }
    }

    std::map<Variable,int64> distance;
    Variable start = *variables.begin();
    bool cont = false;
    do
    {
        distance[start] = 0;
        for (unsigned int count = 0; count < outgoing_.size()-1; ++count)
        {
            for (unsigned int index_e = 0; index_e != edges_.size(); ++index_e)
            {
                if (isTrue(index_e))
                {
                    Edge e = getEdge(index_e);

                    auto first = distance.find(e.in);
                    if (first!=distance.end())
                    {
                        auto second = distance.find(e.out);
                        if (second!=distance.end())
                        {
                            if (first->second + e.weight < second->second)
                                distance[e.out] = first->second + e.weight;
                        }
                        else
                        {
                            distance[e.out] = first->second + e.weight;
                        }
                    }
                }
            }
        }

        cont = false;
        for (const auto&i : variables)
            if (distance.find(i)==distance.end())
            {
                cont = true;
                start = i;
            }
    }while(cont);


    /// check
    for (unsigned int index_e = 0; index_e != edges_.size(); ++index_e)
    {
        if (isTrue(index_e))
        {
            Edge e = getEdge(index_e);
            auto first = distance.find(e.in);
            if (first!=distance.end())
            {
                auto second = distance.find(e.out);
                if (second!=distance.end())
                {
                    if (first->second + e.weight < second->second)
                        return true;
                }
                else
                {
                    assert(false); // every node should have a weight
                }
            }
            else
            {
                assert(false); // every node should have a weight
            }
        }
    }
    return false;
}

}

*/

