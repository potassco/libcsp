#!/usr/bin/python
# {{{1 GPL License

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

# {{{1 Preamble

import os
import types
from os.path import join

# {{{1 Auxiliary functions

def find_files(env, path):
    oldcwd = os.getcwd()
    try:
        os.chdir(Dir('#').abspath)
        sources = []
        for root, dirnames, filenames in os.walk(path):
            for filename in filenames:
                if filename.endswith(".c") or filename.endswith(".cc") or filename.endswith(".cpp"):
                    sources.append(os.path.join(root, filename))
                if filename.endswith(".yy"):
                    target = os.path.join(root, filename[:-3],  "grammar.cc")
                    source = "#"+os.path.join(root, filename)
                    sources.append(target)
                    env.Bison(target, source)
                if filename.endswith(".xh"):
                    target = os.path.join(root, filename[:-3] + ".hh")
                    source = "#"+os.path.join(root, filename)
                    env.Re2c(target, source)
                if filename.endswith(".xch"):
                    target = os.path.join(root, filename[:-4] + ".hh")
                    source = "#"+os.path.join(root, filename)
                    env.Re2cCond(target, source)
        return sources
    finally:
        os.chdir(oldcwd)

def shared(env, sources):
    return [env.SharedObject(x) for x in sources]

def CheckMyFun(context, name, code, header):
    source = header + "\nint main() {\n" + code + "\nreturn 0; }"
    context.Message('Checking for C++ function ' + name + '()... ')
    result = context.TryLink(source, '.cc')
    context.Result(result)
    return result

def CheckLibs(context, name, libs, header):
    context.Message("Checking for C++ library {0}... ".format(name))
    libs = [libs] if isinstance(libs, types.StringTypes) else libs
    old = context.env["LIBS"][:]
    for lib in libs:
        if os.path.isabs(lib):
            context.env.Append(LIBS=File(lib))
        else:
            context.env.Append(LIBS=lib)
    result = context.TryLink("#include <{0}>\nint main() {{ }}\n".format(header), '.cc')
    context.Result(result)
    if result == 0:
        context.env["LIBS"] = old
    return result

def CheckWithPkgConfig(context, name, versions):
    context.Message("Auto-detecting {0} ({1})... ".format(name, context.env["PKG_CONFIG"]))
    result = False
    if context.env['PKG_CONFIG'] is not None and \
       context.TryAction('${PKG_CONFIG} --atleast-pkgconfig-version=0')[0]:
        for version in versions:
            if context.TryAction('${{PKG_CONFIG}} --exists "{0}"'.format(version))[0]:
                context.env.ParseConfig('${{PKG_CONFIG}} --cflags --libs {0}'.format(version))
                result = True
                break
    context.Result(result)
    return result

# {{{1 Basic environment

Import('env')

base_env = env.Clone()

env['ENV']['PATH'] = os.environ['PATH']
if 'LD_LIBRARY_PATH' in os.environ: env['ENV']['LD_LIBRARY_PATH'] = os.environ['LD_LIBRARY_PATH']

# {{{1 Gringo specific configuration

log_file = join("build", GetOption('build_dir') + ".log")
conf = Configure(env, custom_tests={'CheckMyFun' : CheckMyFun, 'CheckLibs' : CheckLibs, 'CheckWithPkgConfig' : CheckWithPkgConfig}, log_file=log_file)
DEFS = {}
failure = False

if not conf.CheckCXX():
    print 'error: no usable C++ compiler found'
    print "Please check the log file for further information: " + log_file
    Exit(1)

if not conf.CheckSHCXX():
    print 'error: no usable (shared) C++ compiler found'
    print "Please check the log file for further information: " + log_file
    Exit(1)

if not conf.CheckMyFun('snprintf', 'char buf[256]; snprintf (buf,256,"");', '#include <cstdio>'):
    if conf.CheckMyFun('__builtin_snprintf', 'char buf[256]; __builtin_snprintf (buf,256,"");', '#include <cstdio>'):
        DEFS['snprintf']='__builtin_snprintf'

if not conf.CheckMyFun('vsnprintf', 'char buf[256]; va_list args; vsnprintf (buf,256,"", args);', "#include <cstdio>\n#include <cstdarg>"):
    if conf.CheckMyFun('__builtin_vsnprintf', 'char buf[256]; va_list args; __builtin_vsnprintf (buf,256,"", args);', "#include <cstdio>\n#include <cstdarg>"):
        DEFS['vsnprintf']='__builtin_vsnprintf'

if not conf.CheckMyFun('std::to_string', 'std::to_string(10);', "#include <string>"):
    DEFS['MISSING_STD_TO_STRING']=1


env = conf.Finish()
env.PrependUnique(LIBPATH=[Dir('.')])
env.Append(CPPDEFINES=DEFS)

# {{{1 Clasp specific configuration

claspEnv  = env.Clone()
claspConf = Configure(claspEnv, custom_tests = {'CheckLibs' : CheckLibs, 'CheckWithPkgConfig' : CheckWithPkgConfig}, log_file = join("build", GetOption('build_dir') + ".log"))
DEFS = {}

DEFS["WITH_THREADS"] = 0
if env['WITH_THREADS'] is not None:
    DEFS["WITH_THREADS"] = 1
    DEFS["CLASP_USE_STD_THREAD"] = 1
    if env['WITH_THREADS'] == "posix":
        # Note: configuration differs between gcc and clang here
        #       gcc needs -pthread, clang needs -lpthread
        claspConf.env.Append(CPPFLAGS=["-pthread"])
        claspConf.env.Append(LIBS=["pthread"])
    elif env['WITH_THREADS'] == "windows":
        pass # nohing to do
    else:
        print 'error: unknown thread model'
        failure = True


claspEnv = claspConf.Finish()
claspEnv.Append(CPPDEFINES=DEFS)

# {{{1 Check configuration

if failure:
    print "Please check the log file for further information: " + log_file
    Exit(1)

# {{{1 Opts: Library

LIBOPTS_SOURCES = find_files(env, 'libprogram_opts/src')
LIBOPTS_HEADERS = [Dir('#libprogram_opts'), Dir('#libprogram_opts/src')]

optsEnv = env.Clone()
optsEnv.Append(CPPPATH = LIBOPTS_HEADERS)

optsLib  = optsEnv.StaticLibrary('libprogram_opts', LIBOPTS_SOURCES)
optsLibS = optsEnv.StaticLibrary('libprogram_opts_shared', shared(optsEnv, LIBOPTS_SOURCES))

# {{{1 Lp: Library

LIBLP_SOURCES = find_files(env, 'liblp/src')
LIBLP_HEADERS = [Dir('#liblp'), Dir('#liblp/src')]

lpEnv = env.Clone()
lpEnv.Append(CPPPATH = LIBLP_HEADERS)

lpLib  = lpEnv.StaticLibrary('liblp', LIBLP_SOURCES)
lpLibS = lpEnv.StaticLibrary('liblp_shared', shared(lpEnv, LIBLP_SOURCES))

# {{{1 Clasp: Library

LIBCLASP_SOURCES = find_files(env, 'libclasp/src')
LIBCLASP_HEADERS = [Dir('#libclasp'), Dir('#libclasp/src')] + LIBOPTS_HEADERS + LIBLP_HEADERS

claspEnv.Append(CPPPATH = LIBCLASP_HEADERS)

claspLib  = claspEnv.StaticLibrary('libclasp', LIBCLASP_SOURCES)
claspLibS = claspEnv.StaticLibrary('libclasp_shared', shared(claspEnv, LIBCLASP_SOURCES))

# {{{1 order: Library

LIBORDER_SOURCES = find_files(env, 'liborder/src')
LIBORDER_HEADERS = [Dir('#liborder'), 'liborder/src'] + LIBLP_HEADERS

orderEnv = env.Clone()
orderEnv.Append(CPPPATH = LIBORDER_HEADERS)

orderLib  = orderEnv.StaticLibrary('liborder', LIBORDER_SOURCES)
orderLibS = orderEnv.StaticLibrary('liborder_shared', shared(orderEnv, LIBORDER_SOURCES))

# {{{1 Clingcon: Library

LIBCLINGCON_SOURCES = find_files(env, 'libclingcon/src')
LIBCLINGCON_HEADERS = [Dir('#libclingcon')] + LIBORDER_HEADERS + LIBCLASP_HEADERS + LIBLP_HEADERS

clingconEnv = claspEnv.Clone()
clingconEnv.Append(CPPPATH = LIBCLINGCON_HEADERS)

clingconLib  = clingconEnv.StaticLibrary('libclingcon', LIBCLINGCON_SOURCES)
clingconLibS = clingconEnv.StaticLibrary('libclingcon_shared', shared(clingconEnv, LIBCLINGCON_SOURCES))

clingconSharedEnv = clingconEnv.Clone()
clingconSharedEnv.Prepend(LIBS = [orderLibS, claspLibS, lpLibS])
clingconSharedLib = clingconSharedEnv.SharedLibrary('libclingcon', shared(clingconEnv, LIBCLINGCON_SOURCES))
clingconSharedEnv.Alias('libclingcon', clingconSharedLib)

# {{{1 liborder: Tests

TEST_LIBORDER_SOURCES = find_files(env, 'tests/testliborder/src')
TEST_LIBORDER_HEADERS = [Dir('#tests/testliborder')] + LIBORDER_HEADERS + LIBCLASP_HEADERS

orderTestEnv                = claspEnv.Clone()
orderTestEnv.Append(CPPPATH = TEST_LIBORDER_HEADERS)
orderTestEnv.Prepend(LIBS   = [orderLib, claspLib, lpLib])

testOrderProgram = orderTestEnv.Program('test_liborder', TEST_LIBORDER_SOURCES)
AlwaysBuild(orderTestEnv.Alias('test-liborder', [testOrderProgram], testOrderProgram[0].path))
if 'liborder' in env['TESTS']:
    AlwaysBuild(orderTestEnv.Alias('test', [testOrderProgram], testOrderProgram[0].path))

# {{{1 libclingcon: Tests

TEST_LIBCLINGCON_SOURCES  = find_files(env, 'tests/testlibclingcon/src')
TEST_LIBCLINGCON_HEADERS = [Dir('#tests/testlibclingcon')] + LIBORDER_HEADERS + LIBCLINGCON_HEADERS + LIBCLASP_HEADERS + LIBLP_HEADERS

clingconTestEnv                = claspEnv.Clone()
clingconTestEnv.Append(CPPPATH = TEST_LIBCLINGCON_HEADERS)
clingconTestEnv.Prepend(LIBS   = [clingconLib, orderLib, claspLib, lpLib])

testClingconProgram = clingconTestEnv.Program('test_libclingcon', TEST_LIBCLINGCON_SOURCES)
AlwaysBuild(clingconTestEnv.Alias('test-libclingcon', [testClingconProgram], testClingconProgram[0].path))
if 'libclingcon' in env['TESTS']:
    AlwaysBuild(clingconTestEnv.Alias('test', [testClingconProgram], testClingconProgram[0].path))

# {{{1 Liblp: Tests

TEST_LIBLP_SOURCES  = find_files(env, 'liblp/tests')

lpTestEnv                = env.Clone()
lpTestEnv.Append(CPPPATH = LIBLP_HEADERS)
lpTestEnv.Prepend(LIBS   = [lpLib])

testLpProgram = lpTestEnv.Program('test_liblp', TEST_LIBLP_SOURCES)
AlwaysBuild(lpTestEnv.Alias('test-liblp', [testLpProgram], testLpProgram[0].path))
if "liblp" in env["TESTS"]:
    AlwaysBuild(lpTestEnv.Alias('test', [testLpProgram], testLpProgram[0].path))


