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

#include <set>
#include <string>


#include "Opticks.hh"
#include "GGeo.hh"
#include "GBndLib.hh"
#include "GSurfaceLib.hh"
#include "GMergedMesh.hh"

#include "OPTICKS_LOG.hh"
#include "GGEO_BODY.hh"

void misc(GGeo* m_ggeo)
{
    unsigned int nmm = m_ggeo->getNumMergedMesh();
    for(unsigned int i=0 ; i < nmm ; i++)
    { 
        GMergedMesh* mm = m_ggeo->getMergedMesh(i) ;
        unsigned int numVolumes = mm->getNumVolumes();
        unsigned int numVolumesSelected = mm->getNumVolumesSelected();

        LOG(info) << " i " << i 
                  << " numVolumes " << numVolumes       
                  << " numVolumesSelected " << numVolumesSelected ;      

        for(unsigned int j=0 ; j < numVolumes ; j++)
        {
            gbbox bb = mm->getBBox(j);
            bb.Summary("bb");
        }

        GBuffer* friid = mm->getFaceRepeatedInstancedIdentityBuffer();
        if(friid) friid->save<unsigned int>("$TMP/friid.npy");

        GBuffer* frid = mm->getFaceRepeatedIdentityBuffer();
        if(frid) frid->save<unsigned int>("$TMP/frid.npy");
    }
}



// TODO add methods like below to GGeo
//
//    private:
//        void findSensorVolumePairs();
//    public:
//        unsigned getNumSensorVolumePairs();
//        const std::pair<std::string, std::string>& getSensorVolumePair(unsigned p);    
//
//     use this in CGeometry to reconstruct G4LogicalBorderSurface for the cathodes
//     when using CGDMLDetector
//  





void test_GGeo(const GGeo* gg)
{
    GMergedMesh* mm = gg->getMergedMesh(0);
    unsigned numVolumes = mm->getNumVolumes();
    LOG(info) << " numVolumes " << numVolumes ; 

    GBndLib* blib = gg->getBndLib();

    unsigned unset(-1) ; 

    typedef std::pair<std::string, std::string> PSS ; 
    std::set<PSS> pvp ; 

    for(unsigned i=0 ; i < numVolumes ; i++)
    {
        guint4 nodeinfo = mm->getNodeInfo(i);
        unsigned nface = nodeinfo.x ;
        unsigned nvert = nodeinfo.y ;
        unsigned node = nodeinfo.z ;
        unsigned parent = nodeinfo.w ;
        assert( node == i );

        guint4 id = mm->getIdentity(i);
        unsigned node2 = id.x ;
        unsigned mesh = id.y ;
        unsigned boundary = id.z ;
        unsigned sensor = id.w ;
        assert( node2 == i );
        
        guint4 iid = mm->getInstancedIdentity(i);  // nothing new for GlobalMergedMesh 
        assert( iid.x == id.x );
        assert( iid.y == id.y );
        assert( iid.z == id.z );
        assert( iid.w == id.w );

        std::string bname = blib->shortname(boundary);
        guint4 bnd = blib->getBnd(boundary);

        //unsigned imat = bnd.x ; 
        unsigned isur = bnd.y ; 
        unsigned osur = bnd.z ; 
        //unsigned omat = bnd.w ; 

        const char* ppv = parent == unset ? NULL : gg->getPVName(parent) ;
        const char* pv = gg->getPVName(i) ;
        //const char* lv = gg->getLVName(i) ;

        bool hasSurface =  isur != unset || osur != unset ; 

        bool select = hasSurface && sensor > 0 ; 

        if(select)
        {
             std::cout 
             << " " << std::setw(8) << i 
             << " ni[" 
             << " " << std::setw(6) << nface
             << " " << std::setw(6) << nvert 
             << " " << std::setw(6) << node
             << " " << std::setw(6) << parent
             << " ]"
             << " id[" 
             << " " << std::setw(6) << node2
             << " " << std::setw(6) << mesh
             << " " << std::setw(6) << boundary
             << " " << std::setw(6) << sensor
             << " ]"
             
             << " " << std::setw(50) << bname
             << std::endl 
             ;

/*
            std::cout << "  lv " << lv << std::endl ; 
            std::cout << "  pv " << pv << std::endl ; 
            std::cout << " ppv " << ppv << std::endl ; 
*/
            pvp.insert(PSS(pv,ppv));

      }

    }
    LOG(info) << " pvp.size " << pvp.size() ; 

    for(std::set<PSS>::const_iterator it=pvp.begin() ; it != pvp.end() ; it++ )
    {
        std::string pv = it->first ; 
        std::string ppv = it->second ; 

        std::cout 
               << std::setw(60) << pv
               << std::setw(60) << ppv
               << std::endl ; 

    }

} 


void test_GGeo_sd(const GGeo* m_ggeo)
{
    unsigned nlvsd = m_ggeo->getNumLVSD() ;
    LOG(info) << " nlvsd " << nlvsd ; 
}




int main(int argc, char** argv)
{
    OPTICKS_LOG(argc, argv);

    Opticks ok(argc, argv);
    ok.configure(); 

    GGeo gg(&ok);
    gg.loadFromCache();
    gg.dumpStats();

    //test_GGeo(&gg);

    test_GGeo_sd(&gg);



    return 0 ;
}


