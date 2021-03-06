extend-opticks-nnt-to-direct-translation
==========================================

motivation
------------

*opticks-nnt* code generation producing python/CSG and nnode/C++ for 
each solid and the bash tbool for running them was extremely useful 
for debugging booleans in the old route.  So want to extend that one 
step backwards and generate Geant4 geometry code from the nnode model : 
to allow automated clean room testing of each individual G4 solid used 
in a geometry and its translation to the GPU.  Would want to test in 
both unbalanced and balanced forms : as balancing is a probable
cause of issues.

how to use the generated G4 geometry cc for each solid ?
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Ideas:

1.treat it just like python CSG : ie develop some 
  holder class that can be used from some minimal code to 
  convert the G4VSolid into NCSG : so can operate just like the 
  tboolean tests 

  * for that to work will need to polygonize and persist those meshes 
    too so will need to construct a GGeo and persist it : hmm thats a lot of 
    complexity for a test : actually my flawed poly is OK for this, as the 
    poly is just for vis, what matters is the raytrace ... this is 
    taking the same approach as tboolean-

  * proceed with this by : creating a class to minimise the code needed
    to go from a G4VSolid defined in code to a serialised NCSG : can this
    be done from NPY level : actually this needs to do the same as
    analytic/csg.py CSG.Serialize  

    Unusual for Opticks combination of dependencies : G4 and NPY/NCSG, 
    so new pkg y4csg, but need X4Solid so might as well put into X4.
     

2. place the G4VSolid in pure G4 LV/PV G4Material context, 
   and persist to GDML : then can test from that GDML with OKX4Test


Actually will probably need both these approaches, the first is 
most natural for solid level debugging and the second for structure level
debugging.


mechanics of using the generated code
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

In C++ its simpler to generate a bunch of G4VSolid creating 
functions with an index in the name together with a switch statement
to pick them by index. Then can launch with eg *x4-csg 42*

The geometry provider is a separate executable *X4CSGTest* with 
a generated section for the g4code, which serializes to a directory and
emits to stderr the testconfig opticks argument. 


how to implement ?
~~~~~~~~~~~~~~~~~~~~~~

Each nnode primitive subclass+operator type will need 
methods analogous to the *analytic/csg.py* `content_*` 
returning strings and something like the below in the base 
to bring them all together::

    void nnode::as_g4code(std::ostream& out) const 

Follow pattern of GMeshLib::reportMeshUsage for stream mechanics.

I would prefer to not have this code in nnode for clarity but that 
seems difficult initially. So start in nnode by look for opportunites to 
to factor it off.  This can yield a string of g4code for each node
of the tree. 

Note there is no G4 dependency : are just generating strings that 
happen to be Geant4 geometry code.


alt approach : from "System 1" following lunch 
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Doing this in X4Solid will be much easier than nnode:

* have the real Geant4 instance right there to spill its beans
* this code gen is just for testing : so no problem to do it only at conversion 
  and the resulting string can be stored along with the nnode

* but this doesnt help with testing of tree balancing ? BUT its so much 
  faster to implement than the nnode approach that its worth doing if first, checking 
  all solids from this perspective and then worrying about tree balancing later.

::

    implemented via X4SolidBase::setG4Param and X4SolidBase::g4code which gets invoked
    by the setG4Param


what remains to implement
~~~~~~~~~~~~~~~~~~~~~~~~~~~

* transforms, displaced solids



how to test ?
~~~~~~~~~~~~~~

Start in x4:tests/X4SolidTest then move to okg4:tests/OKX4Test 
once nearly done.

OKX4Test
    messy multi-Opticks boot using CGDMLDetector, then converts
    with X4PhysicalVolume populating a 2nd GGeo instance.


how to persist ?
~~~~~~~~~~~~~~~~~~

Start with adhoc writers on nnode/NCSG that take a path, may need 
to go all way up to GParts for "official" writes into geocache.

 

X4PhysicalVolume
-------------------

The recursive *X4PhysicalVolume::convertNode* invokes *X4Solid::Convert* on first meeting each solid::

    506      if(mh->csgnode == NULL)
    507      {
    508          // convert G4VSolid into nnode tree and balance it if overheight 
    509 
    510          nnode* tree = X4Solid::Convert(solid)  ;
    511          nnode* result = NTreeProcess<nnode>::Process(tree, nd->soIdx, lvIdx);
    512 
    513          mh->csgnode = result ;



review opticks-nnt codegen
-------------------------------

During the old python gdml2gltf conversion the geometry codegen is 
invoked as part of saving the csg into the gltf extras.

*opticks.analytic.csg:as_code* invokes node recursive *as_code_r* that 
converts the tree parsed from the GDML into python/CSG or C++/nnode geometry description::

    1335     def as_tbool(self, name="esr"):
    1336         tbf = TBoolBashFunction(name=name, root=self.alabel, body=self.as_code(lang="py")  )
    1337         return str(tbf)
    1338 
    1339     def as_NNodeTest(self, name="esr"):
    1340         nnt  = NNodeTestCPP(name=name, root=self.alabel, body=self.as_code(lang="cpp")  )
    1341         return str(nnt)

*opticks.analytic.csg:save* writes the generated code to file::

 793         self.write_tbool(lvidx, tboolpath)
 794 
 795         nntpath = self.nntpath(treedir, lvidx)
 796         self.write_NNodeTest(lvidx, nntpath)
 797 
 798         nodepath = self.nodepath(treedir)
 799         np.save(nodepath, nodebuf)


These are written inside the extras of the old glTF::

    [blyth@localhost ~]$ opticks-tbool-info

    opticks-tbool-info
    ======================

       opticks-tbool-path 0 : /home/blyth/local/opticks/opticksdata/export/DayaBay_VGDX_20140414-1300/extras/0/tbool0.bash
       opticks-nnt-path 0   : /home/blyth/local/opticks/opticksdata/export/DayaBay_VGDX_20140414-1300/extras/0/NNodeTest_0.cc




review tboolean launching
----------------------------

tboolean-box--::

    import logging
    log = logging.getLogger(__name__)
    from opticks.ana.base import opticks_main
    from opticks.analytic.csg import CSG  
    autoemitconfig="photons:600000,wavelength:380,time:0.2,posdelta:0.1,sheetmask:0x1,umin:0.45,umax:0.55,vmin:0.45,vmax:0.55,diffuse:1,ctmindiffuse:0.5,ctmaxdiffuse:1.0"
    args = opticks_main(csgpath="/tmp/blyth/opticks/tboolean-box--", autoemitconfig=autoemitconfig)

    ... geometry and photon source setup ....

    CSG.Serialize([container, box], args )



* `tboolean-box--` emits the python source to stdout 
* `tboolean-box-` pipes that to python, which serializes geometry trees to csgpath "/tmp/blyth/opticks/tboolean-box--" and emits testconfig to stdout
* `tboolean-box` collects that testconfig into TESTCONFIG and invokes `tboolean--`

::

    tboolean-box- 2>/dev/null | tr "_" "\n"

    autoseqmap=TO:0,SR:1,SA:0
    name=tboolean-box--
    outerfirst=1
    analytic=1
    csgpath=/tmp/blyth/opticks/tboolean-box--
    mode=PyCsgInBox
    autoobject=Vacuum/perfectSpecularSurface//GlassSchottF2
    autoemitconfig=photons:600000,wavelength:380,time:0.2,posdelta:0.1,sheetmask:0x1,umin:0.45,umax:0.55,vmin:0.45,vmax:0.55,diffuse:1,ctmindiffuse:0.5,ctmaxdiffuse:1.0
    autocontainer=Rock//perfectAbsorbSurface/Vacuum


::

    tboolean-box- () 
    { 
        $FUNCNAME- | python $*
    }
    tboolean-box () 
    { 
        TESTNAME=$FUNCNAME TESTCONFIG=$($FUNCNAME- 2>/dev/null) tboolean-- $*
    }
    tboolean-- () 
    { 
        tboolean-;
        local msg="=== $FUNCNAME :";
        local cmdline=$*;
        local stack=2180;
        local testname=$(tboolean-testname);
        local testconfig=$(tboolean-testconfig);
        local torchconfig=$(tboolean-torchconfig);
        $testname--;
        op.sh $cmdline --rendermode +global,+axis --animtimemax 20 --timemax 20 --geocenter --stack $stack --eye 1,0,0 --dbganalytic --test --testconfig "$testconfig" --torch --torchconfig "$torchconfig" --torchdbg --tag $(tboolean-tag) --cat $testname --anakey tboolean --save
    }






