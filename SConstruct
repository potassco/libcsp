#!/usr/bin/python
# {{{ GPL License

# This file is part of gringo - a grounder for logic programs.
# Copyright (C) 2013  Roland Kaminski

# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.

# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.

# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

# }}}

from os import mkdir
from os.path import join, exists

if not exists("build"): mkdir("build")

AddOption('--build-dir', default='debug', metavar='DIR', nargs=1, type='string', dest='build_dir')
AddOption('--test-case', default=None, metavar='NAME', nargs=1, type='string', dest='test_case')

# Note: workaround for scons limitation (try to get a hand on the internal option parser)

opts_file = join("build", GetOption('build_dir') + ".py")

opts = Variables(opts_file, ARGUMENTS)
opts.AddVariables(
    ('CXX'           , 'C++ compiler'),
    ('CC'            , 'C compiler'),
    ('CXXFLAGS'      , 'C++ compiler flags'),
    ('CFLAGS'        , 'C compiler flags'),
    ('CPPPATH'       , 'include paths'),
    ('CPPDEFINES'    , 'defines'),
    ('LIBS'          , 'additional libraries'),
    ('LIBPATH'       , 'library paths'),
    ('LINKFLAGS'     , 'C++ linker flags'),
    ('CLINKFLAGS'    , 'C linker flags'),
    ('RPATH'         , 'library paths to embedd into binaries'),
    ('WITH_THREADS'  , 'enable thread support in clasp library; "posix", "windows", or None'),
    ('TESTS'         , 'enable specific unit tests; [liborder, libclingcon]'),
    )

env = Environment()
env['PKG_CONFIG']     = 'pkg-config'
env['CXX']            = 'c++'
env['CC']             = 'cc'
env['CXXFLAGS']       = ['-std=c++11', '-O0', '-g', '-Wall', '-W', '-pedantic']
env['CFLAGS']         = ['-O0', '-g', '-Wall']
env['LIBS']           = []
env['LINKFLAGS']      = ['-std=c++11', '-O0']
env['CLINKFLAGS']     = []
env['CPPDEFINES']     = {}
env['CPPPATH']        = []
env['LIBPATH']        = []
env['RPATH']          = []
env['WITH_THREADS']   = 'posix'
env['TESTS']          = ['liborder', 'libclingcon']

if GetOption("build_dir") == "static":
    env['CXXFLAGS'] = ['-std=c++11', '-O3', '-Wall']
    env['CFLAGS'] = ['-O3', '-Wall']
    env['LINKFLAGS'] = ['-std=c++11', '-O3', '-static']
    env['CPPDEFINES']['NDEBUG'] = 1
elif GetOption("build_dir") == "release":
    env['CXXFLAGS']   = ['-std=c++11', '-O3', '-Wall']
    env['CFLAGS'] = ['-O3', '-Wall']
    env['LINKFLAGS']  = ['-std=c++11', '-O3']
    env['CPPDEFINES']['NDEBUG'] = 1

opts.Update(env)
opts.Save(opts_file, env)
opts.FormatVariableHelpText = lambda env, opt, help, default, actual, other: "%10s: %s (%s)\n" % (opt, help, actual)

Help(
"""
usage: scons [OPTION] [TARGET] ...

Options:
  --build-dir=DIR             Sets the build directory to build/DIR. If DIR is
                              release or static then options are set,
                              respectively. Otherwise, debug options are set.
                              Default: debug
  --test-case=NAME            Selects which test case to run. If empty all
                              tests will be executed.
                              Default: ''

Targets:
  configure                   Only configure and build nothing.
  liborder                    Build gringo (built by default).
  libclingcon                 Build clingo (built by default).
  test-liborder               Run clingo specific acceptence tests.
  test-libclingcon            Run unit tests for libclingo.

Variables:
""" + opts.GenerateHelpText(env))

# Notes to use gold linker:
#   scons --build=gold \
#     CXXFLAGS="-std=c++11 -O4 -Wall" \
#     LINKFLAGS="-O4 -B build/gold/ld-gold/" \
#     RANLIB=true \
#     ARFLAGS="rc --plugin /usr/lib/llvm/LLVMgold.so"

if not env.GetOption('help'):
    SConscript('SConscript', variant_dir=join('build', GetOption('build_dir')), duplicate=0, exports=['env', 'opts'])

