#pragma once

#include <string>
#include <vector>
#include <map>

#include <glm/fwd.hpp>

template <typename T> class NPY ; 
#include "NPY_API_EXPORT.hh"

/**
NCSG
======

cf dev/csg/csg.py 

* this can currently only Deserialize from a python written directory 

* hmm the raw loaded buffer lacks bounding boxes when derived from user input, 
  to get those need to vivify the CSG tree and the export it back to the buffer


Where NCSG is used
-------------------

void GGeoTest::loadCSG(const char* csgpath, std::vector<GSolid*>& solids)

    NCSG::Deserialize creates csg trees, which are collected
    into GSolids by GMaker::makeFromCSG  

    This is the newer python CSG definition route that is usurping the 
    old bash config GGeoTest::createCsgInBox approach  

GSolid* GMaker::makeFromCSG(NCSG* csg)

    Coordinates:

    * NPolygonizer -> GMesh 
    * GParts::make, create analytic description
    * GSolid, container for GMesh and GParts

GParts* GParts::make( NCSG* tree)

    Little to do, just bndlib hookup



**/

struct nvec4 ; 
struct nnode ; 
struct nmat4pair ; 

class NParameters ; 

class NPY_API NCSG {
        friend struct NCSGLoadTest ; 
        typedef std::map<std::string, nnode*> MSN ; 
    public:
        enum { NJ = 4, NK = 4, MAX_HEIGHT = 10 };
        static const char* FILENAME ; 
        static unsigned NumNodes(unsigned height);
        static int Deserialize(const char* base, std::vector<NCSG*>& trees);
        static NCSG* FromNode(nnode* root, const char* boundary);
    public:
        void dump(const char* msg="NCSG::dump");
        std::string desc();
    public:
        const char* getTreeDir();
        unsigned getIndex();
        const char* getBoundary();
        NPY<float>* getNodeBuffer();
        NPY<float>* getTransformBuffer();
        NParameters* getMeta();
        unsigned getNumNodes();
        unsigned getHeight();
        nnode* getRoot();
    public:
        void check();
        void check_r(nnode* node); 
    private:
        // Deserialize
        NCSG(const char* treedir, unsigned index=0u);
        void setBoundary(const char* boundary);
        unsigned getTypeCode(unsigned idx);
        unsigned getTransformIndex(unsigned idx);
        nvec4 getQuad(unsigned idx, unsigned j);
        void load();
        void import();
        nnode* import_r(unsigned idx, nnode* parent=NULL);
        nmat4pair* import_transform(unsigned itra);
    private:
         // Serialize
        NCSG(nnode* root, unsigned index=0u);
        void export_r(nnode* node, unsigned idx);
        void export_();
    private:
        unsigned    m_index ; 
        nnode*      m_root ;  
        const char* m_treedir ; 
        NPY<float>* m_nodes ; 
        NPY<float>* m_transforms ; 
        NPY<float>* m_gtransforms ; 
        NParameters* m_meta ; 
        unsigned    m_num_nodes ; 
        unsigned    m_num_transforms ; 
        unsigned    m_height ; 
        const char* m_boundary ; 



};


