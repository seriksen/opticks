#include "AssimpGGeo.hh"
#include "AssimpTree.hh"
#include "AssimpNode.hh"

#include <assimp/types.h>
#include <assimp/scene.h>

#include "GVector.hh"
#include "GGeo.hh"
#include "GMesh.hh"
#include "GMaterial.hh"
#include "GBorderSurface.hh"
#include "GSkinSurface.hh"


AssimpGGeo::AssimpGGeo(AssimpTree* tree) 
   : 
   m_tree(tree),
   m_domain_scale(1.f),
   m_values_scale(1.f),
   m_domain_reciprocal(true),
   m_inborder_surface(0),
   m_outborder_surface(0),
   m_skin_surface(0)
{
    // see g4daenode.py as_optical_property_vector

    float hc_over_GeV = 1.2398424468024265e-06 ;  // h_Planck * c_light / GeV / nanometer #  (approx, hc = 1240 eV.nm )  
    float hc_over_MeV = hc_over_GeV*1000. ;
    //float hc_over_eV  = hc_over_GeV*1.e9 ;

    m_domain_scale = hc_over_MeV ; 
    m_values_scale = 1.0f ; 
}

AssimpGGeo::~AssimpGGeo()
{
}


GGeo* AssimpGGeo::convert(const char* ctrl)
{
    GGeo* gg = new GGeo();
    const aiScene* scene = m_tree->getScene();
    convertMaterials(scene, gg, ctrl);
    convertMeshes(scene, gg, ctrl);
    convertStructure(gg);
    return gg ;
}

void AssimpGGeo::addPropertyVector(GPropertyMap* pmap, const char* k, aiMaterialProperty* property )
{
    unsigned int nbyte  = property->mDataLength ; 
    unsigned int nfloat = nbyte/sizeof(float) ;
    assert(nfloat % 2 == 0 && nfloat > 1 );
    unsigned int npair  = nfloat/2 ;


    std::vector<float> vals ; 
    std::vector<float> domain  ; 

    float* data = (float*)property->mData ;

    // dont scale placeholder -1 : 1 domain ranges
    float dscale = data[0] > 0 && data[npair-1] > 0 ? m_domain_scale : 1.f ;   
    float vscale = m_values_scale ; 


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

    for(unsigned int i=0 ; i < npair ; i++)
    {
        float d0 = data[2*i] ; 
        float d = m_domain_reciprocal ? dscale/d0 : dscale*d0 ; 

        domain.push_back( noscale ? d0 : d );
        vals.push_back(   data[2*i+1]*vscale  );

        //if( noscale && ( i < 5 || i > npair - 5) )
        //printf("%4d %10.3e %10.3e \n", i, domain.back(), vals.back() );
    }
    pmap->AddProperty(k, vals.data(), domain.data(), vals.size() );
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


void AssimpGGeo::addProperties(GPropertyMap* pmap, aiMaterial* material)
{
    unsigned int numProperties = material->mNumProperties ;
    for(unsigned int i = 0; i < material->mNumProperties; i++)
    {
        aiMaterialProperty* property = material->mProperties[i] ;
        aiString key = property->mKey ; 
        const char* k = key.C_Str();

        // skip Assimp standard material props $clr.emissive $mat.shininess ?mat.name  etc..
        if( k[0] == '?' || k[0] == '$') continue ;   

        aiPropertyTypeInfo type = property->mType ; 
        if(type == aiPTI_Float)
        { 
            addPropertyVector(pmap, k, property);
        }
        else if( type == aiPTI_String )
        {
            aiString val ; 
            material->Get(k,0,0,val);
            const char* v = val.C_Str();
            //printf("skip k %s v %s \n", k, v );
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
const char* AssimpGGeo::g4dae_skinsurface_volume = "g4dae_skinsurface_volume" ;

void AssimpGGeo::convertMaterials(const aiScene* scene, GGeo* gg, const char* query)
{
    for(unsigned int i = 0; i < scene->mNumMaterials; i++)
    {
        aiMaterial* mat = scene->mMaterials[i] ;
        aiString name_;
        mat->Get(AI_MATKEY_NAME, name_);

        const char* name = name_.C_Str();

        if(strncmp(query, name, strlen(query))!=0) continue ;  

        const char* bspv1 = getStringProperty(mat, g4dae_bordersurface_physvolume1 );
        const char* bspv2 = getStringProperty(mat, g4dae_bordersurface_physvolume2 );
        const char* sslv  = getStringProperty(mat, g4dae_skinsurface_volume );

        if( sslv )
        {
            //printf("AssimpGGeo::convertMaterials materialIndex %u sslv %s  \n", i, sslv);
            GSkinSurface*  gss = new GSkinSurface(name, i);
            gss->setSkinSurface(sslv);
            addProperties(gss, mat);
            gg->add(gss);
        } 
        else if (bspv1 && bspv2 )
        {
            printf("AssimpGGeo::convertMaterials materialIndex %u\n    bspv1 %s\n    bspv2 %s \n", i, bspv1, bspv2 );
            GBorderSurface* gbs = new GBorderSurface(name, i);
            gbs->setBorderSurface(bspv1, bspv2);
            addProperties(gbs, mat);
            gg->add(gbs);
        }
        else
        {
            //printf("AssimpGGeo::convertMaterials materialIndex %u mt %s \n", i, name);
            GMaterial* gmat = new GMaterial(name, i);
            addProperties(gmat, mat);
            gg->add(gmat);
        }

        free((void*)bspv1);
        free((void*)bspv2);
        free((void*)sslv);
    }
}


void AssimpGGeo::convertMeshes(const aiScene* scene, GGeo* gg, const char* query)
{
    for(unsigned int i = 0; i < scene->mNumMeshes; i++)
    {

        aiMesh* mesh = scene->mMeshes[i] ;
        unsigned int numVertices = mesh->mNumVertices;

        aiVector3D* vertices = mesh->mVertices ; 
        gfloat3* gvertices = new gfloat3[numVertices];

        for(unsigned int v = 0; v < mesh->mNumVertices; v++)
        {
            gvertices[v].x = vertices[v].x;
            gvertices[v].y = vertices[v].y;
            gvertices[v].z = vertices[v].z;
        }


        unsigned int numFaces = mesh->mNumFaces;
        aiFace* faces = mesh->mFaces ; 
        guint3*  gfaces = new guint3[numFaces];

        for(unsigned int f = 0; f < mesh->mNumFaces; f++)
        {
            aiFace face = mesh->mFaces[f];
            gfaces[f].x = face.mIndices[0];
            gfaces[f].y = face.mIndices[1];
            gfaces[f].z = face.mIndices[2];
        }


        GMesh* gmesh = new GMesh( gvertices, numVertices, gfaces, numFaces ); 
        gg->add(gmesh);
    }
}


void AssimpGGeo::convertStructure(GGeo* gg)
{
    convertStructure(gg, m_tree->getRoot(), 0);
}

void AssimpGGeo::convertStructure(GGeo* gg, AssimpNode* node, unsigned int depth)
{
    convertStructureVisit( gg, node, depth);
    for(unsigned int i = 0; i < node->getNumChildren(); i++) convertStructure(gg, node->getChild(i), depth + 1);
}

void AssimpGGeo::convertStructureVisit(GGeo* gg, AssimpNode* node, unsigned int depth)
{
    //
    // rubber hits road here ...  analog to collada_to_chroma.py:visit 
    // find way to associate to solid with mesh and in/out materials/surfaces here  
    //
    //  need to find associations of nodes to:
    //
    //     * border surfaces, via pv pair names
    //
    //     * skin surfaces, via lv names
    //
    //     * outside/inside materials (parent/child assumption is expedient) 
    //       via aiNode lists of aiMesh indices [always 1 mesh per AssimpNode]
    //       (into scene meshes) 
    //       each aiMesh has single mMaterialIndex pointing to scene materials
    //


    AssimpNode* pnode = node->getParent();
    if(!pnode) pnode=node ; 

    // in/out materials ...

    unsigned int mti = node->getMaterialIndex() ;
    unsigned int mti_p = pnode->getMaterialIndex();

    GMaterial* mt   = gg->getMaterial(mti);
    GMaterial* mt_p = gg->getMaterial(mti_p);

    //if(node->getIndex() % 1000 == 0)
    //printf("AssimpGGeo::convertStructureVisit nodeIndex %u mti %u mti_p %u  mt %s mt_p %s \n", node->getIndex(), mti, mti_p, mt->getName(), mt_p->getName() );

    const char* lv   = node->getName(0); 
    const char* pv   = node->getName(1); 
    const char* pv_p = pnode->getName(1); 

    GSkinSurface* ss = gg->findSkinSurface(lv);
    if(ss)
    {
       //if(m_skin_surface % 1000 == 0) 
       //  printf("AssimpGGeo::convertStructureVisit ss# %u nodeIndex %u lv %s ss %p \n", m_skin_surface, node->getIndex(), lv, ss );
       m_skin_surface++ ; 
    }



    // NB sibling border surfaces not handled 
    //

    if(node->getIndex() == 0)
    printf("AssimpGGeo::convertStructureVisit border surface\n"); 

    GBorderSurface* ibs = gg->findBorderSurface(pv, pv_p);
    if(ibs)
    {
       printf("ibs# %u nodeIndex %u ibs %p idx %2d\n    pv   %s\n    pv_p %s\n", m_inborder_surface, node->getIndex(), ibs, ibs->getIndex(),pv, pv_p );
       m_inborder_surface++ ; 
    }

    GBorderSurface* obs = gg->findBorderSurface(pv_p, pv);
    if(obs)
    {
       printf("obs# %u nodeIndex %u obs %p idx %2d\n    pv_p %s\n    pv   %s\n", m_outborder_surface, node->getIndex(), obs, obs->getIndex(),pv_p, pv);
       m_outborder_surface++ ; 
    }




}




