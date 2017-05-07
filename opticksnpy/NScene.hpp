#pragma once
#include "NPY_API_EXPORT.hh"
#include <string>
#include <map>
#include <vector>

NPY_API static std::string xform_string( const std::array<float, 16>& xform );


namespace ygltf 
{
    struct glTF_t ;  
    struct fl_gltf ;
}


class NPY_API NScene 
{
    public:
        static NScene* load(const char* base, const char* name="scene.gltf");
    public:
        NScene( ygltf::glTF_t* gltf, ygltf::fl_gltf* fgltf );
    public:
        void dump(const char* msg="NScene::dump");
        void dump_scenes(const char* msg="NScene::dump_scenes");
        void dump_flat_nodes(const char* msg="NScene::dump_flat_nodes");
    private:
        void collect_mesh_totals(int scn_id=0);
        void collect_mesh_instances();
        void check_transforms(int scn_id=0);
    public:
        void dump_mesh_totals(const char* msg="NScene::dump_mesh_totals");
    public:
        unsigned getNumMeshes();
        unsigned getNumInstances(unsigned mesh_idx);
        int getInstanceNodeIndex( unsigned mesh_idx, unsigned instance_idx);
        const std::array<float, 16>& getTransform(unsigned node_idx );
    public:
        void dumpAll();
        void dumpAllInstances( unsigned mesh_idx);
        std::string descInstance( unsigned mesh_idx, unsigned instance_idx );
        std::string descNode( unsigned node_idx );
    private:
        ygltf::glTF_t* m_gltf ;  
        ygltf::fl_gltf* m_fgltf ;

        typedef std::map<int, std::vector<int>>  MMI ; 
        MMI m_mesh_instances ; 

        std::map<int, int> m_mesh_totals ; 

};

