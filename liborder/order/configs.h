// {{{ MIT License

// Copyright 2017 Max Ostrowski

// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.

// }}}

#pragma once

#include <order/config.h>


namespace order
{

//for testing
static Config lazySolveConfigProp1 = Config(true,10000,false,{3,1024},true,true,true,false,true,true,0,1000,1000,true,true, false,true,false, 1,true,std::make_pair(64,true),false);
static Config lazySolveConfigProp2 = Config(true,10000,false,{3,1024},true,true,true,false,true,true,0,1000,1000,true,true, false,true,false, 2,true,std::make_pair(64,false),false);
static Config lazySolveConfigProp3 = Config(true,10000,false,{3,1024},true,true,true,false,true,true,0,1000,1000,true,true, false,true,false, 3,true,std::make_pair(64,true),true);
static Config lazySolveConfigProp4 = Config(true,10000,false,{3,1024},true,true,true,false,true,true,0,1000,1000,true,true, false,true,false, 4,true,std::make_pair(64,false),true);
// actually not non lazy, just creates all literals, but no constraints are translated
static Config nonlazySolveConfig = Config(true,10000,false,{3,1024},true,true,true,false,true,true,0,0,-1,true,true, false,true,false, 4,true,std::make_pair(64,true),false);
static Config lazyDiffSolveConfig = Config(true,10000,false,{3,1024},true,true,true,false,true,true,0,0,1000,true,true, false,true,false, 4,true,std::make_pair(64,true),false);
static Config translateConfig = Config(true,10000,false,{3,1024},true,true,true,false,true,true,0,-1,1000,true,true, false,true,false, 4,true,std::make_pair(64,true),true);

static std::vector<Config> conf1({lazySolveConfigProp1,lazySolveConfigProp2,lazySolveConfigProp3,lazySolveConfigProp4,nonlazySolveConfig});
}
