
import dircache
import subprocess

def traverse(path, prefix=""):
  if path[-1] != '/':
    path = path + '/'
  
  list = dircache.listdir(path)
  dircache.annotate(path, list)
  olist = []
  for item in list:
    if item == '.svn/':
      continue
    if item[-1] == '/':
      olist += traverse(path + item, prefix + item)
    else:
      olist += [prefix + item]
  return olist

class DispatcherClass:
  def __init__(self, function, var, map):
    self.function = function
    self.var = var
    self.map = map
  
  def __call__(self, target, source, env):
    self.function(target, source, *self.var, **self.map)

def dispatcher(function, *var, **map):
  return DispatcherClass(function, var, map)

def exe_rv(line):
  ldat = line.split(" ")
  if line.find("BASHHACK") != -1:
    ldat = ["bash", "-c", line.split(" ", 1)[1]]
  
  print(ldat)
  sp = subprocess.Popen(ldat, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
  so, se = sp.communicate()
  so = str(so)
  se = str(se)
  rv = sp.returncode
  assert(rv != None)
  return so, se, rv
