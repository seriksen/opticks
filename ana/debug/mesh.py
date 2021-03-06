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

Mesh Structural Comparisons
=============================


::

    In [4]: run mesh.py
    INFO:__main__:base /usr/local/env/geant4/geometry/export/DayaBay_VGDX_20140414-1300/g4_00.96ff965744a2f6b78c24e33c80d3a4cd.dae/GMergedMesh/1 
    [[ 720  362 3199 3155]
     [ 672  338 3200 3199]
     [ 960  482 3201 3200]
     [ 480  242 3202 3200]
     [  96   50 3203 3200]]
    INFO:__main__:base /tmp/GMergedMesh/baseGeometry 
    [[ 720  362 3199 3155]
     [ 672  338 3200 3199]
     [ 960  482 3201 3200]
     [ 480  242 3202 3200]
     [  96   50 3203 3200]]
    WARNING:__main__:NO PATH /tmp/GMergedMesh/modifyGeometry/iidentity.npy 
    INFO:__main__:base /tmp/GMergedMesh/modifyGeometry 
    [[       720        362       3199       3155]
     [       672        338       3200       3199]
     [       960        482       3201       3200]
     [       480        242       3202       3200]
     [        96         50       3203       3200]
     [        12         24          0 4294967295]]
    WARNING:__main__:NO PATH /usr/local/env/geant4/geometry/export/dpib/cfg4.f7ba6061a8e024189e641c86eb847ee4.dae/GMergedMesh/0/aiidentity.npy 
    WARNING:__main__:NO PATH /usr/local/env/geant4/geometry/export/dpib/cfg4.f7ba6061a8e024189e641c86eb847ee4.dae/GMergedMesh/0/iidentity.npy 
    WARNING:__main__:NO PATH /usr/local/env/geant4/geometry/export/dpib/cfg4.f7ba6061a8e024189e641c86eb847ee4.dae/GMergedMesh/0/itransforms.npy 
    INFO:__main__:base /usr/local/env/geant4/geometry/export/dpib/cfg4.f7ba6061a8e024189e641c86eb847ee4.dae/GMergedMesh/0 
    [[         0          0          0 4294967295]
     [       720        362          1          0]
     [       720        362          2          1]
     [       960        482          3          2]
     [       576        288          4          2]
     [         0          0          5          2]]


"""

import os, logging
import numpy as np

from opticks.ana.base import opticks_environment
from opticks.ana.mergedmesh import MergedMesh
import matplotlib.pyplot as plt

log = logging.getLogger(__name__)

if __name__ == '__main__':

    logging.basicConfig(level=logging.INFO)
    opticks_environment()

    DPIB_ALL = os.path.expandvars("$IDPATH_DPIB_ALL");
    DPIB_PMT = os.path.expandvars("$IDPATH_DPIB_PMT");

    bases = ["$IDPATH/GMergedMesh/1",
             "/tmp/GMergedMesh/baseGeometry",
             "/tmp/GMergedMesh/modifyGeometry",
             os.path.join(DPIB_ALL,"GMergedMesh/0"),
             os.path.join(DPIB_PMT,"GMergedMesh/0"),
           ]

    for base in bases:
        base = os.path.expandvars(base) 
        if not os.path.exists(base):continue

        mm = MergedMesh(base=base)
        if base.find(DPIB_ALL)>-1 or base.find(DPIB_PMT)>0:
            mm.node_offset = 1
        else:
            mm.node_offset = 0
        pass
        log.info("base %s " % base)
        print  "nodeinfo\n", mm.nodeinfo.view(np.int32)
        nv = mm.nodeinfo[:,1].sum()
        print  "nvert %5d v.shape %s " % (nv, repr(mm.vertices.shape))
        #print  "ce\n", mm.center_extent

        print "itransforms\n",mm.itransforms
        print "iidentity\n",mm.iidentity
        print "aiidentity\n", mm.aiidentity


        

        




