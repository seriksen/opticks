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


#include <iterator>
#include <algorithm>

#include "Opticks.hh"

#include "G4Hype.hh"
#include "G4Polycone.hh"
#include "G4Ellipsoid.hh"
#include "G4Torus.hh"
#include "G4Cons.hh"
#include "G4Trd.hh"
#include "G4Tubs.hh"
#include "G4Box.hh"
#include "G4Orb.hh"
#include "G4Sphere.hh"
#include "G4BooleanSolid.hh"
#include "G4IntersectionSolid.hh"
#include "G4SubtractionSolid.hh"
#include "G4UnionSolid.hh"
#include "G4SystemOfUnits.hh"

#include "X4Transform3D.hh"
#include "X4AffineTransform.hh"
#include "X4Solid.hh"
#include "X4.hh"

#include "SId.hh"
#include "BStr.hh"
#include "OpticksCSG.h"
#include "GLMFormat.hpp"
#include "NGLMExt.hpp"
#include "NGLM.hpp"
#include "NHyperboloid.hpp"
#include "NTorus.hpp"
#include "NCone.hpp"
#include "NConvexPolyhedron.hpp"
#include "NCylinder.hpp"
#include "NZSphere.hpp"
#include "NSphere.hpp"
#include "NBox.hpp"
#include "NDisc.hpp"
#include "NNode.hpp"
#include "NTreeBuilder.hpp"
#include "NTreeProcess.hpp"

#include "PLOG.hh"


unsigned X4Solid::fVerbosity = 0 ; 
void X4Solid::SetVerbosity(unsigned verbosity) // static
{
    fVerbosity = verbosity ; 
}


/**
X4Solid::Convert
-----------------

Canonicaly used from X4PhysicalVolume::convertSolid


**/

nnode* X4Solid::Convert(const G4VSolid* solid, Opticks* ok, const char* boundary)
{
    if(fVerbosity > 0) LOG(error) << " convert " << solid->GetName() ; 

    bool top = true ; 
    X4Solid xs(solid, ok, top);
    nnode* root = xs.root(); 

    root->update_gtransforms(); 

    if(boundary) root->boundary = boundary ; 

    return root ; 
}

/**
X4Solid::Balance
-----------------

Used from X4CSG::X4CSG during g4codegen

**/

nnode* X4Solid::Balance(nnode* raw, unsigned soIdx , unsigned lvIdx )
{
    nnode* root = NTreeProcess<nnode>::Process(raw, soIdx, lvIdx);  // balances deep trees, or if not deep retuns raw
    root->other = raw ; 
    root->boundary = raw->boundary ? strdup(raw->boundary) : NULL ; 
    // note that g4code is not passed, as its inconsistent with the balanced tree presumably 
    return root ; 
}


X4Solid::X4Solid(const G4VSolid* solid, Opticks* ok, bool top)
   :
   X4SolidBase(solid, ok, top),
   m_displaced(NULL)
{
   init(); 
}

X4Solid* X4Solid::getDisplaced() const
{
    return m_displaced ; 
}
bool X4Solid::hasDisplaced() const 
{
    return m_displaced != NULL ; 
}
void X4Solid::setDisplaced(X4Solid* displaced)
{
    m_displaced = displaced ; 
}


void X4Solid::init()
{
   if(fVerbosity > 0) LOG(error) << " instanciate " << desc()  ; 
   switch( m_entityType )
   {
    // generated by x4-case- Tue Jun 19 22:28:05 HKT 2018 
    case _G4DisplacedSolid    : convertDisplacedSolid()        ; break ; 
    case _G4UnionSolid        : convertUnionSolid()            ; break ; 
    case _G4IntersectionSolid : convertIntersectionSolid()     ; break ; 
    case _G4SubtractionSolid  : convertSubtractionSolid()      ; break ; 
    case _G4MultiUnion        : convertMultiUnion()            ; break ; 
    case _G4Box               : convertBox()                   ; break ; 
    case _G4Cons              : convertCons()                  ; break ; 
    case _G4EllipticalCone    : convertEllipticalCone()        ; break ; 
    case _G4Ellipsoid         : convertEllipsoid()             ; break ; 
    case _G4EllipticalTube    : convertEllipticalTube()        ; break ; 
    case _G4ExtrudedSolid     : convertExtrudedSolid()         ; break ; 
    case _G4Hype              : convertHype()                  ; break ; 
    case _G4Orb               : convertOrb()                   ; break ; 
    case _G4Para              : convertPara()                  ; break ; 
    case _G4Paraboloid        : convertParaboloid()            ; break ; 
    case _G4Polycone          : convertPolycone()              ; break ; 
    case _G4GenericPolycone   : convertGenericPolycone()       ; break ; 
    case _G4Polyhedra         : convertPolyhedra()             ; break ; 
    case _G4Sphere            : convertSphere()                ; break ; 
    case _G4TessellatedSolid  : convertTessellatedSolid()      ; break ; 
    case _G4Tet               : convertTet()                   ; break ; 
    case _G4Torus             : convertTorus()                 ; break ; 
    case _G4GenericTrap       : convertGenericTrap()           ; break ; 
    case _G4Trap              : convertTrap()                  ; break ; 
    case _G4Trd               : convertTrd()                   ; break ; 
    case _G4Tubs              : convertTubs()                  ; break ; 
    case _G4CutTubs           : convertCutTubs()               ; break ; 
    case _G4TwistedBox        : convertTwistedBox()            ; break ; 
    case _G4TwistedTrap       : convertTwistedTrap()           ; break ; 
    case _G4TwistedTrd        : convertTwistedTrd()            ; break ; 
    case _G4TwistedTubs       : convertTwistedTubs()           ; break ; 
   } 
}


G4ThreeVector X4Solid::GetAngles(const G4RotationMatrix& mtx)
{
   // from G4GDMLWriteDefine::GetAngles

   G4double x,y,z;
   G4RotationMatrix mat = mtx;
   mat.rectify();   // Rectify matrix from possible roundoff errors

   // Direction of rotation given by left-hand rule; clockwise rotation

   static const G4double kMatrixPrecision = 10E-10;
   const G4double cosb = std::sqrt(mtx.xx()*mtx.xx()+mtx.yx()*mtx.yx());

   if (cosb > kMatrixPrecision)
   {   
      x = std::atan2(mtx.zy(),mtx.zz());
      y = std::atan2(-mtx.zx(),cosb);
      z = std::atan2(mtx.yx(),mtx.xx());
   }   
   else
   {   
      x = std::atan2(-mtx.yz(),mtx.yy());
      y = std::atan2(-mtx.zx(),cosb);
      z = 0.0;
   }   

   return G4ThreeVector(x,y,z);
}

void X4Solid::booleanDisplacement( G4VSolid** pp, G4ThreeVector& pos, G4ThreeVector& rot )
{
    // cf /usr/local/opticks/externals/g4/geant4_10_02_p01/source/persistency/gdml/src/G4GDMLWriteSolids.cc
    int displaced = 0 ; 
    while (true)
    {  
      assert( displaced <= 8 );
      if (G4DisplacedSolid* disp = dynamic_cast<G4DisplacedSolid*>(*pp))
      {  
         pos += disp->GetObjectTranslation();
         rot += GetAngles(disp->GetObjectRotation());
         *pp = disp->GetConstituentMovedSolid();
         displaced++;
         continue;
      }
      break;
   }

   LOG(info) << " booleanDisplacement "
             << " pos " << pos 
             << " rot " << rot 
             ;
}

void X4Solid::convertUnionSolid()
{
    convertBooleanSolid() ;
}
void X4Solid::convertIntersectionSolid()
{
    convertBooleanSolid() ;
}
void X4Solid::convertSubtractionSolid()
{
    convertBooleanSolid() ;
}

void X4Solid::convertDisplacedSolid()
{
    const G4DisplacedSolid* const disp = static_cast<const G4DisplacedSolid*>(m_solid);
    G4VSolid* moved = disp->GetConstituentMovedSolid() ;
    assert( dynamic_cast<G4DisplacedSolid*>(moved) == NULL ); // only a single displacement is handled

    bool top = false ;  // never top of tree : expect to always be a boolean RHS
    X4Solid* xmoved = new X4Solid(moved, m_ok, top);
    setDisplaced(xmoved); 

    nnode* a = xmoved->root();

    glm::mat4 xf_disp = X4Transform3D::GetDisplacementTransform(disp);  
    a->transform = new nmat4triple(xf_disp); 

    // update_gtransforms() is invoked on the full solid, not here from just one node

    setRoot(a); 
}

void X4Solid::convertBooleanSolid()
{  
    const G4BooleanSolid* const solid = static_cast<const G4BooleanSolid*>(m_solid);
    assert(solid); 

    OpticksCSG_t _operator = CSG_ZERO ; 
    if      (dynamic_cast<const G4IntersectionSolid*>(solid)) _operator = CSG_INTERSECTION ;
    else if (dynamic_cast<const G4SubtractionSolid*>(solid))  _operator = CSG_DIFFERENCE ;
    else if (dynamic_cast<const G4UnionSolid*>(solid))        _operator = CSG_UNION ;
    assert( _operator != CSG_ZERO ) ;

    G4VSolid* left  = const_cast<G4VSolid*>(solid->GetConstituentSolid(0));
    G4VSolid* right = const_cast<G4VSolid*>(solid->GetConstituentSolid(1));

    bool is_left_displaced = dynamic_cast<G4DisplacedSolid*>(left) != NULL ;
    bool is_right_displaced = dynamic_cast<G4DisplacedSolid*>(right) != NULL ;

    assert( !is_left_displaced && "not expecting left displacement " ); 

    X4Solid* xleft = new X4Solid(left, m_ok, false); 
    X4Solid* xright = new X4Solid(right, m_ok, false); 

    nnode* a = xleft->root(); 
    nnode* b = xright->root(); 
    nnode* n = nnode::make_operator( _operator, a, b ); 

    setRoot(n); 


    /*
     Hmm will just detecting transforms on b catch em all ?
    
     Need to prepend the transform setup g4code to the node that 
     uses it and assign an transform identifiers for rotation and translation and
     refer to them in the below param list. 

     Hmm the AffineTransform ctor with all the numbers is private, so might as well use boolean 
     ctor with rotation and translation args ?

     X4RotationMatrix makes the protected ctor accessible but then are 
     not generating pure G4 code ? 
    */

    std::vector<std::string> param ;
    std::vector<std::string> keys ;
    param.push_back( xleft->getIdentifier() ); 
    keys.push_back( "left_id" );  

    if(is_right_displaced)
    {
        X4Solid* xright_displaced = xright->getDisplaced() ; 

        //assert(b->gtransform) ; 
        const G4DisplacedSolid* const disp = static_cast<const G4DisplacedSolid*>(right);
        assert( disp ); 
        X4AffineTransform xdirect(disp->GetDirectTransform()); 

        bool identityRotation = xdirect.isIdentityRotation() ; 

        const char* rot_id = identityRotation ? "NULL" : OTHER_ID->get(false) ;
        const char* tla_id = OTHER_ID->get(false) ;

        // TODO: suppress identity 
        std::string rot = identityRotation ? "" : xdirect.getRotationCode(rot_id);
        std::string tla = xdirect.getTranslationCode(tla_id);

        addG4Code(rot.c_str()) ;  
        addG4Code(tla.c_str()) ;  

        param.push_back( xright_displaced->getIdentifier() ); 
        param.push_back( rot_id ) ; 
        param.push_back( tla_id ) ; 
        keys.push_back( "right_id" );  
        keys.push_back( "rotation_id" );  
        keys.push_back( "translation_id" );  
    } 
    else
    {
        param.push_back( xright->getIdentifier() ); 
        keys.push_back( "right_id" );  
    }


    const std::vector<std::string> cparam(param); 
    const std::vector<std::string> ckeys(keys); 
    setG4Param(cparam, ckeys); 
}



const bool X4Solid::convertSphere_duplicate_py_segment_omission = true ; 

nnode* X4Solid::convertSphere_(bool only_inner)
{
    const G4Sphere* const solid = static_cast<const G4Sphere*>(m_solid);

    float rmin = solid->GetInnerRadius()/mm ; 
    float rmax = solid->GetOuterRadius()/mm ; 

    bool has_inner = !only_inner && rmin > 0.f ; 
    nnode* inner = has_inner ? convertSphere_(true) : NULL ;  
    float radius = only_inner ? rmin : rmax ;   

    LOG(verbose) 
              << " radius : " << radius 
              << " only_inner : " << only_inner
              << " has_inner : " << has_inner 
              ;

    float startThetaAngle = solid->GetStartThetaAngle()/degree ; 
    float deltaThetaAngle = solid->GetDeltaThetaAngle()/degree ; 

    // z to the right, theta   0 -> z=r, theta 180 -> z=-r
    float rTheta = startThetaAngle ;
    float lTheta = startThetaAngle + deltaThetaAngle ;
    assert( rTheta >= 0.f && rTheta <= 180.f) ; 
    assert( lTheta >= 0.f && lTheta <= 180.f) ; 

    bool zslice = startThetaAngle > 0.f || deltaThetaAngle < 180.f ; 

    LOG(verbose) 
              << " rTheta : " << rTheta
              << " lTheta : " << lTheta
              << " zslice : " << zslice
              ;

    float x = 0.f ; 
    float y = 0.f ; 
    float z = 0.f ; 

    nnode* cn = NULL ; 
    if(zslice)
    {
        double zmin = radius*std::cos(lTheta*CLHEP::pi/180.) ;
        double zmax = radius*std::cos(rTheta*CLHEP::pi/180.) ;
        assert( zmax > zmin ) ; 
        cn = make_zsphere( x, y, z, radius, zmin, zmax ) ;
        cn->label = BStr::concat(m_name, "_nzsphere", NULL) ; 
    }
    else
    {
        cn = make_sphere( x, y, z, radius );
        cn->label = BStr::concat(m_name, "_nsphere", NULL ) ; 
    }
    
    nnode* ret = has_inner ? nnode::make_operator(CSG_DIFFERENCE, cn, inner) : cn ; 
    if(has_inner) ret->label = BStr::concat(m_name, "_ndifference", NULL ) ; 
  

    float startPhi = solid->GetStartPhiAngle()/degree ; 
    float deltaPhi = solid->GetDeltaPhiAngle()/degree ; 
    bool has_deltaPhi = deltaPhi < 360.f ; 
    
    //assert( startPhi >= 0.f && !has_deltaPhi ); 
    // CAUTION : deltaphi_slab_segment_enabled is switched off in the python

    // phi segment radius segR rotates around origin in x-y plane,
    //  z limited +- segZ/2 


    bool omit = convertSphere_duplicate_py_segment_omission ; 
    
    if(has_deltaPhi && omit)
    {
        LOG(error) << " convertSphere_duplicate_py_segment_omission " << omit 
                   << " has_deltaPhi " << has_deltaPhi
                   << " startPhi " << startPhi 
                   << " deltaPhi " << deltaPhi 
                   << " name " << m_name 
                   ;
    }

    bool deltaPhi_segment_enabled = !omit ; 

    float segZ = radius*1.01 ; 
    float segR = radius*1.5 ;   

    nnode* result =  has_deltaPhi && deltaPhi_segment_enabled 
                  ?
                     intersectWithPhiSegment(ret, startPhi, deltaPhi, segZ, segR ) 
                  :
                     ret 
                  ;

    return result ; 
}



/**
X4Solid::convertSphere
========================

Following ../analytic/gdml.py results in different nnode primitive subclasses 
or small nnode CSG trees depending on parameter values.
    
::
    
    nsphere 
    nzsphere 
       zsliced
    ndifference  
       handling rmin > 0 
    nintersection  
       when applying a phi segment

**/


void X4Solid::convertSphere()
{  
    const G4Sphere* const solid = static_cast<const G4Sphere*>(m_solid);
    assert(solid); 
    //LOG(info) << "\n" << *solid ; 

    bool only_inner = false ; 

    nnode* n = convertSphere_(only_inner); 
    setRoot(n); 


    std::vector<double> param = {
                                 solid->GetInnerRadius() ,
                                 solid->GetOuterRadius() ,
                                 solid->GetStartPhiAngle() ,
                                 solid->GetDeltaPhiAngle() ,
                                 solid->GetStartThetaAngle() ,
                                 solid->GetDeltaThetaAngle()
                               } ;

    std::vector<std::string> keys = {
                                      "innerRadius",
                                      "outerRadius",
                                      "startPhiAngle",
                                      "deltaPhiAngle",
                                      "startThetaAngle",
                                      "deltaThetaAngle"
                                  } ;  

    setG4Param(param,keys);
    setG4Args(param, keys);
}


void X4Solid::convertOrb()
{  
    const G4Orb* const solid = static_cast<const G4Orb*>(m_solid);
    assert(solid); 
    //LOG(info) << "\n" << *solid ; 

    float radius = solid->GetRadius()/mm ; 

    float x = 0.f ; 
    float y = 0.f ; 
    float z = 0.f ; 

    nnode* n =  make_sphere( x, y, z, radius );
    n->label = BStr::concat(m_name, "_sphere", NULL ) ; 

    setRoot(n); 

    std::vector<double> param  = { solid->GetRadius() } ;
    std::vector<std::string> keys  = { "radius" } ;

    setG4Param(param, keys); 
    setG4Args(param, keys);
}


void X4Solid::convertBox()
{  
    // cf ../analytic/gdml.py:Box

    const G4Box* const solid = static_cast<const G4Box*>(m_solid);
    assert(solid); 

    //LOG(info) << "\n" << *solid ; 


    // match G4GDMLWriteSolids::BoxWrite
    float hx = solid->GetXHalfLength()/mm ; 
    float hy = solid->GetYHalfLength()/mm ; 
    float hz = solid->GetZHalfLength()/mm ; 

    float x = 2.0*hx ; 
    float y = 2.0*hy ; 
    float z = 2.0*hz ; 

    nnode* n =  make_box3( x, y, z);
    n->label = BStr::concat(m_name, "_box3", NULL ) ; 
    setRoot(n); 

    std::vector<double> param = { hx, hy, hz } ;
    std::vector<std::string> keys  = { "xHalfLength", "yHalfLength", "zHalfLength" } ;


    setG4Param(param, keys);
    setG4Args(param, keys);
}


nnode* X4Solid::convertTubs_cylinder()
{  
    const G4Tubs* const solid = static_cast<const G4Tubs*>(m_solid);
    assert(solid); 

    float rmin = solid->GetInnerRadius()/mm ; 
    float rmax = solid->GetOuterRadius()/mm ; 
    float hz = solid->GetZHalfLength()/mm ;  

    bool has_inner = rmin > 0.f ; 
   
    nnode* inner = NULL ; 
    if(has_inner)
    {
        // Expand inner-z by 1%, note that this does not change geometry : 
        // as are expanding the inner tube in z which are about to subtract away.
        // This is a simple way of avoiding CSG coincident constituent surface glitches.

        float nudge_inner = 0.01f ; 
        float dz = hz*nudge_inner ;  
        inner = make_cylinder(rmin, -(hz+dz), (hz+dz) ); 
        inner->label = BStr::concat( m_name, "_inner", NULL ); 
    }

    nnode* outer = make_cylinder(rmax, -hz, hz);
    outer->label = BStr::concat( m_name, "_outer", NULL ); 

    nnode* tube = has_inner ? nnode::make_operator(CSG_DIFFERENCE, outer, inner) : outer ; 
    tube->label = BStr::concat( m_name, "_difference", NULL );

    return tube ; 
}

const float X4Solid::hz_disc_cylinder_cut = 1.f ; // 1mm 

nnode* X4Solid::convertTubs_disc()
{  
    const G4Tubs* const solid = static_cast<const G4Tubs*>(m_solid);
    assert(solid); 
 
    float rmin = solid->GetInnerRadius()/mm ; 
    float rmax = solid->GetOuterRadius()/mm ; 
    float hz = solid->GetZHalfLength()/mm ;  

    assert( hz < hz_disc_cylinder_cut && "this is only applicable to very thin-in-z cylinders that can be regarded as discs" ) ; 

    nnode* tube = make_disc(rmin,rmax, -hz, hz);
    tube->label = BStr::concat( m_name, "_disc", NULL ); 

    return tube ; 
}

void X4Solid::convertTubs()
{  
    // cf ../analytic/gdml.py:Tube

    const G4Tubs* const solid = static_cast<const G4Tubs*>(m_solid);
    assert(solid); 
    //LOG(info) << "\n" << *solid ; 

    float hz = solid->GetZHalfLength()/mm ;  
    float z = hz*2.0 ;   // <-- this full-length z is what GDML stores

    float startPhi = solid->GetStartPhiAngle()/degree ; 
    float deltaPhi = solid->GetDeltaPhiAngle()/degree ; 
    float rmax = solid->GetOuterRadius()/mm ; 

    bool pick_disc = hz < hz_disc_cylinder_cut ; 

    nnode* tube = pick_disc ? convertTubs_disc() : convertTubs_cylinder() ; 

    bool deltaPhi_segment_enabled = true ; 
    bool has_deltaPhi = deltaPhi < 360.f ; 

    float segZ = z*1.01 ; 
    float segR = rmax*1.5 ;   

    // TODO: calculate what the segment prism segR size  should be rather 
    //       than this adhoc choice.
    //       As are intersecting it doesnt matter if the segR is too big, 
    //       but being too small could result in partial segmenting of the base shape
    //
    //       Is 50% bigger than rmax always a safe choice ?

    nnode* result =  has_deltaPhi && deltaPhi_segment_enabled 
                  ?
                     intersectWithPhiSegment(tube, startPhi, deltaPhi, segZ, segR ) 
                  :
                     tube 
                  ;

    setRoot(result); 



    std::vector<double> param = { 
                                  solid->GetInnerRadius(), 
                                  solid->GetOuterRadius(), 
                                  solid->GetZHalfLength(), 
                                  solid->GetStartPhiAngle(),
                                  solid->GetDeltaPhiAngle() 
                               } ;

    std::vector<std::string> keys = { 
                                  "innerRadius", 
                                  "outerRadius", 
                                  "zHalfLength", 
                                  "startPhiAngle",
                                  "deltaPhiAngle" 
                               } ;

    setG4Param(param, keys);
    setG4Args(param, keys);
}


/**
X4Solid::intersectWithPhiSegment
---------------------------------

* suffers degeneracy collapse from segment to a plane when deltaPhi = 180 

**/

nnode* X4Solid::intersectWithPhiSegment(nnode* whole, float startPhi, float deltaPhi, float segZ, float segR )  
{
    bool has_deltaphi = deltaPhi < 360.f ; 
    assert( has_deltaphi ) ; 

    nnode* segment = NULL ; 
  
    if( startPhi == 0.f && deltaPhi == 180.f )
    {
        LOG(error) << " special cased startPhi == 0.f && deltaPhi == 180.f " ; 

        // pure guesswork starting point : need some visualization guidance 

        nbbox bb = whole->bbox();
        float sx = bb.max.x - bb.min.x ; 
        float sy = bb.max.y - bb.min.y ; 
        float sz = bb.max.z - bb.min.z ; 

        segment = make_box3(sx,sy,sz);  
        //segment.transform = nmat4triple::make_transform(
        segment->label = BStr::concat(m_name, "_segment_box", NULL); 
    }
    else
    {
        float phi0 = startPhi ; 
        float phi1 = startPhi + deltaPhi ; 
        segment = nconvexpolyhedron::make_segment(phi0, phi1, segZ, segR );  
        segment->label = BStr::concat(m_name, "_segment_wedge", NULL); 
    }

    nnode* result = nnode::make_operator(CSG_INTERSECTION, whole, segment); 
    result->label = BStr::concat(m_name, "_intersection", NULL); 

    return result ;   
}

void X4Solid::convertTrd()
{  
/**
Following 

* G4GDMLWriteSolids::TrdWrite
* ../analytic/gdml.py 
* ../analytic/prism.py 

**/
    const G4Trd* const solid = static_cast<const G4Trd*>(m_solid);
    assert(solid); 

    float x1 = 2.0*solid->GetXHalfLength1()/mm ;
    float x2 = 2.0*solid->GetXHalfLength2()/mm ; 
    float y1 = 2.0*solid->GetYHalfLength1()/mm ; 
    float y2 = 2.0*solid->GetYHalfLength2()/mm ; 
    float z = 2.0*solid->GetZHalfLength()/mm ; 

    nnode* trd = nconvexpolyhedron::make_trapezoid(z, x1, y1, x2, y2 ); 
    trd->label = BStr::concat(m_name, "_solid", NULL ); 

    setRoot(trd); 

    std::vector<double> param = { solid->GetXHalfLength1() , 
                                 solid->GetXHalfLength2() , 
                                 solid->GetYHalfLength1() ,
                                 solid->GetYHalfLength2() ,
                                 solid->GetZHalfLength()  } ;  

    std::vector<std::string> keys = {
                                    "xHalfLength1",  
                                    "xHalfLength2",  
                                    "yHalfLength1",  
                                    "yHalfLength2",  
                                    "zHalfLength"
                                   } ;   

    setG4Param(param, keys);
    setG4Args(param, keys);
}


nnode* X4Solid::convertCons_(bool only_inner)
{
    const G4Cons* const cone = static_cast<const G4Cons*>(m_solid);
    assert(cone); 

    // G4GDMLWriteSolids::ConeWrite

    float rmax1    = cone->GetOuterRadiusMinusZ()/mm ;
    float rmax2    = cone->GetOuterRadiusPlusZ()/mm  ;

    float rmin1    = cone->GetInnerRadiusMinusZ()/mm ;
    float rmin2    = cone->GetInnerRadiusPlusZ()/mm  ;

    float z        = 2.0*cone->GetZHalfLength()/mm   ;
    float startPhi = cone->GetStartPhiAngle()/degree ;
    float deltaPhi = cone->GetDeltaPhiAngle()/degree ;

    bool has_inner = !only_inner && (rmin1 > 0.f || rmin2 > 0.f) ; 
    nnode* inner = has_inner ? convertCons_(true) : NULL ; 

    float r1 = only_inner ? rmin1 : rmax1 ; 
    float r2 = only_inner ? rmin2 : rmax2 ; 
    float z1 = -z/2.0 ; 
    float z2 = z/2.0 ; 

    nnode* cn = make_cone(r1,z1,r2,z2) ;
    cn->label = BStr::concat(m_name, "_cn", NULL ) ; 

    nnode* ret = has_inner ? nnode::make_operator(CSG_DIFFERENCE, cn, inner) : cn ; 
    if(has_inner) ret->label = BStr::concat(m_name, "_ndifference", NULL ) ; 


    bool deltaPhi_segment_enabled = true ; 
    bool has_deltaPhi = deltaPhi < 360.f ; 

    float segZ = z*1.01 ; 
    float segR = std::max(rmax1, rmax2)*1.5 ;   

    // TODO: calculate what the segment prism segR size  should be rather 
    //       than this adhoc choice.
    //       As are intersecting it doesnt matter if the segR is too big, 
    //       but being too small could result in partial segmenting of the base shape
    //
    //       Is 50% bigger than rmax always a safe choice ?

    nnode* result =  has_deltaPhi && deltaPhi_segment_enabled 
                  ?
                     intersectWithPhiSegment(ret, startPhi, deltaPhi, segZ, segR ) 
                  :
                     ret 
                  ;

    return result ; 
}

void X4Solid::convertCons()
{  
    const G4Cons* const solid = static_cast<const G4Cons*>(m_solid);
    assert(solid); 
    //LOG(info) << "\n" << *solid; 

    bool only_inner = false ; 
    nnode* n = convertCons_(only_inner); 
    setRoot(n); 

    std::vector<double> param = {  
                                  solid->GetInnerRadiusMinusZ() ,  
                                  solid->GetOuterRadiusMinusZ() ,  
                                  solid->GetInnerRadiusPlusZ()  ,  
                                  solid->GetOuterRadiusPlusZ()  ,
                                  solid->GetZHalfLength() , 
                                  solid->GetStartPhiAngle() , 
                                  solid->GetDeltaPhiAngle() 
                               } ;

    std::vector<std::string> keys = {
                                  "innerRadiusMinusZ" ,  
                                  "outerRadiusMinusZ" ,  
                                  "innerRadiusPlusZ"  ,  
                                  "outerRadiusPlusZ"  ,
                                  "zHalfLength" , 
                                  "startPhiAngle" , 
                                  "deltaPhiAngle"
                                   } ; 

    setG4Param(param, keys );
    setG4Args(param, keys);
}

void X4Solid::convertTorus()
{  
    const G4Torus* const solid = static_cast<const G4Torus*>(m_solid);
    assert(solid); 

    // G4GDMLWriteSolids::TorusWrite

    float rmin = solid->GetRmin()/mm ; 
    float rmax = solid->GetRmax()/mm ;
    float rtor = solid->GetRtor()/mm ;
    float startPhi = solid->GetSPhi()/degree ; 
    float deltaPhi = solid->GetDPhi()/degree ; 

    if( startPhi < 0 ) 
    {
        LOG(fatal) 
           <<  " changing torus -ve startPhi (degrees) to zero " << startPhi 
           ;
         startPhi = 0.f ; 
    } 

    if( deltaPhi != 360.f ) 
    {
        LOG(fatal) 
           <<  " changing torus deltaPhi (degrees) to 360 " << deltaPhi 
           ;
         deltaPhi = 360.f ; 
    } 


    assert( rmin == 0.f ); // torus with rmin not yet handled 
    assert( startPhi == 0.f && deltaPhi == 360.f ); 

    float r = rmax ; 
    float R = rtor ; 
    assert( R > r ); 

    nnode* n = make_torus(R, r) ;
    n->label = BStr::concat( m_name , "_torus" , NULL ); 
    setRoot(n); 

    std::vector<double> param = {  
                                  solid->GetRmin() ,  
                                  solid->GetRmax() ,  
                                  solid->GetRtor()  ,  
                                  solid->GetSPhi()  ,
                                  solid->GetDPhi()  
                               } ;

    std::vector<std::string> keys = {
                                  "rmin" ,  
                                  "rmax" ,  
                                  "rtor"  ,  
                                  "sPhi"  ,
                                  "dPhi"  
                                 } ; 

    setG4Param(param, keys);
    setG4Args(param, keys);
}

void X4Solid::convertEllipsoid()
{  
    const G4Ellipsoid* const solid = static_cast<const G4Ellipsoid*>(m_solid);
    assert(solid); 

    // G4GDMLWriteSolids::EllipsoidWrite

    float ax = solid->GetSemiAxisMax(0)/mm ; 
    float by = solid->GetSemiAxisMax(1)/mm ; 
    float cz = solid->GetSemiAxisMax(2)/mm ; 

    glm::vec3 scale( ax/cz, by/cz, 1.f) ;   
    // unity scaling in z, so z-coords are unaffected  
 
    float zcut1 = solid->GetZBottomCut()/mm ; 
    float zcut2 = solid->GetZTopCut()/mm ;

    bool dbg_with_hemi_ellipsoid_bug = m_ok->hasOpt("dbg_with_hemi_ellipsoid_bug") ; 

    float z1 ; 
    float z2 ; 
 
    if(dbg_with_hemi_ellipsoid_bug)
    {
        LOG(fatal) << " reproducing old bug : --dbg_with_hemi_ellipsoid_bug  " ; 
        z1 = zcut1 != 0.f && zcut1 > -cz ? zcut1 : -cz ; 
        z2 = zcut2 != 0.f && zcut2 <  cz ? zcut2 :  cz ; 
        //       HUH ^^^^^^^^^^^^^ WHATS SPECIAL ABOUT ZERO
        //       see notes/issues/review-analytic-geometry.rst    
    }
    else
    { 
        z1 = zcut1 > -cz ? zcut1 : -cz ; 
        z2 = zcut2 <  cz ? zcut2 :  cz ; 
    }

    assert( z2 > z1 ) ;  

    bool zslice = z1 > -cz || z2 < cz ;  

    LOG(debug) 
         << " zcut1 " << zcut1 
         << " zcut2 " << zcut2
         << " z1 " << z1
         << " z2 " << z2
         << " cz " << cz
         << " zslice " << zslice
         ;


    nnode* cn = zslice ? 
                          (nnode*)make_zsphere( 0.f, 0.f, 0.f, cz, z1, z2 ) 
                       :
                          (nnode*)make_sphere( 0.f, 0.f, 0.f, cz )
                       ;

    cn->label = BStr::concat(m_name, "_ellipsoid", NULL) ; 
    cn->transform = nmat4triple::make_scale( scale );
    
    LOG(debug) 
         << std::endl  
         << gpresent("tr.t", cn->transform->t ) 
         << std::endl  
         << gpresent("tr.v", cn->transform->v )  
         << std::endl  
         << gpresent("tr.q", cn->transform->q ) 
         << std::endl  
         ;

    setRoot(cn); 

    std::vector<double> param = {  
                                  solid->GetSemiAxisMax(0) ,  
                                  solid->GetSemiAxisMax(1) ,  
                                  solid->GetSemiAxisMax(2) ,  
                                  solid->GetZBottomCut()  ,
                                  solid->GetZTopCut()  
                               } ;

    std::vector<std::string> keys = {
                                  "semiAxisMaxX" ,  
                                  "semiAxisMaxY" ,  
                                  "semiAxisMaxZ" ,  
                                  "zBottomCut"  ,
                                  "zTopCut"
                                  } ;  

    setG4Param(param, keys);
    setG4Args(param, keys);
}


void X4Solid::convertPolyconePrimitives( const std::vector<zplane>& zp,  std::vector<nnode*>& prims )
{
    for( unsigned i=1 ; i < zp.size() ; i++ )
    {
        const zplane& zp1 = zp[i-1] ; 
        const zplane& zp2 = zp[i] ; 
        double r1 = zp1.rmax ; 
        double r2 = zp2.rmax ; 
        double z1 = zp1.z ; 
        double z2 = zp2.z ; 

        if( z1 == z2 )
        {
            //LOG(warning) << " skipping z2 == z1 zp " ; 
            continue ; 
        }
        
        bool z_ascending = z2 > z1 ; 
        if(!z_ascending) LOG(fatal) << " !z_ascending " 
                                    << " z1 " << z1  
                                    << " z2 " << z2
                                    ;  
        assert(z_ascending); 

        nnode* n = NULL ; 
        if( r2 == r1 )
        { 
            n = make_cylinder(r2, z1, z2);
            n->label = BStr::concat( m_name, i-1, "_zp_cylinder" ); 
        }
        else
        {
            n = make_cone(r1,z1,r2,z2) ;
            n->label = BStr::concat<unsigned>(m_name, i-1 , "_zp_cone" ) ; 
        }
        prims.push_back(n); 
    }   // over pairs of planes
}



const bool X4Solid::convertPolycone_duplicate_py_inner_omission = true ; 

void X4Solid::convertPolycone()
{  
    // G4GDMLWriteSolids::PolyconeWrite
    // G4GDMLWriteSolids::ZplaneWrite
    // ../analytic/gdml.py 

    //LOG(error) << "START" ; 

    const G4Polycone* const solid = static_cast<const G4Polycone*>(m_solid);
    assert(solid); 
    const G4PolyconeHistorical* ph = solid->GetOriginalParameters() ;

    float startphi = ph->Start_angle/degree ;  
    float deltaphi = ph->Opening_angle/degree ;
    assert( startphi == 0.f && deltaphi == 360.f ); 

    unsigned nz = ph->Num_z_planes ; 

    //LOG(error) << " nz " << nz ; 

    std::vector<zplane> zp(nz) ; 
    std::set<double> Rmin ; 

    for (int i=0; i < int(nz) ; i++) 
    {
        zp[i] = { ph->Rmin[i], ph->Rmax[i], ph->Z_values[i] } ;  
        Rmin.insert( ph->Rmin[i] );

/*
        LOG(error) << " zp[i] (rmin,rmax,z)" << i  
                   << " (" 
                   << zp[i].rmin 
                   << " , "
                   << zp[i].rmax
                   << " , "
                   << zp[i].z
                   << ")"
                   ;
*/

    }

    if( zp.size() == 2 && zp[0].z > zp[1].z )  // Aug 2018 FIX: was [0] [0] 
    {
        LOG(debug) << "Polycone swap misordered pair of zplanes for " << m_name ; 
        std::reverse( std::begin(zp), std::end(zp) ) ; 
    }

    std::vector<nnode*> prims ; 
    convertPolyconePrimitives( zp, prims ); 

    //LOG(info) << "pre-UnionTree" ; 
    nnode* cn = NTreeBuilder<nnode>::UnionTree(prims) ;
    //LOG(info) << "post-UnionTree" ; 


    bool multi_Rmin = Rmin.size() > 1 ; 
    if( multi_Rmin ) 
    {
        LOG(fatal) << " multiple Rmin is unhandled " << m_name ;  
    }
    assert( !multi_Rmin ) ; 

    double rmin = *Rmin.begin() ; 
    bool has_inner = rmin > 0. ; 

    nnode* inner = NULL ; 
    if(has_inner)
    {
        double zmin = zp[0].z ; 
        double zmax = zp[nz-1].z ; 
        inner = make_cylinder(rmin, zmin, zmax);
        inner->label = BStr::concat( m_name, "_inner_cylinder", NULL  ); 
    }


    bool omit = convertPolycone_duplicate_py_inner_omission ; 

    if( has_inner && omit )
    { 
        LOG(error) << " convertPolycone_duplicate_py_inner_omission " << omit ;
        inner = NULL ;  
    }

    nnode* result = inner ? nnode::make_operator(CSG_DIFFERENCE, cn, inner )  : cn ; 
    setRoot(result); 

    convertPolycone_g4code();
    //LOG(error) << "DONE" ; 
}


void X4Solid::convertPolycone_g4code()
{
    const G4Polycone* const so = static_cast<const G4Polycone*>(m_solid);
    assert(so); 
    const G4PolyconeHistorical* ph = so->GetOriginalParameters() ;

    std::vector<std::string> param ;

    double startPhi = so->GetStartPhi() ; 
    double endPhi = so->GetEndPhi() ; 

    double phiStart = startPhi ;   
    double phiTotal = endPhi - startPhi ; 
    unsigned numZPlanes = ph->Num_z_planes ; 

    param.push_back( X4::Value(phiStart) ); 
    param.push_back( phiTotal < CLHEP::twopi ? X4::Value(phiTotal) : "CLHEP::twopi" ); 
    param.push_back( X4::Value(numZPlanes) ); 

    const char* zPlane_id = OTHER_ID->get(false) ; 
    const char* rInner_id = OTHER_ID->get(false) ; 
    const char* rOuter_id = OTHER_ID->get(false) ; 

    std::string zPlane = X4::Array(ph->Z_values, numZPlanes , zPlane_id ) ;
    std::string rInner = X4::Array(ph->Rmin    , numZPlanes , rInner_id ) ;
    std::string rOuter = X4::Array(ph->Rmax    , numZPlanes , rOuter_id ) ;

    addG4Code( zPlane.c_str() ); 
    addG4Code( rInner.c_str() ); 
    addG4Code( rOuter.c_str() ); 

    param.push_back( zPlane_id ) ;  
    param.push_back( rInner_id ) ;  
    param.push_back( rOuter_id ) ;  

    std::vector<std::string> keys = { 
                "phiStart",
                "phiTotal",
                "numZPlanes",
                "zPlane_id", 
                "rInner_id", 
                "rOuter_id" 
               } ; 

    setG4Param(param, keys );
    // no setG4args for the string ones
} 


void X4Solid::convertHype()
{  
    const G4Hype* const solid = static_cast<const G4Hype*>(m_solid);
    assert(solid); 
    LOG(info) << "\n" << *solid ; 

    bool only_inner = false ; 
    nnode* n = convertHype_(only_inner); 
    setRoot(n); 

    std::vector<double> param = {
                                  solid->GetInnerRadius(),
                                  solid->GetOuterRadius(),
                                  solid->GetInnerStereo(),
                                  solid->GetOuterStereo(),
                                  solid->GetZHalfLength()
                                } ;

    std::vector<std::string> keys = {
                                  "innerRadius",
                                  "outerRadius",
                                  "innerStereo",
                                  "outerStereo",
                                  "zHalfLength"
                                } ;
 
    setG4Param(param, keys);
    setG4Args(param, keys);
}

nnode* X4Solid::convertHype_(bool only_inner)
{
    const G4Hype* const solid = static_cast<const G4Hype*>(m_solid);

    // G4GDMLWriteSolids::HypeWrite
    float rmin = solid->GetInnerRadius()/mm ; 
    float rmax = solid->GetOuterRadius()/mm ;
    float inst = solid->GetInnerStereo()/degree ; 
    float outst = solid->GetOuterStereo()/degree ;
    float z = 2.0*solid->GetZHalfLength()/mm ; 

    bool has_inner = !only_inner && rmin > 0.f ; 

    nnode* inner = has_inner ? convertHype_(true) : NULL ;  

    float radius = only_inner ? rmin : rmax ;   
    float stereo = only_inner ? inst : outst ; 

    /*
     Opticks CSG_HYPERBOLOID uses
                x^2 +  y^2  =  r0^2 * (  (z/zf)^2  +  1 )

     G4Hype uses
                x^2 + y^2 = (z*tanphi)^2 + r^2
                x^2 + y^2 =  r0^2 * ( (z*tanphi/r0)^2 + 1 )

     So     
               tanphi/r0 = 1/zf

               zf = r0/tanphi

               tanphi = r0/zf

                  phi = arctan(r0/zf)

    */

    float zf = radius/std::tan(stereo*CLHEP::pi/180.) ;
    float z2 =  z/2.0 ; 
    float z1 = -z/2.0 ;
 
    nnode* cn = make_hyperboloid( radius, zf, z1, z2 );
    cn->label = BStr::concat(m_name, "_hyperboloid", NULL ) ; 
    
    nnode* result = inner ? nnode::make_operator(CSG_DIFFERENCE, cn, inner )  : cn ; 
    return result ; 
}


