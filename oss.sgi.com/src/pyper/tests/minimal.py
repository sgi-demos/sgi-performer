from libpyper import *

pfInit()
pfConfig()

p = pfVec3()
p.set(1,2,3)
p.length()

q = p+p
print q

pfExit()

