
from __future__ import with_statement

import os
import commands
import sys
import re

from SCons.Environment import Environment
from SCons.Util import Split
from SCons.Defaults import Copy

from util import dispatcher

def Installers(platform):
  
  if platform == "win":
    def MakeDeployables(env, commandstrip):
      deploy_dlls = Split("fmodex.dll libfreetype-6.dll")
      
      deployfiles = []
      paths = ["Glop/Glop/cygwin/dll/"]
      
      for item in deploy_dlls:
        found = False
        for prefix in paths:
          if os.path.exists(prefix + item):
            deployfiles += [commandstrip(env, prefix + item)]
            found = True
            break
        assert(found)

      return deployfiles

    def generateInstaller(target, source, name, copyprefix, files, deployfiles, finaltarget, mainexe, version, longname):

      directories = {"data" : None}
      
      rfiles = []
      for item in files:
        print("files work", str(item))
        if str.find(str(item), "cygdrive") != -1:
          print("complex")
          rfiles += [re.compile(".*no_such_thing/").sub("", str(item))]
          print(re.compile(".*no_such_thing/").sub("", str(item)))
        else:
          print("simple")
          rfiles += [item]
      files = rfiles
      
      for item in [x.split('/', 1)[1] for x in files]:
        for steps in range(len(item.split('/')) - 1):
          directories[item.rsplit('/', steps + 1)[0]] = None
      
      directories = [x.replace('/', '\\') for x in directories.iterkeys()]
      files = [x.replace('/', '\\') for x in files]
      deployfiles = [x.replace('/', '\\') for x in deployfiles]
      
      mainexe = str(mainexe).replace('/', '\\')
      
      install = ""
      uninstall = ""

      for line in directories:
        install = install + 'CreateDirectory "$INSTDIR\\%s"\n' % line
        uninstall = 'RMDir "$INSTDIR\\%s"\n' % line + uninstall

      for line in files:
        print(line)
        if str.find(line, "build\\") != -1:
          src = line
        else:
          src = line.split('\\', 1)[1]
        install = install + 'File "/oname=%s" "%s"\n' % (line.split('\\', 1)[1], src)
        uninstall = 'Delete "$INSTDIR\\%s"\n' % line.split('\\', 1)[1] + uninstall

      #install = install + 'File "/oname=settings" "settings.%s"\n' % copyprefix
      #uninstall = 'Delete "$INSTDIR\\settings"\n' + uninstall;

      for line in deployfiles:
        install = install + 'File "/oname=%s" "%s"\n' % (line.rsplit('\\', 1)[1], line)
        uninstall = 'Delete "$INSTDIR\\%s"\n' % line.rsplit('\\', 1)[1] + uninstall
      
      install = install + 'File "/oname=%s.exe" "%s"\n' % (name, mainexe)
      uninstall = 'Delete "$INSTDIR\\%s.exe"\n' % name + uninstall

      with open(str(source[0])) as inp:
        with open(str(target[0]), "w") as otp:
          for line in inp.readlines():
            line = line.strip()
            if line == "$$$INSTALL$$$":
              print >> otp, install
            elif line == "$$$UNINSTALL$$$":
              print >> otp, uninstall
            elif line == "$$$VERSION$$$":
              print >> otp, '!define PRODUCT_VERSION "%s"' % version
            elif line == "$$$TYPE$$$":
              print >> otp, '!define PRODUCT_TYPE "%s"' % copyprefix
            elif line == "$$$OUTFILE$$$":
              print >> otp, 'OutFile "%s"' % finaltarget
            else:
              print >> otp, line.replace("$$$LONGNAME$$$", longname).replace("$$$EXENAME$$$", name + ".exe")

    def MakeInstaller(env, type, version, binaries, data, deployables, installers, suffix, name, longname):
      nsipath = '#build/installer_%s.nsi' % (suffix)
      ident = '%s%s' % (version, suffix)
      finalpath = 'build/%s-%s%s.exe' % (name, version, suffix)
      mainexe = binaries[name + "-" + type]
      
      deps = data[type] + deployables + [mainexe]
      
      nsirv = env.Command(nsipath, ['installer.nsi.template', 'SConstruct_installer.py'] + deps, dispatcher(generateInstaller, name=name, longname=longname, copyprefix=type, files=[str(x) for x in data[type]], deployfiles=[re.search(r"build/deploy/.*", str(x)).group(0) for x in deployables], finaltarget=finalpath, mainexe=mainexe, version=ident)) # Technically it only depends on those files existing, not their actual contents.
      return env.Command(finalpath, nsirv + deps, "%s - < ${SOURCES[0]}" % installers)
    
    return MakeDeployables, MakeInstaller
  elif platform == "linux":
    def MakeDeployables(env, commandstrip):
      return []

    def SpewDatafiles(env, suffix, binaries, data, type, incdata):
      sources = []
      binrel = []

      for item in ["%s-%s" % (name, type), "vecedit-%s" % type, "reporter"]:
        brs = "usr/games/%s" % item.rsplit('-', 1)[0]
        sources += env.Command("build/deploy/%s/%s" % (suffix, brs), binaries[item], Copy('$TARGET', '$SOURCE'))
        binrel += [brs]

      if incdata:
        for item in data:
          sources += env.Command("build/deploy/%s/usr/share/d-net/data/%s" % (suffix, str(item).split('/', 1)[1]), item, Copy('$TARGET', '$SOURCE'))
        sources += env.Command("build/deploy/%s/usr/share/d-net/settings" % (suffix), "settings." + type, Copy('$TARGET', '$SOURCE'))
        sources += env.Command("build/deploy/%s/usr/share/app-install/icons/d-net-icon.png" % (suffix), "resources/dneticomulti.ico", "convert $SOURCE[1] $TARGET")
        
        sources += env.Command("build/deploy/%s/usr/share/doc/d-net/copyright" % suffix, "resources/license.txt", Copy('$TARGET', '$SOURCE'))
        sources += env.Command("build/deploy/%s/usr/share/menu/d-net" % suffix, "resources/linux/menu-d-net", Copy('$TARGET', '$SOURCE'))
        sources += env.Command("build/deploy/%s/usr/share/applications/d-net.desktop" % suffix, "resources/linux/applications-d-net", Copy('$TARGET', '$SOURCE'))

      sources += env.Command("build/deploy/%s/DEBIAN/postrm" % suffix, "resources/linux/postrm", Copy('$TARGET', '$SOURCE'))
      sources += env.Command("build/deploy/%s/DEBIAN/postinst" % suffix, "resources/linux/postinst", Copy('$TARGET', '$SOURCE'))

      return sources, binrel
      

    def MakeInstaller(env, type, shopcaches, version, binaries, data, deployables, installers, suffix):
      vtoken = "%s+%s" % (version, suffix)
      depsuffix = suffix + "+dependey"
      deploysources, binrel = SpewDatafiles(env, suffix, binaries, data[type] + shopcaches, type, True)
      dependcreatesources, binrel = SpewDatafiles(env, depsuffix, binaries, data[type] + shopcaches, type, False)

      depcont = env.Command("build/deploy/%s/debian/control" % depsuffix, ["resources/linux/control"] + dependcreatesources, """(cat $SOURCE ; echo "Source: lulz") > $TARGET""")
      shlibdepl = ""
      for item in binrel:
        if shlibdepl != "":
          shlibdepl += " -e"
        shlibdepl += item
      subst = env.Command("build/deploy/%s/debian/substvars" % depsuffix, depcont + dependcreatesources, """cd build/deploy/%s && dpkg-shlibdeps %s""" % (depsuffix, shlibdepl))
      sources = env.Command("build/deploy/%s/DEBIAN/control" % suffix, ["resources/linux/control"] + subst + deploysources, """(cat $SOURCE ; (cat ${SOURCES[1]} | cut -d : -f2- | sed 's/=/: /')) | sed -e 's/&&&VERSION&&&/%s/' -e 's/&&&INSTALLSIZE&&&/'`du -s build/deploy/%s/usr | cut -f 1`'/' > $TARGET""" % (vtoken, suffix))

      return env.Command("build/dnet-%s.deb" % (vtoken), sources, "fakeroot dpkg-deb -bz9 build/deploy/%s $TARGET" % suffix)
    
    return MakeDeployables, MakeInstaller
  else:
    lol

