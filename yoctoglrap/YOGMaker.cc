#include <iostream>
#include <iomanip>

#include "PLOG.hh"
#include "SVec.hh"
#include "BFile.hh"
#include "BStr.hh"
#include "NPY.hpp"

#include "YOG.hh"
#include "YOGMaker.hh"
#include "YOGGeometry.hh"

using ygltf::glTF_t ; 
using ygltf::node_t ; 
using ygltf::scene_t ; 
using ygltf::mesh_t ; 
using ygltf::material_t ; 
using ygltf::material_pbrMetallicRoughness_t ; 
using ygltf::mesh_primitive_t ; 
using ygltf::buffer_t ; 
using ygltf::buffer_data_t ; 
using ygltf::bufferView_t ; 
using ygltf::accessor_t ; 

namespace YOG {

void Maker::demo_create(const Geometry& geom)
{
    int vtx_b = add_buffer<float>(geom.vtx, "subfold/subsubfold/vtx.npy");
    int vtx_v = add_bufferView(vtx_b, ARRAY_BUFFER );
    int vtx_a = add_accessor( vtx_v, geom.count,  VEC3,  FLOAT );  
    set_accessor_min_max( vtx_a, geom.vtx_minf, geom.vtx_maxf );

    int idx_b = add_buffer<unsigned>(geom.idx, "subfold/subsubfold/idx.npy");
    int idx_v = add_bufferView(idx_b, ELEMENT_ARRAY_BUFFER );
    int idx_a = add_accessor( idx_v, geom.count,  SCALAR, UNSIGNED_INT );
    set_accessor_min_max( idx_a, geom.idx_minf, geom.idx_maxf );


    int red_mat   = add_material(1,0,0); 
    int green_mat = add_material(0,1,0); 
    int blue_mat  = add_material(0,0,1); 

    int red_mesh = add_mesh();
    int green_mesh = add_mesh();
    int blue_mesh = add_mesh();

    add_primitives_to_mesh( red_mesh, TRIANGLES, vtx_a, idx_a, red_mat );  
    add_primitives_to_mesh( green_mesh, TRIANGLES, vtx_a, idx_a, green_mat );  
    add_primitives_to_mesh( blue_mesh, TRIANGLES, vtx_a, idx_a, blue_mat );  


    int a = add_node();
    int b = add_node();
    int c = add_node();

    set_node_mesh(a, red_mesh);
    set_node_mesh(b, green_mesh);
    set_node_mesh(c, blue_mesh);
  
    set_node_translation(a,  1.f, 0.f, 0.f); 
    set_node_translation(b,  0.f, 1.f, 0.f); 
    set_node_translation(c,  0.f, 0.f, 1.f); 

    add_scene();

    append_node_to_scene( a );
    //append_node_to_scene( b );
    //append_node_to_scene( c );

    append_child_to_node( b,  a);
    append_child_to_node( c,  a);
}


Maker::Maker(Sc* sc_, bool saveNPYToGLTF_ )  
   :
   sc(sc_),
   gltf(new glTF_t()),
   saveNPYToGLTF(saveNPYToGLTF_),
   converted(false)
{
}

void Maker::convert()
{
    assert( converted == false );
    converted = true ; 

    add_scene();
    append_node_to_scene( sc->root );

    for(int i=0 ; i < sc->nodes.size() ; i++ )
    {
        Nd* nd = sc->nodes[i] ; 

        int n = add_node();
        node_t& node = get_node(n) ;
 
        node.name = nd->name ;  // pvName 
        node.mesh = nd->soIdx ; 
        node.children = nd->children ; 
        node.extras["boundary"] = nd->boundary ; 
    }

    for(int i=0 ; i < sc->meshes.size() ; i++ )
    {
        Mh* mh = sc->meshes[i] ; 

        // reddish : placeholder, material should be coming down the pike : not invented here
        // creating a material for every mesh, is just wrong... need 
        // to collect sc->materials and define the association in the YOG model, not here
        int mat = add_material(1,0,0);   
        int m = add_mesh(); 
        //////////////////////////////

        set_mesh_data( m,  mh,  mat ); 
    }
}

void Maker::set_mesh_data( int meshIdx, Mh* mh, int materialIdx )
{   
    mesh_t& mesh = get_mesh(meshIdx) ;
    mesh.name = mh->soName ;  // 
#ifdef TEMPORARY
    mesh.extras["csg.desc"] = mh->csg ? mh->csg->tag() : "-" ; 
#endif

    int vtx_a = set_mesh_data_vertices( mh );
    int idx_a = set_mesh_data_indices( mh );
 
    add_primitives_to_mesh( meshIdx, TRIANGLES, vtx_a, idx_a, materialIdx );  
}

std::string Maker::get_mesh_uri( Mh* mh, const char* bufname ) const 
{
    return BFile::FormRelativePath("extras", BStr::itoa(mh->lvIdx), bufname );
}

int Maker::set_mesh_data_vertices( Mh* mh )
{
    NPY<float>* vtx = mh->vtx ;
    std::string uri = get_mesh_uri( mh, "vertices.npy" );

    std::vector<float> f_min ; 
    std::vector<float> f_max ; 
    vtx->minmax(f_min,f_max);

    SVec<float>::Dump("vtx.fmin", f_min ); 
    SVec<float>::Dump("vtx.fmax", f_max ); 

    int vtx_b = add_buffer<float>(vtx, uri.c_str() );
    int vtx_v = add_bufferView(vtx_b, ARRAY_BUFFER );

    int vtx_a = add_accessor( vtx_v, vtx->getShape(0),  VEC3,  FLOAT );  
    assert( vtx->getNumElements() == 3 );

    set_accessor_min_max( vtx_a, f_min, f_max );

    return vtx_a ; 
}

int Maker::set_mesh_data_indices( Mh* mh )
{
    NPY<unsigned>* idx = mh->idx ;

    if(!idx->hasShape(-1,1))
    {
        std::string bef = idx->getShapeString() ; 
        idx->reshape(-1,1); 
        std::string aft = idx->getShapeString() ; 

        LOG(info) 
           << "reshaped idx buffer" 
           << " from " << bef 
           << " to " << aft 
           ; 
    } 

    std::string uri = get_mesh_uri( mh, "indices.npy" );

    std::vector<unsigned> u_min ; 
    std::vector<unsigned> u_max ; 
    idx->minmax(u_min,u_max);

    std::vector<float> f_min(u_min.begin(), u_min.end());
    std::vector<float> f_max(u_max.begin(), u_max.end());

    SVec<float>::Dump("idx.fmin", f_min ); 
    SVec<float>::Dump("idx.fmax", f_max ); 

    int idx_b = add_buffer<unsigned>(idx, uri.c_str() );
    int idx_v = add_bufferView(idx_b, ELEMENT_ARRAY_BUFFER );
    int idx_a = add_accessor( idx_v, idx->getShape(0),  SCALAR, UNSIGNED_INT );
    set_accessor_min_max( idx_a, f_min, f_max );
 
    return idx_a ; 
}

template <typename T> 
int Maker::add_buffer( NPY<T>* npy, const char* uri )
{
    LOG(info) << "add_buffer" 
              << " uri " << uri 
              << " shape " << npy->getShapeString()
              ;

    specs.push_back(npy->getBufferSpec()) ; 
    NBufferSpec& spec = specs.back() ;
    spec.uri = uri ; 
    assert( spec.ptr == (void*)npy );

    buffer_t bu ; 
    bu.uri = uri ; 
    bu.byteLength = spec.bufferByteLength ; 

    if( saveNPYToGLTF )
    {
        npy->saveToBuffer( bu.data ) ; 
    }

    int buIdx = gltf->buffers.size() ; 
    gltf->buffers.push_back( bu );

    return buIdx ; 
}

int Maker::add_bufferView( int bufferIdx, TargetType_t targetType )
{
    const NBufferSpec& spec = specs[bufferIdx] ;  

    bufferView_t bv ;  
    bv.buffer = bufferIdx  ;
    bv.byteOffset = spec.headerByteLength ; // offset to skip the NPY header 
    bv.byteLength = spec.dataSize() ;

    switch(targetType)
    {
        case ARRAY_BUFFER          : bv.target = bufferView_t::target_t::array_buffer_t         ; break ;
        case ELEMENT_ARRAY_BUFFER  : bv.target = bufferView_t::target_t::element_array_buffer_t ; break ;
    }
    bv.byteStride = 0 ; 

    int bufferViewIdx = gltf->bufferViews.size() ; 
    gltf->bufferViews.push_back( bv );
    return bufferViewIdx ;
}

int Maker::add_accessor( int bufferViewIdx, int count, Type_t type, ComponentType_t componentType ) 
{
    accessor_t ac ; 
    ac.bufferView = bufferViewIdx ; 
    ac.byteOffset = 0 ;
    ac.count = count ; 

    switch(type)
    {
        case SCALAR   : ac.type = accessor_t::type_t::scalar_t ; break ; 
        case VEC2     : ac.type = accessor_t::type_t::vec2_t   ; break ; 
        case VEC3     : ac.type = accessor_t::type_t::vec3_t   ; break ; 
        case VEC4     : ac.type = accessor_t::type_t::vec4_t   ; break ; 
        case MAT2     : ac.type = accessor_t::type_t::mat2_t   ; break ; 
        case MAT3     : ac.type = accessor_t::type_t::mat3_t   ; break ; 
        case MAT4     : ac.type = accessor_t::type_t::mat4_t   ; break ; 
    }

    switch(componentType)
    {
        case BYTE           : ac.componentType = accessor_t::componentType_t::byte_t           ; break ; 
        case UNSIGNED_BYTE  : ac.componentType = accessor_t::componentType_t::unsigned_byte_t  ; break ; 
        case SHORT          : ac.componentType = accessor_t::componentType_t::short_t          ; break ; 
        case UNSIGNED_SHORT : ac.componentType = accessor_t::componentType_t::unsigned_short_t ; break ; 
        case UNSIGNED_INT   : ac.componentType = accessor_t::componentType_t::unsigned_int_t   ; break ; 
        case FLOAT          : ac.componentType = accessor_t::componentType_t::float_t          ; break ; 
    }

    int accessorIdx = gltf->accessors.size() ; 
    gltf->accessors.push_back( ac );

    return accessorIdx ;
}

void Maker::set_accessor_min_max(int accessorIdx, const std::vector<float>& minf , const std::vector<float>& maxf )
{
    accessor_t& ac = get_accessor(accessorIdx);
    ac.min = minf ; 
    ac.max = maxf ; 
}

int Maker::add_mesh()
{
    int meshIdx = gltf->meshes.size() ; 
    mesh_t mh ; 
    gltf->meshes.push_back( mh );
    return meshIdx ;
}

int Maker::add_scene()
{
    int sceneIdx = gltf->scenes.size() ; 
    scene_t sc ; 
    gltf->scenes.push_back( sc );
    return sceneIdx ;
}

int Maker::add_node()
{
    int nodeIdx = gltf->nodes.size() ; 
    node_t nd ; 
    gltf->nodes.push_back( nd );
    return nodeIdx ;
}

void Maker::append_node_to_scene(int nodeIdx, int sceneIdx)
{
    scene_t& sc = get_scene(sceneIdx);
    sc.nodes.push_back(nodeIdx); 
}

void Maker::append_child_to_node(int childIdx, int nodeIdx ) 
{
    node_t& nd = get_node(nodeIdx);
    nd.children.push_back(childIdx); 
}

scene_t& Maker::get_scene(int idx)
{
    assert( idx < gltf->scenes.size() );
    return gltf->scenes[idx] ; 
}
mesh_t& Maker::get_mesh(int idx)
{
    assert( idx < gltf->meshes.size() );
    return gltf->meshes[idx] ; 
}
node_t& Maker::get_node(int idx)
{
    assert( idx < gltf->nodes.size() );
    return gltf->nodes[idx] ; 
}
accessor_t& Maker::get_accessor(int idx)
{
    assert( idx < gltf->accessors.size() );
    return gltf->accessors[idx] ; 
}



void Maker::add_primitives_to_mesh( int meshIdx, Mode_t mode, int positionIdx, int indicesIdx, int materialIdx )
{
    assert( meshIdx < gltf->meshes.size() );
    mesh_t& mh = gltf->meshes[meshIdx] ; 

    mesh_primitive_t mp ; 

    mp.attributes = {{"POSITION", positionIdx }} ; 
    mp.indices = indicesIdx  ; 
    mp.material = materialIdx ; 

    switch(mode)
    {
        case POINTS         :  mp.mode = mesh_primitive_t::mode_t::points_t         ; break ; 
        case LINES          :  mp.mode = mesh_primitive_t::mode_t::lines_t          ; break ; 
        case LINE_LOOP      :  mp.mode = mesh_primitive_t::mode_t::line_loop_t      ; break ; 
        case LINE_STRIP     :  mp.mode = mesh_primitive_t::mode_t::line_strip_t     ; break ; 
        case TRIANGLES      :  mp.mode = mesh_primitive_t::mode_t::triangles_t      ; break ; 
        case TRIANGLE_STRIP :  mp.mode = mesh_primitive_t::mode_t::triangle_strip_t ; break ; 
        case TRIANGLE_FAN   :  mp.mode = mesh_primitive_t::mode_t::triangle_fan_t   ; break ; 
    }
 
    mh.primitives.push_back(mp) ;
}

void Maker::set_node_mesh(int nodeIdx, int meshIdx)
{
    node_t& node = get_node(nodeIdx) ;  
    node.mesh = meshIdx ;  
}

void Maker::set_node_translation(int nodeIdx, float x, float y, float z)
{
    node_t& node = get_node(nodeIdx) ;  
    node.translation = {{ x, y, z }} ;  
}

int Maker::add_material(
     float baseColorFactor_r, 
     float baseColorFactor_g, 
     float baseColorFactor_b, 
     float baseColorFactor_a, 
     float metallicFactor, 
     float roughnessFactor 
    )
{
    material_pbrMetallicRoughness_t mr ; 
    mr.baseColorFactor =  {{ baseColorFactor_r, baseColorFactor_g, baseColorFactor_b, baseColorFactor_a }} ;
    mr.metallicFactor = metallicFactor ;
    mr.roughnessFactor = roughnessFactor ;

    material_t mt ; 
    mt.pbrMetallicRoughness = mr ; 

    int materialIdx = gltf->materials.size() ; 
    gltf->materials.push_back( mt );
    return materialIdx ;
}

void Maker::save(const char* path) const 
{
    assert( converted == true );

    bool save_bin = saveNPYToGLTF ; 
    bool save_shaders = false ; 
    bool save_images = false ; 

    bool createDirs = true ; 
    BFile::preparePath(path, createDirs); 

    save_gltf(path, gltf, save_bin, save_shaders, save_images);
    std::cout << "writing " << path << std::endl ; 

    std::ifstream fp(path);
    std::string line;
    while(std::getline(fp, line)) std::cout << line << std::endl ; 

    if(!save_bin)
        saveBuffers(path);
}

void Maker::saveBuffers(const char* path) const 
{
    // This can save buffers into non-existing subfolders, 
    // which it creates, unlike the ygltf buffer saving.

    LOG(info) << " path " << path ; 

    std::string dir = BFile::ParentDir(path);

    for(unsigned i=0 ; i < specs.size() ; i++)
    {
        const NBufferSpec& spec = specs[i]; 

        std::string bufpath_ = BFile::FormPath( dir.c_str(),  spec.uri.c_str() ); 
        const char* bufpath = bufpath_.c_str() ; 

        std::cout << std::setw(3) << i
                  << " "
                  << " spec.uri " << spec.uri 
                  << " bufpath " << bufpath 
                  << std::endl 
                  ;

        if(spec.ptr)
        {
            NPYBase* ptr = const_cast<NPYBase*>(spec.ptr);
            ptr->save(bufpath);
        }
    }
}

template YOG_API int Maker::add_buffer<float>(NPY<float>*, const char* );
template YOG_API int Maker::add_buffer<unsigned>(NPY<unsigned>*, const char* );


} // namespace



