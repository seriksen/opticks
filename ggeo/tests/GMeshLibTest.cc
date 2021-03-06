/*
 * Copyright (c) 2019 Opticks Team. All Rights Reserved.
 *
 * This file is part of Opticks
 * (see https://bitbucket.org/simoncblyth/opticks).
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); 
 * you may not use this file except in compliance with the License.  
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software 
 * distributed under the License is distributed on an "AS IS" BASIS, 
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  
 * See the License for the specific language governing permissions and 
 * limitations under the License.
 */

// TEST=GMeshLibTest om-t

/**
GMeshLibTest.cc
==================

Loads the GMeshLib include NCSG solids from geocache and
allows dumping.

In direct workflow only need one envvar, the OPTICKS_KEY to identify the 
geocache to load from and the "--envkey" option to switch on sensitivity to it::
 
   GMeshLibTest --envkey --dbgmesh sFasteners
 

In legacy workflow needed the op.sh script to set multiple envvars in order to 
find the geocache::
 
   op.sh --dsst --gmeshlib --dbgmesh near_top_cover_box0xc23f970  
   op.sh --dsst --gmeshlib --dbgmesh near_top_cover_box0x


dsst
    sets geometry selection envvars, defining the path to the geocache
gmeshlib
    used by op.sh script to pick this executable GMeshLibTest 
dbgmesh
    name of mesh to dump 

**/

#include <cassert>
#include <iostream>
#include <iomanip>

#include "Opticks.hh"
#include "NCSG.hpp"
#include "GMesh.hh"
#include "GMeshLib.hh"
#include "GMesh.hh"
#include "NBBox.hpp"
#include "NQuad.hpp"
#include "NNode.hpp"
#include "GLMFormat.hpp"
#include "NGLMExt.hpp"

#include "OPTICKS_LOG.hh"




void test_getDbgMeshByName(const Opticks& ok, const GMeshLib* meshlib)
{
    const char* dbgmesh = ok.getDbgMesh();
    if(dbgmesh)
    {
        bool startswith = true ; 
        const GMesh* mesh = meshlib->getMeshWithName(dbgmesh, startswith);
        mesh->dump("GMesh::dump", 50);
        const NCSG* solid = mesh->getCSG(); 
        assert( solid );     
        solid->dump();  
    }
    else
    {
        LOG(info) << "no dbgmesh" ; 
    }
}


void test_dump0( const GMeshLib* meshlib )
{
    unsigned num_mesh = meshlib->getNumMeshes(); 
    LOG(info) << " num_mesh " << num_mesh ; 

    for(unsigned i=0 ; i < num_mesh ; i++)
    {
        const GMesh* mesh = meshlib->getMeshSimple(i); 
        const char* name = mesh->getName() ; 

        const NCSG* solid = mesh->getCSG(); 
        nbbox bba = solid->bbox(); // global frame bbox
        nvec4 ce = bba.center_extent() ; 

        //nnode* root = solid->getRoot(); 
        //if( root->transform && !root->transform->is_identity() ) LOG(info) << " tr " << *root->transform ; 

        std::cout  
            << std::setw(2) << i 
            << std::setw(45) << ( name ? name : "NULL" )
            << " bba " << bba.description()
            << " ce " << std::setw(25) << ce.desc()
            << " " << std::setw(2) << i 
            << std::endl    
            ; 
    }
}


void test_dump1( const GMeshLib* meshlib )
{
    unsigned num_mesh = meshlib->getNumMeshes(); 
    LOG(info) << " num_mesh " << num_mesh ; 

    for(unsigned i=0 ; i < num_mesh ; i++)
    {
        const GMesh* mesh = meshlib->getMeshSimple(i); 
        const char* name = mesh->getName() ; 
        const NCSG* csg = mesh->getCSG(); 
        glm::vec4 ce0 = csg->bbox_center_extent(); 

         
        //const_cast<NCSG*>(csg)->apply_centering(); 
        const_cast<GMesh*>(mesh)->applyCentering(); 

        int w(40);   

        glm::vec4 ce1 = csg->bbox_center_extent(); 
        std::cout  
            << std::setw(2) << i 
            << std::setw(45) << ( name ? name : "NULL" )
            << " ce0 " << std::setw(w) << gformat(ce0)
            << " ce1 " << std::setw(w) << gformat(ce1)
            << " " << std::setw(2) << i 
            << std::endl    
            ; 
    }
}


int main(int argc, char** argv)
{
    OPTICKS_LOG(argc, argv);

    Opticks ok(argc, argv, "--envkey" );
    ok.configure();
    if(!ok.isDirect())
    {
        LOG(fatal) << "this is a direct only test : that means use --envkey option and have a valid OPTICKS_KEY envvar "  ; 
        return 0 ; 
    }


    GMeshLib* meshlib = GMeshLib::Load(&ok);

    //test_getDbgMeshByName( ok, meshlib ); 
    //test_dump0( meshlib ); 
    test_dump1( meshlib ); 


    return 0 ; 
}

