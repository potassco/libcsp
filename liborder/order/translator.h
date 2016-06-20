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
#include <cstddef>
#include <limits>
#include <order/linearpropagator.h>
#include <order/config.h>



namespace order
{

class Translator
{
public:
    Translator(CreatingSolver& s, const Config& conf) : s_(s), conf_(conf)
    {}

    /// translate the constraint
    /// returns false if addclause fails
    bool doTranslate(VariableCreator &vc, const ReifiedLinearConstraint& l);


private:


    bool doTranslateImplication(VariableCreator &vc, Literal l, const LinearConstraint& c);
    CreatingSolver& s_;
    const Config& conf_;
};

inline bool translate(CreatingSolver& s, VariableCreator& vc, std::vector<ReifiedLinearConstraint>& rl, const Config& conf)
{
    Translator t(s, conf);

    uint64 size = conf.translateConstraints == -1 ? std::numeric_limits<uint64>::max() : conf.translateConstraints;


    unsigned int num = rl.size();
    for (unsigned int i = 0; i < num;)
    {
        if (rl[i].l.productOfDomainsExceptLastLEx(vc,size))
        {
            if (!t.doTranslate(vc,rl[i]))
                return false;
            std::swap(rl[i],*(rl.begin()+num-1));
            --num;
        }
        else
            ++i;
    }
    rl.erase(rl.begin()+num,rl.end());
    return true;
}


}
