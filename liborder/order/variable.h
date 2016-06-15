#pragma once
#include "types.h"
#include <tuple>
#include <cassert>
#include <cmath>
#include <limits>
//#include <cstddef>


namespace order
{
/// change to 32bit to safe some space
using Variable = unsigned int;
//const Variable InvalidVar = std::numeric_limits<unsigned int>::max();
const Variable InvalidVar = 4294967295;


/// represents a View on a Variable  which is equal to  (a*v)/b  + c
struct View
{
    View(Variable v=0, int32 a=1, int32 c = 0) : v(v), a(a), c(c) {}
    Variable v;
    int32 a;
    int32 c;
    
    /// a*x + c = rhs
    /// returns a given rhs
    int64 divide(int64 rhs) const
    {
        return reversed() ? std::ceil((double)(rhs-c)/a) : std::floor((double)(rhs-c)/a);
    }
    
    /// return rhs given a
    int64 multiply(int64 x) const
    {
        return (int64)(a) * (int64)(x) + (int64)(c);
    }
    
    bool reversed() const { return a<0; }

    inline View& operator *=(int32 x) { a*=x; c*=x; return *this; }
    inline View& operator +=(int32 x) { c+=x; return *this; }
    
    
};

inline bool operator<(const View& v1, const View& v2)  { return std::tie(v1.v,v1.a,v1.c) < std::tie(v2.v,v2.a,v2.c); }
inline bool operator==(const View& v1, const View& v2)  { return std::tie(v1.v,v1.a,v1.c) == std::tie(v2.v,v2.a,v2.c); }
inline bool operator!=(const View& v1, const View& v2)  { return std::tie(v1.v,v1.a,v1.c) != std::tie(v2.v,v2.a,v2.c); }
inline View operator*(const View& v1, int32 x) {return View(v1.v,v1.a*x,v1.c*x); }
inline View operator+(const View& v1, int32 x) {return View(v1.v,v1.a,v1.c+x); }
}
