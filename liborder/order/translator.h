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
