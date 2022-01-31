#include "PLOG.hh"
#include "SPath.hh"
#include "sc4u.h"

#include "CSGFoundry.h"
#include "CSGQuery.h"
#include "CSGGrid.h"

#ifdef DEBUG_RECORD
#include "CSGRecord.h"
#endif


#include "OpticksCSG.h"
#include "csg_intersect_node.h"
#include "csg_intersect_tree.h"

CSGQuery::CSGQuery( const CSGFoundry* fd_ ) 
    :
    fd(fd_),
    prim0(fd->getPrim(0)),
    node0(fd->getNode(0)),
    plan0(fd->getPlan(0)),
    itra0(fd->getItra(0)),
    select_prim(nullptr),
    select_nodeOffset(0),
    select_numNode(0),
    select_root(nullptr)
{
   init(); 
}

void CSGQuery::init()
{
    selectPrim(0u,0u);  // select first prim of first solid 
    // TODO: use MOI to identify the prim to select : so this can work with full geometries
}

void CSGQuery::selectPrim(unsigned solidIdx, unsigned primIdxRel )
{
    const CSGPrim* pr = fd->getSolidPrim(solidIdx, primIdxRel); 
    assert( pr );  
    selectPrim(pr); 
}

void CSGQuery::selectPrim( const CSGPrim* pr )
{
    select_prim = pr ; 
    select_nodeOffset = pr->nodeOffset() ; 
    select_numNode = pr->numNode() ; 
    select_root = node0 + pr->nodeOffset() ; 

    LOG(info) 
         << " select_prim " << select_prim
         << " select_nodeOffset " << select_nodeOffset
         << " select_numNode " << select_numNode
         << " select_root " << select_root 
         ;   
}

unsigned CSGQuery::getSelectedTreeHeight() const 
{
    return TREE_HEIGHT(select_numNode) ; 
}


/**
CSGQuery::getSelectedNode
---------------------------

nodeIdxRel
   1-based tree index, root=1 

**/

const CSGNode* CSGQuery::getSelectedNode( int nodeIdxRel ) const 
{
    const CSGNode* nd = nodeIdxRel - 1 < select_numNode ? select_root + nodeIdxRel - 1 : nullptr  ;
    return nd ; 
}


float CSGQuery::operator()(const float3& position ) const
{
    return distance_tree( position, select_numNode, select_root, plan0, itra0 ) ;
}

bool CSGQuery::intersect( quad4& isect,  float t_min, const quad4& p ) const 
{
    float3 ray_origin    = make_float3(  p.q0.f.x, p.q0.f.y, p.q0.f.z );  
    float3 ray_direction = make_float3(  p.q1.f.x, p.q1.f.y, p.q1.f.z );  
    unsigned gsid = p.q3.u.w ; 
    return intersect( isect, t_min, ray_origin, ray_direction, gsid ); 
}



const float CSGQuery::SD_CUT = -1e-3 ; 

bool CSGQuery::IsSpurious( const quad4& isect ) // static
{
    float sd = isect.q1.f.w ;  
    bool spurious = sd < SD_CUT ; 
    return spurious ; 
}


/**
CSGQuery::intersect
--------------------

CPU exercise of CUDA intersect code

isect.q0.f.xyx
    surface normal at the intersect

isect.q0.f.w
    t, ray_direction multiple at the intersect  

isect.q1.f.xyz
    intersect position 

isect.q1.f.w 
    surface distance at intersect position.

    This is expected to be close to zero, 
    deviations from zero can be used to identify some 
    forms of spurious intersects.

    Note however that the most common cause of spurious intersects, 
    coincident faces, does not show up this way as the surface distance is 
    zero for both constituents across the common face.  

isect.q2.f.xyz
    ray_origin

isect.q3.f.xyz
    ray_direction

**/



bool CSGQuery::intersect( quad4& isect,  float t_min, const float3& ray_origin, const float3& ray_direction, unsigned gsid ) const 
{
    isect.zero();
    bool valid_intersect = intersect_tree(isect.q0.f, select_numNode, select_root, plan0, itra0, t_min, ray_origin, ray_direction );  

    if( valid_intersect )
    {   
        float t = isect.q0.f.w ; 
        float3 ipos = ray_origin + t*ray_direction ;   
        float sd = distance_tree( ipos, select_numNode, select_root, plan0, itra0 ) ;

        isect.q1.f.x = ipos.x ;
        isect.q1.f.y = ipos.y ;
        isect.q1.f.z = ipos.z ;
        isect.q1.f.w = sd ;  

        isect.q2.f.x = ray_origin.x ; 
        isect.q2.f.y = ray_origin.y ; 
        isect.q2.f.z = ray_origin.z ;
        isect.q2.f.w = t_min ; 

        isect.q3.f.x = ray_direction.x ; 
        isect.q3.f.y = ray_direction.y ; 
        isect.q3.f.z = ray_direction.z ;
        isect.q3.u.w = gsid ;  
    }   
    return valid_intersect ; 
}

std::string CSGQuery::Desc( const quad4& isect, const char* label  )  // static
{
    float sd = isect.q1.f.w ; 
    bool spurious = IsSpurious(isect); 
    std::stringstream ss ; 
    ss 
         << label 
         << std::endl 
         << std::setw(30) << " q0 norm t " 
         << "(" 
         << std::setw(10) << std::fixed << std::setprecision(4) << isect.q0.f.x 
         << std::setw(10) << std::fixed << std::setprecision(4) << isect.q0.f.y
         << std::setw(10) << std::fixed << std::setprecision(4) << isect.q0.f.z 
         << std::setw(10) << std::fixed << std::setprecision(4) << isect.q0.f.w 
         << ")" 
         << std::endl 
         << std::setw(30) << " q1 ipos sd " 
         << "(" 
         << std::setw(10) << std::fixed << std::setprecision(4) << isect.q1.f.x 
         << std::setw(10) << std::fixed << std::setprecision(4) << isect.q1.f.y
         << std::setw(10) << std::fixed << std::setprecision(4) << isect.q1.f.z 
         << std::setw(10) << std::fixed << std::setprecision(4) << sd
         << ")" 
         << ( spurious ? " SPURIOUS INTERSECT " : "-" ) 
         << " sd < SD_CUT : " 
         << std::setw(10) << std::fixed << std::setprecision(4) <<  SD_CUT 
         << std::endl 
         << std::setw(30) << " q2 ray_ori t_min " 
         << "(" 
         << std::setw(10) << std::fixed << std::setprecision(4) << isect.q2.f.x 
         << std::setw(10) << std::fixed << std::setprecision(4) << isect.q2.f.y
         << std::setw(10) << std::fixed << std::setprecision(4) << isect.q2.f.z 
         << std::setw(10) << std::fixed << std::setprecision(4) << isect.q2.f.w 
         << ")" 
         << std::endl 
         << std::setw(30) << " q3 ray_dir gsid " 
         << "(" 
         << std::setw(10) << std::fixed << std::setprecision(4) << isect.q3.f.x 
         << std::setw(10) << std::fixed << std::setprecision(4) << isect.q3.f.y
         << std::setw(10) << std::fixed << std::setprecision(4) << isect.q3.f.z 
         << std::setw(20) << C4U_desc(isect.q3.u.w) 
         << ")" 
         << std::endl 
         ;

    std::string s = ss.str() ; 
    return s ; 
}



/**
CSGQuery::intersect_again
--------------------------


**/
bool CSGQuery::intersect_again( quad4& isect, const quad4& prev_isect ) const 
{
    float t_min = prev_isect.q2.f.w ; 
    float3 ray_origin    = make_float3( prev_isect.q2.f.x, prev_isect.q2.f.y, prev_isect.q2.f.z ); 
    float3 ray_direction = make_float3( prev_isect.q3.f.x, prev_isect.q3.f.y, prev_isect.q3.f.z ); 
    unsigned gsid = prev_isect.q3.u.w ; 

#ifdef DEBUG_RECORD
    CSGRecord::SetEnabled(true); 
#endif
    bool valid_intersect = intersect( isect, t_min, ray_origin, ray_direction, gsid );  
#ifdef DEBUG_RECORD
    CSGRecord::SetEnabled(false); 
#endif

    LOG(info) 
        << std::endl 
        << Desc(prev_isect, "prev_isect" ) 
        << Desc(isect,      "isect" ) 
        ;
    return valid_intersect ; 
}


CSGGrid* CSGQuery::scanPrim(int resolution) const 
{
    const CSGPrim* pr = select_prim ;
    if( pr == nullptr )
    {
        LOG(fatal) << " no prim is selected " ;
        return nullptr ;
    }

    const float4 ce =  pr->ce() ;
    CSGGrid* grid = new CSGGrid( ce, resolution, resolution, resolution );
    grid->scan(*this) ;
    return grid ;
}

void CSGQuery::dumpPrim() const 
{
    if( select_numNode == 0 ) return ;

    LOG(info) 
          << " select_numNode " << select_numNode
          << " select_nodeOffset " << select_nodeOffset
          ; 

    for(int nodeIdx=select_nodeOffset ; nodeIdx < select_nodeOffset+select_numNode ; nodeIdx++)
    {
        const CSGNode* nd = node0 + nodeIdx ;
        LOG(info) << "    nodeIdx " << std::setw(5) << nodeIdx << " : " << nd->desc() ;
    }
}

void CSGQuery::dump(const char* msg) const
{
    LOG(info) << msg ; 
    dumpPrim(); 
}

