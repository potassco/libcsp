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
