//  ggv --recs

#include <cassert>

#include "Types.hpp"
#include "Typ.hpp"

#include "RecordsNPY.hpp"
#include "PhotonsNPY.hpp"

#include "Opticks.hh"

#include "OpticksEvent.hh"
#include "OpticksFlags.hh"
#include "GBndLib.hh"
#include "GMaterialLib.hh"
#include "GSurfaceLib.hh"

// architecture problem, need ggeo-/GPropertyLib funciontality 
// regards meanings of things at the lower npy- level 
// (in python just read in from persisted)
//
// maybe retain Types and simplify it to a holder of maps
// obtained from the higher level, or create a higher level 
// GEvt ?

int main(int argc, char** argv)
{
    // canonically App::indexEvtOld , contrast with npy-/ana.py

    Opticks* ok = new Opticks(argc, argv, "recs.log");

    Types* types = ok->getTypes();

    GBndLib* blib = GBndLib::load(ok, true); 
    GMaterialLib* mlib = blib->getMaterialLib();
    //GSurfaceLib*  slib = blib->getSurfaceLib();

    // see GGeo::setupTyp
    Typ* typ = ok->getTyp();
    OpticksFlags* flags = ok->getFlags();
    typ->setMaterialNames(mlib->getNamesMap());
    typ->setFlagNames(flags->getNamesMap());

    const char* src = "torch" ; 
    const char* tag = "1" ; 
    const char* det = ok->getDetector() ; 

    OpticksEvent* evt = OpticksEvent::load(src, tag, det) ;
    assert(evt); 

    NPY<float>* fd = evt->getFDomain();
    NPY<float>* ox = evt->getPhotonData();
    NPY<short>* rx = evt->getRecordData();


/*
    const char* idpath = cache->getIdPath();
    NPY<float>* fdom = NPY<float>::load(idpath, "OPropagatorF.npy");
    //NPY<int>*   idom = NPY<int>::load(idpath, "OPropagatorI.npy");

    // array([[[9, 0, 0, 0]]], dtype=int32)     ??? no 10 for maxrec 
    // NumpyEvt::load to do this ?

    NPY<float>* ox = NPY<float>::load("ox", src, tag, "dayabay");
    ox->Summary();

    NPY<short>* rx = NPY<short>::load("rx", src, tag, "dayabay");
    rx->Summary();

*/


    unsigned int maxrec = 10 ; 
    bool flat = true ; 
    RecordsNPY* rec = new RecordsNPY(rx, maxrec, flat);
    rec->setDomains(fd);
    rec->setTypes(types);
    rec->setTyp(typ);

    PhotonsNPY* pho = new PhotonsNPY(ox);
    pho->setRecs(rec);
    pho->setTypes(types);
    pho->setTyp(typ);
    pho->dump(0  ,  "ggv --recs dpho 0");

    return 0 ; 
}
