
from __future__ import with_statement

if False:

  import os
  import commands
  import sys

  from SCons.Environment import Environment
  from SCons.Util import Split
  from SCons.Defaults import Copy

  from util import dispatcher

  def Installers(platform):
    
    if platform == "win":
      def MakeDeployables(env, commandstrip):
        deploy_dlls = Split("SDL.dll libogg-0.dll libvorbis-0.dll libvorbisfile-3.dll libpng-3.dll wxbase28_gcc_custom.dll wxmsw28_core_gcc_custom.dll wxmsw28_gl_gcc_custom.dll mingwm10.dll")
        
        deployfiles = []
        paths = ["/usr/mingw/local/bin/", "/usr/mingw/local/lib/", "/bin/"]
        
        for item in deploy_dlls:
          for prefix in paths:
            if os.path.exists(prefix + item):
              deployfiles += [commandstrip(env, prefix + item)]
              break

        return deployfiles

      def generateInstaller(target, source, copyprefix, files, deployfiles, finaltarget, mainexe, vecedit, version):

        directories = {"data" : None}
        
        for item in [x.split('/', 1)[1] for x in files]:
          for steps in range(len(item.split('/')) - 1):
            directories["data/" + item.rsplit('/', steps + 1)[0]] = None
        
        directories = [x.replace('/', '\\') for x in directories.iterkeys()]
        files = [x.replace('/', '\\') for x in files]
        deployfiles = [x.replace('/', '\\') for x in deployfiles]
        
        mainexe = str(mainexe).replace('/', '\\')
        vecedit = str(vecedit).replace('/', '\\')
        
        install = ""
        uninstall = ""

        for line in directories:
          install = install + 'CreateDirectory "$INSTDIR\\%s"\n' % line
          uninstall = 'RMDir "$INSTDIR\\%s"\n' % line + uninstall

        for line in files:
          install = install + 'File "/oname=data\\%s" "%s"\n' % (line.split('\\', 1)[1], line)
          uninstall = 'Delete "$INSTDIR\\data\\%s"\n' % line.split('\\', 1)[1] + uninstall

        install = install + 'File "/oname=settings" "settings.%s"\n' % copyprefix
        uninstall = 'Delete "$INSTDIR\\settings"\n' + uninstall;

        for line in deployfiles:
          install = install + 'File "/oname=%s" "%s"\n' % (line.rsplit('\\', 1)[1], line)
          uninstall = 'Delete "$INSTDIR\\%s"\n' % line.rsplit('\\', 1)[1] + uninstall
        
        install = install + 'File "/oname=d-net.exe" "%s"\n' % mainexe
        uninstall = 'Delete "$INSTDIR\\d-net.exe"\n' + uninstall
        
        install = install + 'File "/oname=vecedit.exe" "%s"\n' % vecedit
        uninstall = 'Delete "$INSTDIR\\vecedit.exe"\n' + uninstall

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
                print >> otp, line

      def MakeInstaller(env, type, shopcaches, version, binaries, data, deployables, installers, suffix):
        nsipath = 'build/installer_%s.nsi' % (suffix)
        ident = '%s-%s' % (version, suffix)
        finalpath = 'build/dnet-%s-%s.exe' % (version, suffix)
        mainexe = binaries["d-net-" + type]
        vecedit = binaries["vecedit-" + type]
        
        deps = data[type] + deployables + shopcaches + [mainexe] + [vecedit]
        
        nsirv = env.Command(nsipath, ['installer.nsi.template', 'SConstruct_installer.py'] + deps, dispatcher(generateInstaller, copyprefix=type, files=[str(x) for x in data[type] + shopcaches], deployfiles=[str(x) for x in deployables], finaltarget=finalpath, mainexe=mainexe, vecedit=vecedit, version=ident)) # Technically it only depends on those files existing, not their actual contents.
        return env.Command(finalpath, nsirv + deps, "%s - < ${SOURCES[0]}" % installers)
      
      return MakeDeployables, MakeInstaller
    elif platform == "linux":
      def MakeDeployables(env, commandstrip):
        return []

      def SpewDatafiles(env, suffix, binaries, data, type, incdata):
        sources = []
        binrel = []

        for item in ["d-net-%s" % type, "vecedit-%s" % type, "reporter"]:
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

