#include "PLOG.hh"
#include "SSys.hh"
#include "SPath.hh"
#include "sc4u.h"

#include "CSGFoundry.h"
#include "SName.h"
#include "CSGQuery.h"
#include "CSGGrid.h"

#ifdef DEBUG_RECORD
#include "CSGRecord.h"
#endif

#ifdef DEBUG_CYLINDER
#include "CSGDebug_Cylinder.hh"
#endif



#include "OpticksCSG.h"

#include "csg_intersect_leaf.h"
#include "csg_intersect_node.h"
#include "csg_intersect_tree.h"


const int CSGQuery::VERBOSE = SSys::getenvint("VERBOSE", 0); 


CSGQuery::CSGQuery( const CSGFoundry* fd_ ) 
    :
    fd(fd_),
    prim0(fd->getPrim(0)),
    node0(fd->getNode(0)),
    plan0(fd->getPlan(0)),
    itra0(fd->getItra(0)),
    select_prim(nullptr),
    select_nodeOffset(0),
    select_prim_numNode(0),
    select_root_node(nullptr),   // set by selectPrim
    select_root_typecode(CSG_ZERO),
    select_root_subNum(0),
    select_is_tree(true)
{
    init(); 
}

/**
CSGQuery::init
----------------

MOI meshName:meshOrdinal:instanceIndex 
   selects by meshname irrespective of which solid it appears in 

SOPR
   selects more formally by solidIdx:primIdxRel 

**/

void CSGQuery::init()
{
    // select first prim of first solid unless SOPR overrides that 
    int solidIdx = 0 ; 
    int primIdxRel = 0 ; 

    const char* sopr = SSys::getenvvar("SOPR", "0:0" ); 
    SName::ParseSOPR(solidIdx, primIdxRel, sopr );    

    LOG(info) << " sopr " << sopr << " solidIdx " << solidIdx << " primIdxRel " << primIdxRel ; 
    selectPrim(solidIdx, primIdxRel );  
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
    select_prim_ce = pr->ce(); 
    select_nodeOffset = pr->nodeOffset() ; 
    select_prim_numNode = pr->numNode() ; 
    select_root_node = node0 + pr->nodeOffset() ; 
    select_root_typecode = select_root_node->typecode(); 
    select_root_subNum = select_root_node->subNum(); 



    select_is_tree = CSG::IsTree((OpticksCSG_t)select_root_typecode) ; 

    if(VERBOSE > 0 ) LOG(info) 
         << " select_prim " << select_prim
         << " select_nodeOffset " << select_nodeOffset
         << " select_prim_numNode " << select_prim_numNode
         << " select_root_node " << select_root_node 
         << " select_root_typecode " << CSG::Name(select_root_typecode)
         << " select_root_subNum " << select_root_subNum
         << " getSelectedTreeHeight " << getSelectedTreeHeight()
         ;   


    if( select_root_subNum == 0 )
    {
        LOG(fatal) << "select_root_subNum ZERO " ; 
        //assert(0); 
    }
}

int CSGQuery::getSelectedType() const
{
    return select_root_typecode ; 
}
int CSGQuery::getSelectedTreeHeight() const 
{
    return select_is_tree ? TREE_HEIGHT(select_root_subNum) : -1 ;   
}

/**
CSGQuery::getSelectedNode
---------------------------

**/

const CSGNode* CSGQuery::getSelectedNode( int nodeIdx ) const 
{
    const CSGNode* nd = nodeIdx < select_prim_numNode ? select_root_node + nodeIdx : nullptr  ;
    return nd ; 
}


/**
CSGQuery::getSelectedTreeNode
------------------------------

nodeIdx
   0-based tree index, root=0 

**/

const CSGNode* CSGQuery::getSelectedTreeNode( int nodeIdx ) const 
{
    const CSGNode* nd = nodeIdx < select_root_subNum ? select_root_node + nodeIdx : nullptr  ;
    return nd ; 
}


float CSGQuery::distance(const float3& position ) const
{
    float sd = distance_prim( position, select_root_node, plan0, itra0 ) ; 
    return sd ;  
}

float CSGQuery::operator()(const float3& position ) const
{
    return distance(position); 
}

/**
CSGQuery::intersect
----------------------

This is invoked by CSGGeometry::saveCenterExtentGenstepIntersect using photons 
generated by SCenterExtentGenstep 

**/

bool CSGQuery::intersect( quad4& isect,  float t_min, const quad4& p ) const 
{
    float3 ray_origin    = make_float3(  p.q0.f.x, p.q0.f.y, p.q0.f.z );    // HMM: this could use sphoton approach avoiding temporaries
    float3 ray_direction = make_float3(  p.q1.f.x, p.q1.f.y, p.q1.f.z );  
    unsigned gsid = p.q3.u.w ; 
    return intersect( isect, t_min, ray_origin, ray_direction, gsid ); 
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


TODO: compare the isect with simtrace, it would be convenient to have same layout 

**/

bool CSGQuery::intersect( quad4& isect,  float t_min, const float3& ray_origin, const float3& ray_direction, unsigned gsid ) const 
{
    isect.zero();

    isect.q2.f.x = ray_origin.x ; 
    isect.q2.f.y = ray_origin.y ; 
    isect.q2.f.z = ray_origin.z ;
    isect.q2.f.w = t_min ;          

    isect.q3.f.x = ray_direction.x ; 
    isect.q3.f.y = ray_direction.y ; 
    isect.q3.f.z = ray_direction.z ;
    isect.q3.u.w = gsid ;  

#ifdef DEBUG
    std::cout << "CSGQuery::intersect  ray_origin " << ray_origin << " ray_direction " << ray_direction << std::endl ; 
#endif

    bool valid_intersect = intersect_prim( isect.q0.f, select_root_node, plan0, itra0, t_min, ray_origin, ray_direction ) ; 
    if( valid_intersect ) 
    {
        float t = isect.q0.f.w ; 
        float3 ipos = ray_origin + t*ray_direction ;   

        float sd = distance(ipos) ;

        isect.q1.f.x = ipos.x ;
        isect.q1.f.y = ipos.y ;
        isect.q1.f.z = ipos.z ;
        isect.q1.f.w = sd ;     // HMM: overwrite of tmin ?
    }
    return valid_intersect ; 
}






/**
CSGQuery::distance
-------------------

Used for debugging distance issues

**/

void CSGQuery::distance( quad4& isect,  const float3& ray_origin ) const 
{
    isect.zero();
    isect.q1.f.w = distance(ray_origin) ;  // HMM thats an overwrite 

    isect.q2.f.x = ray_origin.x ; 
    isect.q2.f.y = ray_origin.y ; 
    isect.q2.f.z = ray_origin.z ;
    isect.q2.f.w = 0.f ; 
}


const float CSGQuery::SD_CUT = -1e-3 ; 

bool CSGQuery::IsSpurious( const quad4& isect ) // static
{
    float sd = isect.q1.f.w ;  
    bool spurious = sd < SD_CUT ; 
    return spurious ; 
}


std::string CSGQuery::Label() // static
{
    std::stringstream ss ; 
    ss << "CSGQuery::Label " ; 

#ifdef DEBUG
    ss << " DEBUG" ; 
#else
    ss << " not-DEBUG" ; 
#endif

#ifdef DEBUG_RECORD
    ss << " DEBUG_RECORD" ; 
#else
    ss << " not-DEBUG_RECORD" ; 
#endif

#ifdef DEBUG_CYLINDER
    ss << " DEBUG_CYLINDER" ; 
#else
    ss << " not-DEBUG_CYLINDER" ; 
#endif



    std::string s = ss.str() ; 
    return s ; 
}


/**



**/


std::string CSGQuery::Desc( const quad4& isect, const char* label, bool* valid_intersect  )  // static
{
    float sd = isect.q1.f.w ; 
    bool spurious = IsSpurious(isect); 
    std::stringstream ss ; 
    ss 
         << std::setw(30) << label 
         << " "
         << ( valid_intersect ? ( *valid_intersect ? "HIT" : "MISS" )  : " " ) 
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
         //<< std::setw(10) << std::fixed << std::setprecision(4) << isect.q2.f.w        // problem as simtrace has boundary here
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
CSGQuery::simtrace
--------------------

**/

bool CSGQuery::simtrace( quad4& p ) const 
{
    float3* ray_origin = p.v2() ; 
    float3* ray_direction = p.v3() ; 
    float t_min = p.q1.f.w ;   

    // the 1st float4 argumnent gives surface normal at intersect and distance 
    bool valid_intersect = intersect_prim( p.q0.f, select_root_node, plan0, itra0, t_min, *ray_origin, *ray_direction ) ; 
    if( valid_intersect ) 
    {
        float t = p.q0.f.w ; 
        float3 ipos = (*ray_origin) + t*(*ray_direction) ;   
        p.q1.f.x = ipos.x ;
        p.q1.f.y = ipos.y ;
        p.q1.f.z = ipos.z ;
        //p.q1.f.w = distance(ipos) ;     // HMM: overwrite of tmin is problematic
    }
    return valid_intersect ; 
}


void CSGQuery::post(const char* outdir) 
{
#ifdef DEBUG_CYLINDER
    if(outdir)
    {
        const std::vector<CSGDebug_Cylinder>& record = CSGDebug_Cylinder::record ; 
        LOG(info) << " CSGDebug_Cylinder::record.size " << record.size() << " outdir " << outdir ; 
        CSGDebug_Cylinder::Save(outdir); 
    }
#endif
}



/**
CSGQuery::intersect_again
--------------------------


**/
bool CSGQuery::intersect_again( quad4& isect, const quad4& prev ) const 
{
    float3* ray_origin = prev.v2() ; 
    float3* ray_direction = prev.v3() ; 
    float t_min = prev.q1.f.w ;    // t_min was formerly  prev.q2.f.w 

    /*
    std::cout 
        << "CSGQuery::intersect_again" 
        << " ray_origin " << *ray_origin 
        << " ray_direction " << *ray_direction 
        << " t_min " << t_min  
        << std::endl
        ; 

    */

    //if(t_min != 0.f) LOG(error) << " t_min " << t_min ;  // not zero eg from propagate epsilon 0.05
    //assert( t_min == 0.f );  
    unsigned gsid = prev.q3.u.w ; 

#ifdef DEBUG_RECORD
    CSGRecord::SetEnabled(true); 
#endif

    bool valid_intersect = intersect( isect, t_min, *ray_origin, *ray_direction, gsid );  

#ifdef DEBUG_RECORD
    CSGRecord::SetEnabled(false); 
#endif

   /*
    LOG(info) 
        << std::endl 
        << Desc(prev,  "prev_isect" ) 
        << Desc(isect, "isect" ) 
        ;

    */ 

    return valid_intersect ; 
}


/**
CSGQuery::scanPrim
--------------------

Used by sdf_geochain.sh CSGGeometry::saveSignedDistanceField

**/

CSGGrid* CSGQuery::scanPrim(int resolution) const 
{
    const CSGPrim* pr = select_prim ;
    if( pr == nullptr )
    {
        LOG(fatal) << " no prim is selected " ;
        return nullptr ;
    }

    const float4 ce =  pr->ce() ;
    LOG(info) << " ce " << ce << " resolution " << resolution ;  

    CSGGrid* grid = new CSGGrid( ce, resolution, resolution, resolution );
    grid->scan(*this) ;
    return grid ;
}

void CSGQuery::dumpPrim(const char* msg) const 
{
    
    if( select_prim_numNode == 0 ) return ;

    LOG(info) 
          << msg 
          << " select_prim_numNode " << select_prim_numNode
          << " select_nodeOffset " << select_nodeOffset
          ; 

    for(int nodeIdx=select_nodeOffset ; nodeIdx < select_nodeOffset+select_prim_numNode ; nodeIdx++)
    {
        const CSGNode* nd = node0 + nodeIdx ;
        LOG(info) << nd->desc() ;
    }
}

void CSGQuery::dump(const char* msg) const
{
    LOG(info) << msg ; 
    dumpPrim(); 
}


