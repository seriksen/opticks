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

#include <algorithm>
#include <iomanip>
#include <climits>

#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>

#include "SGDML.hh"

#include "BStr.hh"
#include "BFile.hh"

// npy-
#include "NSensorList.hpp"
#include "NSensor.hpp"

//okc-
#include "Opticks.hh"
#include "OpticksResource.hh"
#include "OpticksQuery.hh"

#include "GVector.hh"
#include "GMatrix.hh"

#include "GDomain.hh" 
#include "GAry.hh" 
#include "GProperty.hh" 
#include "GPropertyMap.hh" 

#include "GMaterial.hh"
#include "GBorderSurface.hh"
#include "GSkinSurface.hh"
#include "GOpticalSurface.hh"
#include "GGeoSensor.hh"

#include "GBndLib.hh"
#include "GSurfaceLib.hh"

#include "GMesh.hh"
#include "GVolume.hh"
#include "GGeo.hh"


#include "AssimpGGeo.hh"
#include "AssimpImporter.hh"
#include "AssimpTree.hh"
#include "AssimpSelection.hh"
#include "AssimpNode.hh"

#include <assimp/types.h>
#include <assimp/scene.h>



/*
        g4daeview.sh -g 3148:3155
        g4daeview.sh -g 4813:4816    Iws/SST/Oil  outside of SST high reflectivity 0.8, inside of SST low reflectivity 0.1
*/


// ctor macro facilitates GGeo staying ignorant of AssimpWrap and Assimp
#define GMATRIXF(m) \
           GMatrixF( \
                     (m).a1,(m).a2,(m).a3,(m).a4, \
                     (m).b1,(m).b2,(m).b3,(m).b4, \
                     (m).c1,(m).c2,(m).c3,(m).c4, \
                     (m).d1,(m).d2,(m).d3,(m).d4) \



#include "PLOG.hh"


AssimpGGeo::AssimpGGeo(GGeo* ggeo, AssimpTree* tree, AssimpSelection* selection, OpticksQuery* query) 
   : 
   m_ok(ggeo->getOpticks()),
   m_sensor_list(m_ok->getSensorList()),  
   m_ggeo(ggeo),
   m_tree(tree),
   m_selection(selection),
   m_query(query),
   m_nosel(query->isNoSelection()),
   m_domain_scale(1.f),
   m_values_scale(1.f),
   m_domain_reciprocal(true),
   m_skin_surface(0),
   m_doubleskin_surface(0),
   m_inborder_surface(0),
   m_outborder_surface(0),
   m_no_surface(0),
   m_volnames(m_ok->hasVolnames()),
   m_reverse(true),        // true: ascending wavelength ordering of properties
   m_cathode_amat(NULL),
   m_verbosity(0)
{
    init();
}


//bool AssimpGGeo::getVolNames()
//{
//    return m_volnames ; 
//}

void AssimpGGeo::setVerbosity(unsigned int verbosity)
{
    m_verbosity = verbosity ; 
}



void AssimpGGeo::init()
{
    // TODO: consolidate constant handling into okc-
    //       see also ggeo-/GConstant and probably elsewhere
    //
    // see g4daenode.py as_optical_property_vector

    double hc_over_GeV = 1.2398424468024265e-06 ;  // h_Planck * c_light / GeV / nanometer #  (approx, hc = 1240 eV.nm )  
    double hc_over_MeV = hc_over_GeV*1000. ;
    //float hc_over_eV  = hc_over_GeV*1.e9 ;

    m_domain_scale = static_cast<float>(hc_over_MeV) ; 
    m_values_scale = 1.0f ; 

}



int AssimpGGeo::load(GGeo* ggeo) // static
{
    // THIS IS THE ENTRY POINT SET IN OpticksGeometry::loadGeometryBase

    Opticks* ok = ggeo->getOpticks();
    OpticksResource* resource = ok->getResource();
    OpticksQuery* query = ok->getQuery() ;

    const char* path = ok->getDAEPath() ;
    const char* ctrl = resource->getCtrl() ;
    unsigned loadVerbosity = ok->getLoadVerbosity();
    unsigned importVerbosity = ok->getImportVerbosity();

    LOG(info)<< "AssimpGGeo::load "  
             << " path " << ( path ? path : "NULL" ) 
             << " query " << ( query ? query->getQueryString() : "NULL" )
             << " ctrl " << ( ctrl ? ctrl : "NULL" )
             << " importVerbosity " << importVerbosity 
             << " loaderVerbosity " << loadVerbosity 
             ; 


    bool exists = path ? BFile::ExistsFile(path ) : false ;
    if(!exists)
    {
        LOG(fatal) << " missing G4DAE path " << path ;  
        return 101 ; 
    }

    assert(path);
    assert(query);
    assert(ctrl);

    AssimpImporter assimp(path, importVerbosity);

    assimp.import();

    LOG(info) << "AssimpGGeo::load select START " ; 

    AssimpSelection* selection = assimp.select(query);

    LOG(info) << "AssimpGGeo::load select DONE  " ; 


    AssimpTree* tree = assimp.getTree();


    AssimpGGeo agg(ggeo, tree, selection, query); 

    agg.setVerbosity(loadVerbosity);

    int rc = agg.convert(ctrl);

    return rc ;
}


int AssimpGGeo::convert(const char* ctrl)
{
    LOG(info) << "AssimpGGeo::convert ctrl " << ctrl ; 

    const aiScene* scene = m_tree->getScene();

    convertMaterials(scene, m_ggeo, ctrl );
    m_ggeo->afterConvertMaterials(); 

    convertSensors( m_ggeo ); 
    convertMeshes(scene, m_ggeo, ctrl);

    convertStructure(m_ggeo);

    return 0 ;
}

void AssimpGGeo::addPropertyVector(GPropertyMap<float>* pmap, const char* k, aiMaterialProperty* property )
{
    const char* shortname = pmap->getShortName();

    LOG(debug) << "AssimpGGeo::addPropertyVector " 
              << " shortname " << (shortname ? shortname : "-" )
              << " k " << k 
               ; 

    float* data = (float*)property->mData ;
    unsigned int nbyte  = property->mDataLength ; 
    unsigned int nfloat = nbyte/sizeof(float) ;
    assert(nfloat % 2 == 0 && nfloat > 1 );
    unsigned int npair  = nfloat/2 ;


    // dont scale placeholder -1 : 1 domain ranges
    double dscale = data[0] > 0 && data[npair-1] > 0 ? m_domain_scale : 1.f ;   
    double vscale = m_values_scale ; 

    // debug some funny domains : default zeros coming from somewhere 
    bool noscale =     ( pmap->isSkinSurface() 
                            && 
                         ( 
                            strcmp(k, "RINDEX") == 0 
                         )
                       )
                    || 
                       ( pmap->isBorderSurface() 
                             && 
                         ( 
                            strcmp(k, "BACKSCATTERCONSTANT") == 0 
                            ||
                            strcmp(k, "SPECULARSPIKECONSTANT") == 0 
                         )
                       ) ;   


    //if(noscale) 
    //    printf("AssimpGGeo::addPropertyVector k %-35s nbyte %4u nfloat %4u npair %4u \n", k, nbyte, nfloat, npair);

    std::vector<float> vals ; 
    std::vector<float> domain  ; 

    for( unsigned int i = 0 ; i < npair ; i++ ) 
    {
        double d0 = data[2*i] ; 
        double d = m_domain_reciprocal ? dscale/d0 : dscale*d0 ; 
        double v = data[2*i+1]*vscale  ;

        double dd = noscale ? d0 : d ; 

        domain.push_back( static_cast<float>(dd) );
        vals.push_back( static_cast<float>(v) );  

        //if( noscale && ( i < 5 || i > npair - 5) )
        //printf("%4d %10.3e %10.3e \n", i, domain.back(), vals.back() );
    }

    if(m_reverse)
    {
       std::reverse(vals.begin(), vals.end());
       std::reverse(domain.begin(), domain.end());
    }

    pmap->addProperty(k, vals.data(), domain.data(), vals.size() );
}




const char* AssimpGGeo::getStringProperty(aiMaterial* material, const char* query)
{
    for(unsigned int i = 0; i < material->mNumProperties; i++)
    {
        aiMaterialProperty* property = material->mProperties[i] ;
        aiString key = property->mKey ; 
        const char* k = key.C_Str();

        // skip Assimp standard material props $clr.emissive $mat.shininess ?mat.name  etc..
        if( k[0] == '?' || k[0] == '$') continue ;   

        aiPropertyTypeInfo type = property->mType ; 
        if(type == aiPTI_String)
        {
           aiString val ; 
           material->Get(k,0,0,val);
           const char* v = val.C_Str();
           if(strncmp(k, query, strlen(query))==0 ) return strdup(v) ;
        }
    }
    return NULL ;
}



bool AssimpGGeo::hasVectorProperty(aiMaterial* material, const char* propname)
{
    aiMaterialProperty* prop = getVectorProperty(material, propname);
    return prop != NULL ; 
}

aiMaterialProperty* AssimpGGeo::getVectorProperty(aiMaterial* material, const char* propname )
{
    aiMaterialProperty* ret = NULL ;
    //unsigned int numProperties = material->mNumProperties ;
    for(unsigned int i = 0; i < material->mNumProperties; i++)
    {
        aiMaterialProperty* property = material->mProperties[i] ;
        aiString key = property->mKey ; 
        const char* k = key.C_Str();

        if( k[0] == '?' || k[0] == '$') continue ;   

        aiPropertyTypeInfo type = property->mType ; 
        if(type == aiPTI_Float && strncmp(k, propname, strlen(k))==0)
        { 
            ret = property ; 
            break ; 
        }
    }
    return ret ; 
}




void AssimpGGeo::addProperties(GPropertyMap<float>* pmap, aiMaterial* material )
{
    //unsigned int numProperties = material->mNumProperties ;
    for(unsigned int i = 0; i < material->mNumProperties; i++)
    {
        aiMaterialProperty* property = material->mProperties[i] ;
        aiString key = property->mKey ; 
        const char* k = key.C_Str();

        // skip Assimp standard material props $clr.emissive $mat.shininess ?mat.name  etc..
        if( k[0] == '?' || k[0] == '$') continue ;   

        //printf("AssimpGGeo::addProperties i %d k %s \n", i, k ); 

        aiPropertyTypeInfo type = property->mType ; 
        if(type == aiPTI_Float)
        { 
            addPropertyVector(pmap, k, property );
        }
        else if( type == aiPTI_String )
        {
            aiString val ; 
            material->Get(k,0,0,val);
            //const char* v = val.C_Str();
            //printf("skip k %s v %s \n", k, v ); needs props are plucked elsewhere
        }
        else
        {
            printf("skip k %s \n", k);
        }
    }
    //printf("addProperties props %2d %s \n", numProperties, pmap->getName());
}

void AssimpGGeo::setDomainScale(float domain_scale)
{
    m_domain_scale = domain_scale ; 
}
void AssimpGGeo::setValuesScale(float values_scale)
{
    m_values_scale = values_scale  ; 
}




const char* AssimpGGeo::g4dae_bordersurface_physvolume1 = "g4dae_bordersurface_physvolume1" ;
const char* AssimpGGeo::g4dae_bordersurface_physvolume2 = "g4dae_bordersurface_physvolume2" ;
const char* AssimpGGeo::g4dae_skinsurface_volume        = "g4dae_skinsurface_volume" ;

const char* AssimpGGeo::g4dae_opticalsurface_name       = "g4dae_opticalsurface_name" ;
const char* AssimpGGeo::g4dae_opticalsurface_type       = "g4dae_opticalsurface_type" ;
const char* AssimpGGeo::g4dae_opticalsurface_model      = "g4dae_opticalsurface_model" ;
const char* AssimpGGeo::g4dae_opticalsurface_finish     = "g4dae_opticalsurface_finish" ;
const char* AssimpGGeo::g4dae_opticalsurface_value      = "g4dae_opticalsurface_value" ;

const char* AssimpGGeo::g4dae_material_srcidx = "g4dae_material_srcidx" ;



const char* AssimpGGeo::EFFICIENCY = "EFFICIENCY" ; 


void AssimpGGeo::convertMaterials(const aiScene* scene, GGeo* gg, const char* query )
{
    LOG(info)<<"AssimpGGeo::convertMaterials " 
             << " query " << query 
             << " mNumMaterials " << scene->mNumMaterials  
             ;


    GBndLib* blib = gg->getBndLib() ; 
    assert( blib ); 
    GDomain<float>* standard_domain = blib->getStandardDomain(); 


    for(unsigned int i = 0; i < scene->mNumMaterials; i++)
    {
        unsigned int index = i ;  // hmm, make 1-based later 

        aiMaterial* mat = scene->mMaterials[i] ;

        aiString name_;
        mat->Get(AI_MATKEY_NAME, name_);

        const char* name0 = name_.C_Str();
        const char* name = BStr::NumField( name0, ':') == 3 ? BStr::GetField( name0, ':', 2 ) : name0 ; 

        //if(strncmp(query, name, strlen(query))!=0) continue ;  

        const char* srcidx_ = getStringProperty(mat, g4dae_material_srcidx );
        int srcidx = BStr::atoi( srcidx_ ) ; 

        LOG(debug) << " i " << i 
                  << " name " << name 
                  << " srcidx " << srcidx 
                  ;

        const char* bspv1 = getStringProperty(mat, g4dae_bordersurface_physvolume1 );
        const char* bspv2 = getStringProperty(mat, g4dae_bordersurface_physvolume2 );

        const char* sslv  = getStringProperty(mat, g4dae_skinsurface_volume );

        const char* osnam = getStringProperty(mat, g4dae_opticalsurface_name );
        const char* ostyp = getStringProperty(mat, g4dae_opticalsurface_type );
        const char* osmod = getStringProperty(mat, g4dae_opticalsurface_model );
        const char* osfin = getStringProperty(mat, g4dae_opticalsurface_finish );
        const char* osval = getStringProperty(mat, g4dae_opticalsurface_value );

        bool is_os = osnam && ostyp && osmod && osfin && osval ;     

        if(is_os)
        {
            float fraction = boost::lexical_cast<float>(osval) ; 
            assert( fraction >= 0.f && fraction <= 1.f ); 
        }

        GOpticalSurface* os = is_os ? new GOpticalSurface(osnam, ostyp, osmod, osfin, osval) : NULL ; 

        if(is_os)
            LOG(error) 
            << " osnam " << std::setw(80) << osnam 
            << " ostyp " << ostyp
            << " osmod " << osmod
            << " osfin " << osfin
            << " osval " << osval
            ;




        // assimp "materials" are used to hold skinsurface and bordersurface properties, 
        // as well as material properties
        // which is which is determined by the properties present 

        if(os)
        {
            LOG(debug) << "AssimpGGeo::convertMaterials os " << i << " " << os->description(); 

            // assert(strcmp(osnam, name) == 0); 
            //      formerly enforced same-name convention between OpticalSurface 
            //      and the skin or border surface that references it, but JUNO doesnt follow that  
        }

        if( sslv )
        {
            assert(os && "all ss must have associated os");

            GSkinSurface* gss = new GSkinSurface(name, index, os);


            LOG(debug) << "GSkinSurface " 
                      << " name " << name 
                      << " sslv " << sslv 
                      ; 

            gss->setStandardDomain(standard_domain);
            gss->setSkinSurface(sslv);
            addProperties(gss, mat );

            LOG(debug) << gss->description(); 
            gg->add(gss);

            {
                // without standard domain applied
                GSkinSurface*  gss_raw = new GSkinSurface(name, index, os);
                gss_raw->setSkinSurface(sslv);
                addProperties(gss_raw, mat );
                gg->addRaw(gss_raw); 
            }   

        } 
        else if (bspv1 && bspv2 )
        {
            assert(os && "all bs must have associated os");
            GBorderSurface* gbs = new GBorderSurface(name, index, os);

            gbs->setStandardDomain(standard_domain);
            gbs->setBorderSurface(bspv1, bspv2);
            addProperties(gbs, mat );

            LOG(debug) << gbs->description(); 

            gg->add(gbs);

            {
                // without standard domain applied
                GBorderSurface* gbs_raw = new GBorderSurface(name, index, os);
                gbs_raw->setBorderSurface(bspv1, bspv2);
                addProperties(gbs_raw, mat );
                gg->addRaw(gbs_raw);
            }
        }
        else
        {
            assert(os==NULL);


            //printf("AssimpGGeo::convertMaterials aiScene materialIndex %u (GMaterial) name %s \n", i, name);
            GMaterial* gmat = new GMaterial(name, index);
            gmat->setMetaKV<int>("srcidx", srcidx);  
            gmat->setStandardDomain(standard_domain);
            addProperties(gmat, mat );
            gg->add(gmat);

            {
                // without standard domain applied
                GMaterial* gmat_raw = new GMaterial(name, index);
                addProperties(gmat_raw, mat );
                gg->addRaw(gmat_raw);
            }

            if(hasVectorProperty(mat, EFFICIENCY ))
            {
                gg->setCathode(gmat) ;  
                m_cathode_amat = mat ; 
            }

        }



        free((void*)bspv1);
        free((void*)bspv2);
        free((void*)sslv);

        free((void*)osnam);
        free((void*)ostyp);
        free((void*)osfin);
        free((void*)osmod);
        free((void*)osval);

    }


}


/**
AssimpGGeo::convertSensors
---------------------------

Invoked by AssimpGGeo::convert after convertMaterials (which collects 
surfaces as well as materials).

Opticks is a surface based simulation, as opposed to 
Geant4 which is CSG volume based. In Geant4 hits are formed 
on stepping into volumes with associated SensDet.
The Opticks equivalent is intersecting with a "SensorSurface", 
which are fabricated by AssimpGGeo::convertSensors.


Need this for direct geometry route, so have to relocate 
this into GGeo.

**/

void AssimpGGeo::convertSensors(GGeo* gg)
{
    assert( m_sensor_list ) ; 

    if(!m_cathode_amat )
    {
         LOG(warning) << "AssimpGGeo::convertSensors m_cathode_amat NULL : no aiMaterial with an efficiency property ?  " ;
         //assert(0); 
         return ; 
    }
    if(!gg->getCathode() )
    {
         LOG(warning) << "AssimpGGeo::convertSensors m_cathode_gmat NULL : no material with an efficiency property ?  " ;
         assert(0); 
         return ; 
    }


    gg->getCathode()->Summary();


    convertSensors( gg, m_tree->getRoot(), 0); 

    unsigned int nclv = gg->getNumCathodeLV();

    LOG(info) << "AssimpGGeo::convertSensors"
              << " nclv " << nclv
              ;

    assert( nclv > 0 ); 


    // DYB: nclv=2 for hemi and headon PMTs 
    for(unsigned int i=0 ; i < nclv ; i++)
    {
        const char* sslv = gg->getCathodeLV(i);
        unsigned int index = gg->getNumMaterials() + gg->getNumSkinSurfaces() + gg->getNumBorderSurfaces() ; 
        // standard materials/surfaces use the originating aiMaterial index, 
        // extend that for fake SensorSurface by toting up all 

        LOG(info) << "AssimpGGeo::convertSensors" 
                  << " i " << i
                  << " sslv " << sslv 
                  << " index " << index 
                  ;


        GSkinSurface* gss = GGeoSensor::MakeSensorSurface(sslv, index);
        gss->setStandardDomain() ;  // default domain 
        gss->setSensor();

        addProperties(gss, m_cathode_amat );

        LOG(info) << "AssimpGGeo::convertSensors gss " << gss->description(); 
 
        gg->add(gss);

        {
            // without standard domain applied
            GSkinSurface* gss_raw = GGeoSensor::MakeSensorSurface(sslv, index);
            // not setting sensor, only the standardized need that
            addProperties(gss_raw, m_cathode_amat );
            gg->addRaw(gss_raw);
        }   
    }
}

/**
AssimpGGeo::convertSensors TODO: rename to collectSensorLV
------------------------------------------------------------

Recursively searches for volumes with nodeIndex that 
is on the m_sensor_list and which have the cathode material 
(with an EFFICIENCY property).  The lv name of such volumes
are collected into GGeo with addLVSD.

**/

void AssimpGGeo::convertSensors(GGeo* gg, AssimpNode* node, unsigned int depth)
{
    // addLVSD into gg
    convertSensorsVisit(gg, node, depth);
    for(unsigned int i = 0; i < node->getNumChildren(); i++) convertSensors(gg, node->getChild(i), depth + 1);
}

void AssimpGGeo::convertSensorsVisit(GGeo* gg, AssimpNode* node, unsigned int depth)
{
    unsigned int nodeIndex = node->getIndex();
    const char* lv_dae   = node->getName(0); 
    bool trimPtr = false ; 
    const char* lv = BStr::DAEIdToG4(lv_dae, trimPtr ); 

    LOG(debug) 
        << " lv_dae " << lv_dae 
        << " lv " << lv
        ;


    //const char* pv   = node->getName(1); 
    unsigned int mti = node->getMaterialIndex() ;
    GMaterial* mt = gg->getMaterial(mti);
    assert( mt );
    
    /*
    NSensor* sensor0 = sens->getSensor( nodeIndex ); 
    NSensor* sensor1 = sens->findSensorForNode( nodeIndex ); 
    assert(sensor0 == sensor1);
    // these do not match
    */

    NSensor* sensor = m_sensor_list ? m_sensor_list->findSensorForNode( nodeIndex ) : NULL ; 

    GMaterial* cathode = gg->getCathode() ; 

    const char* cathode_material_name = gg->getCathodeMaterialName() ;
    const char* name = mt->getName() ; 
    bool name_match = strcmp(name, cathode_material_name) == 0 ;  
    bool ptr_match = mt == cathode ;   // <--- always false 
 
    //const char* sd = "SD_AssimpGGeo" ; 
    const char* sd = "SD0" ; 

    if(sensor && name_match)
    {
         LOG(debug) << "AssimpGGeo::convertSensorsVisit " 
                   << " depth " << depth 
                   << " lv " << lv  
                   << " sd " << sd 
                   << " ptr_match " << ptr_match 
                   ;
         gg->addLVSD(lv, sd) ;   
    }
}


GMesh* AssimpGGeo::convertMesh(const aiMesh* mesh, unsigned int index )
{
    const char* meshname = mesh->mName.C_Str() ; 
    unsigned int numVertices = mesh->mNumVertices;
    unsigned int numFaces = mesh->mNumFaces;

    LOG(debug) << "AssimpGGeo::convertMesh " 
               << " index " << std::setw(4) << index
               << " v " << std::setw(4) << numVertices
               << " f " << std::setw(4) << numFaces
               << " n " << meshname 
               ; 


    aiVector3D* vertices = mesh->mVertices ; 
    gfloat3*   gvertices = new gfloat3[numVertices];
    for(unsigned int v = 0; v < numVertices; v++)
    {
        gvertices[v].x = vertices[v].x;
        gvertices[v].y = vertices[v].y;
        gvertices[v].z = vertices[v].z;
    }

    assert(mesh->HasNormals()); 
    aiVector3D* normals = mesh->mNormals ; 
    gfloat3* gnormals  = new gfloat3[numVertices];
    for(unsigned int v = 0; v < mesh->mNumVertices; v++)
    {
        gnormals[v].x = normals[v].x;
        gnormals[v].y = normals[v].y;
        gnormals[v].z = normals[v].z;
    }

    //aiFace* faces = mesh->mFaces ; 
    guint3*  gfaces = new guint3[numFaces];
    for(unsigned int f = 0; f < mesh->mNumFaces; f++)
    {
        aiFace face = mesh->mFaces[f];
        gfaces[f].x = face.mIndices[0];
        gfaces[f].y = face.mIndices[1];
        gfaces[f].z = face.mIndices[2];
    }

    gfloat2* gtexcoords = NULL ;

    GMesh* gmesh = new GMesh( index, gvertices, numVertices, gfaces, numFaces, gnormals, gtexcoords); 
    return gmesh ; 
}




unsigned AssimpGGeo::getNumMeshes()
{
    const aiScene* scene = m_tree->getScene();
    return scene->mNumMeshes ;
}

GMesh* AssimpGGeo::convertMesh(const char* qname)
{
    LOG(debug) << "AssimpGGeo::convertMesh qname " << qname ; 
 
    const aiScene* scene = m_tree->getScene();
    GMesh* gmesh = NULL ;  
    for(unsigned int i = 0; i < scene->mNumMeshes; i++)
    {
        aiMesh* mesh = scene->mMeshes[i] ;
        const char* meshname = mesh->mName.C_Str() ; 
        std::string tmeshname = BStr::trimPointerSuffixPrefix(meshname, NULL );

        //LOG(verbose) << "AssimpGGeo::convertMesh" << " tmeshname " << tmeshname ; 

        if(tmeshname.compare(qname)==0) 
        {
            gmesh = convertMesh(i);
            break ;
        }
    }
    return gmesh ; 
}


GMesh* AssimpGGeo::convertMesh(unsigned int index )
{
    const aiScene* scene = m_tree->getScene();
    assert(index < scene->mNumMeshes);
    aiMesh* mesh = scene->mMeshes[index] ;
    GMesh* graw = convertMesh(mesh, index );
    return graw ; 
}


void AssimpGGeo::convertMeshes(const aiScene* scene, GGeo* gg, const char* /*query*/)
{
    LOG(info)<< "AssimpGGeo::convertMeshes NumMeshes " << scene->mNumMeshes ;

    for(unsigned int i = 0; i < scene->mNumMeshes; i++)
    {
        aiMesh* mesh = scene->mMeshes[i] ;
    
        const char* meshname = mesh->mName.C_Str() ; 

        GMesh* graw = convertMesh(mesh, i );

        GMesh* gmesh = graw->makeDedupedCopy(); // removes duplicate vertices, re-indexing faces accordingly

        delete graw ; 

        gmesh->setName(meshname);

        GMesh* gfixed = gg->invokeMeshJoin(gmesh);  
 
        assert(gfixed) ; 

        if(gfixed != gmesh)
        {
            LOG(verbose) << "AssimpGGeo::convertMeshes meshfixing was done for "
                        << " meshname " << meshname 
                        << " index " << i 
                         ; 

            delete gmesh ;
        }

        gg->add(gfixed);

    }
}



void AssimpGGeo::convertStructure(GGeo* gg)
{
    LOG(info) << "AssimpGGeo::convertStructure, closing GGeo ";

    m_ggeo->close(); 

    assert( m_ggeo == gg ) ;  

    gg->dumpSkinSurface();

    convertStructure(gg, m_tree->getRoot(), 0, NULL);

    LOG(info) << "AssimpGGeo::convertStructure found surfaces "
              << " skin " << m_skin_surface 
              << " doubleskin " << m_doubleskin_surface 
              << " outborder " << m_outborder_surface 
              << " inborder " << m_inborder_surface 
              << " no " << m_no_surface  ;
}

void AssimpGGeo::convertStructure(GGeo* gg, AssimpNode* node, unsigned int depth, GVolume* parent)
{
    // recursive traversal of the AssimpNode tree
    // note that full tree is traversed even when a partial selection is applied 

    GVolume* volume = convertStructureVisit( gg, node, depth, parent);

    bool selected = m_nosel ? true : m_selection && m_selection->contains(node) ;   // twas hotspot for geocache creation before nosel special case

    volume->setSelected(selected);

    gg->add(volume);

    if(parent) // GNode hookup
    {
        parent->addChild(volume);
        volume->setParent(parent);
    }
    else
    {
        assert(node->getIndex() == 0);   // only root node has no parent 
    }

    for(unsigned int i = 0; i < node->getNumChildren(); i++) convertStructure(gg, node->getChild(i), depth + 1, volume);
}




GVolume* AssimpGGeo::convertStructureVisit(GGeo* gg, AssimpNode* node, unsigned int depth, GVolume* /*parent*/)
{
    // Associates node to extra information analogous to collada_to_chroma.py:visit
    //
    // * outside/inside materials (parent/child assumption is expedient) 
    // * border surfaces, via pv pair names
    // * skin surfaces, via lv names
    //
    // Volume-centric naming 
    //
    // outer-surface 
    //      relevant to inwards going photons, from parent to self
    //
    // inner-surface
    //      relevant to outwards going photons, from self to parent  
    // 
    //
    // Skinsurface are not always leaves 
    // (UnstStainlessSteel cable trays with single child BPE inside). 
    // Nevetheless treat skinsurface like an outer border surface.
    //
    // NB sibling border surfaces are not handled, but there are none of these 
    //


    //AssimpNode* cnode = node->getChild(0);   // first child, if any
    AssimpNode* pnode = node->getParent();
    if(!pnode) pnode=node ; 

    unsigned int nodeIndex = node->getIndex();
    bool dbg = false ; // nodeIndex >= 3199 && nodeIndex <=3203 ; 

    aiMatrix4x4 g = node->getGlobalTransform();
    GMatrixF* gtransform = new GMATRIXF(g) ;

    aiMatrix4x4 l = node->getLevelTransform(-2); // -1 is always identity 
    GMatrixF* ltransform = new GMATRIXF(l) ; 
    unsigned int msi = node->getMeshIndex();
    const GMesh* mesh = gg->getMesh(msi);

    if(!mesh)
    {
       LOG(fatal) << "AssimpGGeo::convertStructureVisit NULL mesh"
                  << " depth " << depth 
                  << " msi " << msi 
                  ;

    }
    assert(mesh);


    unsigned int mti = node->getMaterialIndex() ;
    GMaterial* mt = gg->getMaterial(mti);

    unsigned int mti_p = pnode->getMaterialIndex();
    GMaterial* mt_p = gg->getMaterial(mti_p);


    const char* omat = mt_p->getShortName() ; 
    const char* imat = mt->getShortName() ; 
   // bool problem_pair  = strcmp(omat, "UnstStainlessSteel") == 0 && strcmp(imat, "BPE") == 0 ;


    //printf("AssimpGGeo::convertStructureVisit nodeIndex %d (mti %u mt %p) (mti_p %u mt_p %p) (msi %u mesh %p) \n", nodeIndex, mti, mt, mti_p, mt_p,  msi, mesh  );
    if(dbg || m_verbosity > 1)
    LOG(info) << "AssimpGGeo::convertStructureVisit" 
               << " nodeIndex " << std::setw(6) << nodeIndex
               << " ( mti " << std::setw(4) << mti << " mt " << (void*)mt << " ) "
               << std::setw(20) << ( mt ? mt->getShortName() : "-" ) 
               << " ( mti_p " << std::setw(4) << mti_p << " mt_p " << (void*)mt_p << " ) " 
               << std::setw(20) << ( mt_p ? mt_p->getShortName() : "-" )
               << " ( msi " << std::setw(4) << msi << " mesh " << (void*)mesh << " ) " << ( mesh ? mesh->getName() : "-" )
               ;  


    if(m_verbosity > 1)
    gtransform->Summary("AssimpGGeo::convertStructureVisit gtransform");

    if(m_verbosity > 1)
    ltransform->Summary("AssimpGGeo::convertStructureVisit ltransform");


    GVolume* volume = new GVolume(nodeIndex, gtransform, mesh ); 
    volume->setLevelTransform(ltransform);

    const char* lv   = node->getName(0); 
    const char* pv   = node->getName(1); 

    const char* lv_p   = pnode->getName(0); 
    const char* pv_p   = pnode->getName(1); 

    gg->countMeshUsage(msi, nodeIndex);

    GPropertyMap<float>* isurf  = NULL ; 
    GPropertyMap<float>* osurf  = NULL ; 

    GBorderSurface* obs = gg->findBorderSurface(pv_p, pv);  // outer surface (parent->self) 
    GBorderSurface* ibs = gg->findBorderSurface(pv, pv_p);  // inner surface (self->parent) 
    GSkinSurface*   sks = gg->findSkinSurface(lv);          
    GSkinSurface*   sks_p = gg->findSkinSurface(lv_p);   
    // dont like sks_p : but it seems to correspond with G4OpBoundary surface resolution see notes/issues/ab-blib.rst

    unsigned int nsurf = 0 ;
    if(sks) nsurf++ ;
    if(ibs) nsurf++ ;
    if(obs) nsurf++ ;
    assert(nsurf == 0 || nsurf == 1 || nsurf == 2); 

    // see notes/issues/ab-blib.rst 

    if(obs)
    {
        osurf = obs ; 
        isurf = NULL ; 
        m_outborder_surface++ ; 
    }
    else if(ibs)
    {
        osurf = NULL ; 
        isurf = ibs ; 
        m_inborder_surface++ ; 
    }
    else if(sks && !sks_p)
    {
        osurf = sks ; 
        isurf = sks ;      
        m_skin_surface++ ; 
    }
    else if(!sks && sks_p )  
    {
        osurf = sks_p ; 
        isurf = sks_p ;     
        m_skin_surface++ ; 
    }
    else if(sks && sks_p ) // doubleskin : not yet seen in wild 
    {
        assert(0); 
        bool swap = false ;    // unsure ... needs some testing vs G4
        osurf = swap ? sks_p : sks   ; 
        isurf = swap ? sks   : sks_p ; 
        m_doubleskin_surface++ ; 
    }
    else
    {
        m_no_surface++ ;
    }


    NSensor* sensor = m_sensor_list ? m_sensor_list->findSensorForNode( nodeIndex ) : NULL ; 
    volume->setSensor( sensor );  

    GBndLib* blib = gg->getBndLib();  
    GSurfaceLib* slib = gg->getSurfaceLib();  


    // boundary identification via 4-uint 
    unsigned boundary = blib->addBoundary( 
                                  omat,
                                  osurf ? osurf->getShortName() : NULL ,
                                  isurf ? isurf->getShortName() : NULL ,
                                  imat
                                  );

    volume->setBoundary(boundary);
    {
       // sensor indices are set even for non sensitive volumes in PMT viscinity
       // TODO: change that 
       // this is a workaround that requires an associated sensitive surface
       // in order for the index to be provided

        unsigned int surface = blib->getOuterSurface(boundary);
        bool oss = slib->isSensorSurface(surface); 
        unsigned int ssi = oss ? NSensor::RefIndex(sensor) : 0 ; 
        volume->setSensorSurfaceIndex( ssi ); 
    } 

    char* desc = node->getDescription("\n\noriginal node description"); 
    volume->setDescription(desc);
    volume->setName(node->getName());  // this is LV name, maybe set PV name too : actully PV name would make more sense

    if(m_volnames)
    {
        volume->setPVName(pv);
        volume->setLVName(lv);
    }
    return volume ; 
}



/*

Why does this show up with isurf rather than osurf ?


GBoundaryLib boundary index 16 
imat material 59 __dd__Materials__MineralOil0xbf5c830
ABSLENGTH
   0    899.871    219.400
   1    898.892    236.700
   2    897.916    257.300
   3    896.877    278.000
   4    895.905    292.700
 539    190.977     10.800
 540    189.976     11.100
 541    120.023     11.100
 542     79.990     11.100
RAYLEIGH
   0    799.898 500000.000
   1    699.922 300000.000
   2    589.839 170000.000
   3    549.819 100000.000
   4    489.863  62000.000
   7    299.986   7600.000
   8    199.975    850.000
   9    120.023    850.000
  10     79.990    850.000
RINDEX
   0    799.898      1.456
   1    690.701      1.458
   2    589.002      1.462
   3    546.001      1.464
   4    486.001      1.468
  14    139.984      1.642
  15    129.990      1.534
  16    120.023      1.434
  17     79.990      1.434
isurf bordersurface 4 __dd__Geometry__AdDetails__AdSurfacesAll__SSTOilSurface
REFLECTIVITY
   0    799.898      0.100
   1    199.975      0.100
   2    120.023      0.100
   3     79.990      0.100




Many instances of skin surfaces with differing names but the same 
property values are causing total of 73 different boundarys ...


GBoundaryLib boundary index 62 
imat material 75 __dd__Materials__UnstStainlessSteel0xc5c11e8
ABSLENGTH
   0    799.898      0.001
   1    199.975      0.001
   2    120.023      0.001
   3     79.990      0.001
osurf skinsurface 38 __dd__Geometry__PoolDetails__PoolSurfacesAll__UnistrutRib5Surface
REFLECTIVITY
   0    826.562      0.400
   1    190.745      0.400
RINDEX
   0      0.000      0.000
   1      0.000      0.000
GBoundaryLib boundary index 63 
imat material 75 __dd__Materials__UnstStainlessSteel0xc5c11e8
ABSLENGTH
   0    799.898      0.001
   1    199.975      0.001
   2    120.023      0.001
   3     79.990      0.001
osurf skinsurface 37 __dd__Geometry__PoolDetails__PoolSurfacesAll__UnistrutRib4Surface
REFLECTIVITY
   0    826.562      0.400
   1    190.745      0.400
RINDEX
   0      0.000      0.000
   1      0.000      0.000



Adjust identity to be based on a property name and hash alone ?



*/
