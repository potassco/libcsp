#pragma once

#include <order/config.h>


namespace order
{

//for testing
static Config lazySolveConfigProp1 = Config(true,10000,false,{3,1024},true,true,true,false,true,true,0,1000,1000,true,true, false,true,false, 1,true,std::make_pair(64,true));
static Config lazySolveConfigProp2 = Config(true,10000,false,{3,1024},true,true,true,false,true,true,0,1000,1000,true,true, false,true,false, 2,true,std::make_pair(64,false));
static Config lazySolveConfigProp3 = Config(true,10000,false,{3,1024},true,true,true,false,true,true,0,1000,1000,true,true, false,true,false, 3,true,std::make_pair(64,true));
static Config lazySolveConfigProp4 = Config(true,10000,false,{3,1024},true,true,true,false,true,true,0,1000,1000,true,true, false,true,false, 4,true,std::make_pair(64,false));
// actually not non lazy, just creates all literals, but no constraints are translated
static Config nonlazySolveConfig = Config(true,10000,false,{3,1024},true,true,true,false,true,true,0,0,-1,true,true, false,true,false, 4,true,std::make_pair(64,true));
static Config lazyDiffSolveConfig = Config(true,10000,false,{3,1024},true,true,true,false,true,true,0,0,1000,true,true, false,true,false, 4,true,std::make_pair(64,true));
static Config translateConfig = Config(true,10000,false,{3,1024},true,true,true,false,true,true,0,-1,1000,true,true, false,true,false, 4,true,std::make_pair(64,true));

static std::vector<Config> conf1({lazySolveConfigProp1,lazySolveConfigProp2,lazySolveConfigProp3,lazySolveConfigProp4,nonlazySolveConfig});
}
