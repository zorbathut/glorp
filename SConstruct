
from __future__ import with_statement

import sys
import os
sys.path.insert(1, os.getcwd() + "/glop/site_scons")

from SConstruct_config import Conf
from SConstruct_installer import Installers

import copy
import sys
import re
import dircache

from util import exe_rv

# Globals
Decider('MD5-timestamp')
SetOption('implicit_cache', 1)

Import('name sources longname data')

env, categories, flagtypes, platform, installers = Conf()
MakeDeployables, MakeInstaller = Installers(platform)

#stdpackage = Split("debug os util parse args init")

SConscript("glop/SConstruct")

# List of buildables
buildables = [
  [name, "GAME", Split("core debug debug_911_off os util parse args init") + ["../" + x for x in Split(sources)]],
]

def addReleaseVersion(buildables, item, suffix):
  tversion = [item[0] + "-" + suffix] + item[1:]
  tversion[2] = tversion[2] + ["../version_" + suffix]
  buildables += [tversion]

def splitVersions(buildables, name):
  for item in [x for x in buildables if x[0] == name]:
    addReleaseVersion(buildables, item, "demo")
    addReleaseVersion(buildables, item, "release")
    item[2] += ["../version_local"]

splitVersions(buildables, name)  # craft2



to_build = []
built = {}
programs = {}

includeculls = {}
includeculleditems = {}
includecullheaders = {}

provideincludecull = False
for item in COMMAND_LINE_TARGETS:
  if re.match("includecull\..*", item):
    provideincludecull = True

def cullitude(item, abbreviation, ptmp):
  if not (item, abbreviation) in includeculleditems:
    includeculleditems[(item, abbreviation)] = None
    if not item in includeculls:
      includeculls[item] = []
      
    if item.rsplit('.', 1)[1] == "cpp":
      includeculls[item] += env.Command("#build/glorp/%s.%s.o" % (item, abbreviation), built[(build[1], item.rsplit('.', 1)[0])], Copy("$TARGET", "$SOURCE"))
    elif item.rsplit('.', 1)[1] == "h":
      if not item in includecullheaders:
        includecullheaders[item] = env.Command("#build/glorp/%s.cpp" % item, item, Copy("$TARGET", "$SOURCE"))
      includeculls[item] += env.Object("#build/glorp/%s.%s.o" % (item, abbreviation), includecullheaders[item], **ptmp)
    else:
      fnord();
    
    with open("%s" % item) as f:
      for line in f:
        metchsteek = re.match("""^#include "(.*)"$""", line)
        if metchsteek != None:
          fil = metchsteek.group(1)
          if fil[0:7] == "minizip":
            continue
          cullitude(fil, abbreviation, ptmp)

for build in buildables:
  params = {}
  for param in flagtypes:
    buildstring = param + "_" + build[1];
    params[param] = []
    for i in range(buildstring.count("_"), -1, -1):
      params[param] += env[buildstring.rsplit("_", i)[0]]
  
  abbreviation = "".join(x.lower()[0] for x in build[1].split("_"))
  
  objects = []
  
  for item in build[2]:
    if not (build[1], item) in built:
      built[(build[1], item)] = env.Object("#build/glorp/%s.%s.o" % (item, abbreviation), "%s.cpp" % item, **params)
      
      # enable the somewhat-slow includecull structures
      if provideincludecull:
        ptmp = dict([k for k in params.items()])
        ptmp["CPPPATH"] = ptmp["CPPPATH"] + ["."]
        cullitude(item + ".cpp", abbreviation, ptmp)
      
    objects += built[(build[1], item)]
  
  if len(build) > 3:
    for item in build[3]:
      if not (build[1], item) in built:
        built[(build[1], item)] = env.Object("#build/glorp/%s.%s.o" % (item, abbreviation), "%s.c" % item, **params)
      objects += built[(build[1], item)]
      
  if len(build) > 4 and platform=="win":
    for item in build[4]:
      if not (build[1], item) in built:
        built[(build[1], item)] = env.Command("#build/glorp/%s.%s.res" % (item, abbreviation), "%s.rc" % item, "nice windres $SOURCE -O coff -o $TARGET")
      objects += built[(build[1], item)]
  
  objects += ["glop/build-dbg-Glop/libGlopDbg.a"]
  
  programs[build[0]] = env.Program("#build/" + build[0], objects, **params)[0]

def make_data():
  rv = ["../" + x for x in data]
  
  list = dircache.listdir("..")
  for item in list:
    if item.find(".lua") != -1:
      print(item)
      rv += ["../" + item]
  
  return rv

data_dests = {}
data_dests["release"] = make_data()
data_dests["demo"] = make_data()
  
if 0:
  # data copying and merging
  data_source = traverse("data_source")

  data_vecedit = [x for x in data_source if x.split('/')[0] == "vecedit"]
  data_oggize = [x for x in data_source if x.split('.')[-1] == "wav"]
  data_copy = [x for x in data_source if not (x in data_vecedit or x in data_oggize or x in data_merge)]

  extramergedeps = {"base/tank.dwh" : ["base/weapon_sparker.dwh"], "base/factions.dwh" : [x for x in data_copy if x.rsplit('/', 1)[0] == "base/faction_icons"]}

  csvs = dict([(re.match("^build/notes_(.*)\.csv$", str(item)).group(1), item) for item in env.Command(["build/notes_%s.csv" % item.split('.')[-3].split('/')[-1] for item in data_merge], [programs["ods2csv"], "notes.ods"], "./$SOURCE notes.ods --addr2line")])

  def make_datadir(dest, mergeflags = ""):
    results = []
    vecresults = []
    
    for item in data_copy:
      results += env.Command(dest + "/" + item, "data_source/" + item, Copy("$TARGET", '$SOURCE'))
    
    for item in data_oggize:
      results += env.Command(dest + "/" + item.rsplit('.', 1)[0] + ".ogg", "data_source/" + item, "%s -q 6 -o $TARGET $SOURCE" % oggpath)
    
    return results

  data_dests = {}
  data_dests["release"] = make_datadir("data_release")
  data_dests["demo"] = make_datadir("data_demo", "--demo")


# deploy directory and associated
def commandstrip(env, source):
  return env.Command('#build/deploy/%s' % str(source).split('/')[-1], source, "cp $SOURCE $TARGET && (strip -s $TARGET || true)")[0]

programs_stripped = {}
for key, value in programs.items():
  programs_stripped[key] = commandstrip(env, value)

deployfiles = MakeDeployables(env, commandstrip)
#deployfiles += env.Command('#build/deploy/license.txt', '#resources/license.txt', Copy("$TARGET", '$SOURCE'))
#deployfiles += [programs_stripped["reporter"]]

version = str.strip(exe_rv("BASHHACK (cd .. && git describe)")[0])

def MakeInstallerShell(typ, suffix):
  return MakeInstaller(name=name, env=env, type=typ, version=version, binaries=programs_stripped, data=data_dests, deployables=deployfiles, installers=installers, suffix=suffix, longname=longname)

allpackages = []

allpackages += Alias("packagedemo", MakeInstallerShell("demo", "-demo"))
allpackages += Alias("package", Alias("packagerelease", MakeInstallerShell("release", "")))

Alias("allpackages", allpackages)

# version_*.cpp
def addVersionFile(type):
  env.Command('#version_%s.cpp' % type, [], 'echo extern const char game_version[] = \\"' + version + '\\"\; > $TARGET')

for item in "local demo release".split():
  addVersionFile(item)

# cleanup
env.Clean("#build", "#build")
#env.Clean("data_release", "data_release")
#env.Clean("data_demo", "data_demo")

libcopy = [env.Command("#build/libfreetype-6.dll", "glop/Glop/cygwin/dll/libfreetype-6.dll", Copy("$TARGET", '$SOURCE')), env.Command("#build/fmodex.dll", "glop/Glop/cygwin/dll/fmodex.dll", Copy("$TARGET", '$SOURCE'))]
env.Dir("build")

# How we actually do stuff
def command(env, name, deps, line):
  env.AlwaysBuild(env.Alias(name, deps, line))

fulldata = env.Alias(name + " program and release data", data_dests["release"] + [programs[name]] + [libcopy])
if not env.GetOption('clean'):
  env.Default(fulldata) # if we clean, we want to clean everything

localflags = ""
stdrun = localflags + ""

command(env, "run", fulldata, "%s %s" % (programs[name], stdrun))
#command(env, "run", fulldata, "%s %s" % ("cygcheck", programs[name]))
command(env, "runclean", fulldata, "%s %s" % (programs[name], localflags))

for key, value in includeculls.items():
  command(env, "includecull." + key, value, "")
