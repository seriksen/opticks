#!/usr/bin/env python
#
# Copyright (c) 2019 Opticks Team. All Rights Reserved.
#
# This file is part of Opticks
# (see https://bitbucket.org/simoncblyth/opticks).
#
# Licensed under the Apache License, Version 2.0 (the "License"); 
# you may not use this file except in compliance with the License.  
# You may obtain a copy of the License at
#
#   http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software 
# distributed under the License is distributed on an "AS IS" BASIS, 
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  
# See the License for the specific language governing permissions and 
# limitations under the License.
#

"""
"""
import os, logging, numpy as np
log = logging.getLogger(__name__)

from opticks.ana.geocache import keydir
from opticks.ana.prim import Dir

class Geom2d(object):
    """
    Scratch geometry, for designing flight paths
    """
    def __init__(self, kd, ridx=0):
        self.ridx = str(ridx)
        self.kd = kd
        self._pv = None
        self._lv = None
        self.ce = np.load(os.path.join(self.kd, "GMergedMesh", self.ridx,"center_extent.npy"))
        self.d = Dir(os.path.expandvars(os.path.join("$IDPATH/GParts",self.ridx)))     ## mm0 analytic
        self.select_gprim()
    
    def _get_pv(self):
        if self._pv is None:
            self._pv =  np.loadtxt(os.path.join(self.kd, "GNodeLib/PVNames.txt" ), dtype="|S100" )
        return self._pv
    pv = property(_get_pv)
    
    def _get_lv(self):
        if self._lv is None:
            self._lv =  np.loadtxt(os.path.join(self.kd, "GNodeLib/LVNames.txt" ), dtype="|S100" )
        return self._lv
    lv = property(_get_lv)

    def select_gprim(self, names=False):
        pp = self.d.prims
        sli = slice(0,None)
        gprim = []
        for p in pp[sli]:
            if p.lvIdx in [11,12]: continue   # expHall and topRock
            if p.lvIdx in [8,9]: continue   # too many  of these LV
            if p.numParts > 1: continue     # skip compounds for now
            gprim.append(p)
            print(repr(p)) 
            vol = p.idx[0]
            print(self.ce[vol])
            #print(str(p)) 
            if names:
                pv = self.pv[vol]
                lv = self.lv[vol]
                print(pv)
                print(lv)
            pass
        pass
        self.gprim = gprim 


    def dump(self):
        for i,p in enumerate(self.gprim):
            assert len(p.parts) == 1 
            pt = p.parts[0]
            print(repr(p)) 
            #print(str(p))
            #print(pt) 
            #print(pt.tran) 
 
    def render(self, ax, art3d=None):   
        sc = 1000
        for i,p in enumerate(self.gprim):
            assert len(p.parts) == 1 
            pt = p.parts[0]
            sh = pt.as_shape("prim%s" % i, sc=sc ) 
            if sh is None: 
               print(str(p))
               continue
            #print(sh)
            for pa in sh.patches():
                ax.add_patch(pa)
                if not art3d is None:
                    art3d.pathpatch_2d_to_3d(pa, z=0, zdir="y")
                pass
            pass
        pass





if __name__ == '__main__':

    logging.basicConfig(level=logging.INFO)

    ok = os.environ["OPTICKS_KEY"]
    kd = keydir(ok)
    log.info(kd)
    assert os.path.exists(kd), kd 
    os.environ["IDPATH"] = kd    ## TODO: avoid having to do this, due to prim internals

    mm0 = Geom2d(kd, ridx=0)

    import matplotlib.pyplot as plt 
    from mpl_toolkits.mplot3d import Axes3D 
    import mpl_toolkits.mplot3d.art3d as art3d

    plt.ion()
    fig = plt.figure(figsize=(6,5.5))
    ax = fig.add_subplot(111,projection='3d')
    plt.title("mm0 geom2d")
    sz = 25

    ax.set_xlim([-sz,sz])
    ax.set_ylim([-sz,sz])
    ax.set_zlim([-sz,sz])

    ax.set_xlabel("x")
    ax.set_ylabel("y")
    ax.set_zlabel("z")


    mm0.render(ax, art3d=art3d)






