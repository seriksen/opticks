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
mattype.py : 
===============================


Testing material mechanics

* round tripping codes 

::

    In [24]: s = "MO Py MO MO Py OV Vm MO"

    In [25]: mt.code(s)
    Out[25]: 1304315108

    In [26]: c = mt.code(s)

    In [27]: "%x" % c
    Out[27]: '4dbe44e4'

    In [30]: type(c)
    Out[30]: int


    In [23]: run material.py
    MO Py MO MO Py OV Vm MO : 4dbe44e4 : MO Py MO MO Py OV Vm MO
                                  -1 
                     ee4        0.954         476939       [3 ] MO Py Py
                      44        0.038          18876       [2 ] MO MO
                     444        0.007           3573       [3 ] MO MO MO
                    ee44        0.001            521       [4 ] MO MO Py Py
                    4444        0.000             58       [4 ] MO MO MO MO
                   44e44        0.000             22       [5 ] MO MO Py MO MO
                   ee444        0.000              4       [5 ] MO MO MO Py Py
                  44ee44        0.000              2       [6 ] MO MO Py Py MO MO
              44e5dbe444        0.000              1       [10] MO MO MO Py OV Vm Bk Py MO MO
                 44ee444        0.000              1       [7 ] MO MO MO Py Py MO MO
                  444e44        0.000              1       [6 ] MO MO Py MO MO MO
                   44444        0.000              1       [5 ] MO MO MO MO MO
                    eee4        0.000              1       [4 ] MO Py Py Py
                              500000         1.00 


"""
import os, sys, datetime, logging
log = logging.getLogger(__name__)
import numpy as np

from opticks.ana.base import Abbrev, ItemList 
from opticks.ana.seq import SeqType, SeqAna
from opticks.ana.proplib import PropLib
from opticks.ana.nload import A

SEQHIS = 0
SEQMAT = 1

def test_roundtrip(mt):
    """
    ::

       MO Py MO MO Py OV Vm MO : 4dbe44e4 : MO Py MO MO Py OV Vm MO

    """
    s = "MO Py MO MO Py OV Va MO"
    #s = "Ml Px Ml Ml Px Om Vm Ml"   ## huh why the capital casing not working ? These are first last 
    i = mt.code(s)
    l = mt.label(i)
    print "%s : %x : %s" % (s, i, l )
    assert l == s, (l,s) 

class MatType(SeqType):
    """
    MatType specializes SeqType by providing it with 
    material codes and abbreviations.

    ::

        In [17]: flags.code2name
        Out[17]: 
        {1: 'GdDopedLS',
         2: 'LiquidScintillator',
         3: 'Acrylic',
         4: 'MineralOil',
       
        In [18]: abbrev.abbr2name
        Out[18]: 
        {'AS': 'ADTableStainlessSteel',
         'Ac': 'Acrylic',
         'Ai': 'Air',
         'Al': 'Aluminium',
         'Bk': 'Bialkali',
         'Dw': 'DeadWater',


    simon:opticksdata blyth$ find . -name abbrev.json
    ./export/DayaBay/GMaterialLib/abbrev.json
    ./resource/GFlags/abbrev.json
    simon:opticksdata blyth$ 


    Formerly used "$OPTICKS_DETECTOR_DIR/GMaterialLib/abbrev.json"
    but that makes no sense in direct workflow, so now
    "$GEOCACHE/GMaterialLib/GPropertyLibMetadata.json"

    """
    def __init__(self, reldir=None):
        material_names = ItemList("GMaterialLib", reldir=reldir)
        material_abbrev = Abbrev("$GEOCACHE/GMaterialLib/GPropertyLibMetadata.json")
        SeqType.__init__(self, material_names, material_abbrev)


if __name__ == '__main__':
    from opticks.ana.main import opticks_main
    ok = opticks_main()

    #mn = ItemList("GMaterialLib")
    #ab = Abbrev("$OPTICKS_DETECTOR_DIR/GMaterialLib/abbrev.json")

    mt = MatType()
    test_roundtrip(mt)

    try:
        ph = A.load_("ph",ok.src,ok.tag,ok.det, pfx=ok.pfx)
    except IOError as err:
        log.fatal(err)
        sys.exit(1) 

    log.info("loaded ph %s %s %s " % ( ph.path, ph.stamp, repr(ph.shape)))

    seqmat = ph[:,0,1]
    ma = SeqAna.for_evt(mt, ok.tag, ok.src, ok.det, pfx=ok.pfx, offset=SEQMAT)
    print(ma.table)




