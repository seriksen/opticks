//
// ********************************************************************
// * License and Disclaimer                                           *
// *                                                                  *
// * The  Geant4 software  is  copyright of the Copyright Holders  of *
// * the Geant4 Collaboration.  It is provided  under  the terms  and *
// * conditions of the Geant4 Software License,  included in the file *
// * LICENSE and available at  http://cern.ch/geant4/license .  These *
// * include a list of copyright holders.                             *
// *                                                                  *
// * Neither the authors of this software system, nor their employing *
// * institutes,nor the agencies providing financial support for this *
// * work  make  any representation or  warranty, express or implied, *
// * regarding  this  software system or assume any liability for its *
// * use.  Please see the license in the file  LICENSE  and URL above *
// * for the full disclaimer and the limitation of liability.         *
// *                                                                  *
// * This  code  implementation is the result of  the  scientific and *
// * technical work of the GEANT4 collaboration.                      *
// * By using,  copying,  modifying or  distributing the software (or *
// * any work based  on the software)  you  agree  to acknowledge its *
// * use  in  resulting  scientific  publications,  and indicate your *
// * acceptance of all terms of the Geant4 Software license.          *
// ********************************************************************
//
////////////////////////////////////////////////////////////////////////
// Optical Photon Boundary Process Class Implementation
////////////////////////////////////////////////////////////////////////
//
// File:        DsG4OpBoundaryProcess.cc
// Description: Discrete Process -- reflection/refraction at
//                                  optical interfaces
// Version:     1.1
// Created:     1997-06-18
// Modified:    1998-05-25 - Correct parallel component of polarization
//                           (thanks to: Stefano Magni + Giovanni Pieri)
//              1998-05-28 - NULL Rindex pointer before reuse
//                           (thanks to: Stefano Magni)
//              1998-06-11 - delete *sint1 in oblique reflection
//                           (thanks to: Giovanni Pieri)
//              1998-06-19 - move from GetLocalExitNormal() to the new 
//                           method: GetLocalExitNormal(&valid) to get
//                           the surface normal in all cases
//              1998-11-07 - NULL OpticalSurface pointer before use
//                           comparison not sharp for: std::abs(cost1) < 1.0
//                           remove sin1, sin2 in lines 556,567
//                           (thanks to Stefano Magni)
//              1999-10-10 - Accommodate changes done in DoAbsorption by
//                           changing logic in DielectricMetal
//              2001-10-18 - avoid Linux (gcc-2.95.2) warning about variables
//                           might be used uninitialized in this function
//                           moved E2_perp, E2_parl and E2_total out of 'if'
//              2003-11-27 - Modified line 168-9 to reflect changes made to
//                           G4OpticalSurface class ( by Fan Lei)
//              2004-02-02 - Set theStatus = Undefined at start of DoIt
//              2005-07-28 - add G4ProcessType to constructor
//              2006-11-04 - add capability of calculating the reflectivity
//                           off a metal surface by way of a complex index 
//                           of refraction - Thanks to Sehwook Lee and John 
//                           Hauptman (Dept. of Physics - Iowa State Univ.)
//
// Author:      Peter Gumplinger
// 		adopted from work by Werner Keil - April 2/96
// mail:        gum@triumf.ca
//
////////////////////////////////////////////////////////////////////////


#include <sstream>
#include <csignal>

#include "G4ios.hh"
#include "G4OpProcessSubType.hh"

#include "DsG4OpBoundaryProcess.h"
#include "G4GeometryTolerance.hh"
#include "G4Version.hh"

#ifdef SCB_DEBUG
#include "G4Event.hh"
#include "G4RunManager.hh"
#endif

#include "Opticks.hh"
#include "OpticksSwitches.h"
#include "CG4.hh"
#include "CTrack.hh"
#include "CMaterialLib.hh"
#include "PLOG.hh"

using CLHEP::eV ; 
using CLHEP::pi ; 
using CLHEP::halfpi ; 
using CLHEP::twopi ; 


const plog::Severity DsG4OpBoundaryProcess::LEVEL = PLOG::EnvLevel("DsG4OpBoundaryProcess", "DEBUG") ; 

/////////////////////////
// Class Implementation
/////////////////////////

        //////////////
        // Operators
        //////////////

// DsG4OpBoundaryProcess::operator=(const DsG4OpBoundaryProcess &right)
// {
// }

        /////////////////
        // Constructors
        /////////////////






std::string DsG4OpBoundaryProcess::description() const
{
    std::stringstream ss ; 
    ss << "DsG4OpBoundaryProcess " ; 
    ss << GetProcessName() ; 
    ss << " (" ; 
#ifdef WITH_REFLECT_CHEAT_DEBUG
    ss << "WITH_REFLECT_CHEAT_DEBUG " ; 
#endif
#ifdef WITH_ALIGN_DEV_DEBUG
    ss << "WITH_ALIGN_DEV_DEBUG " ; 
#endif
#ifdef WITH_FLAT_DEBUG
    ss << "WITH_FLAT_DEBUG " ; 
#endif
#ifdef SCB_BND_DEBUG
    ss << "SCB_BND_DEBUG " ; 
#endif
    ss << ")" ; 
    return ss.str(); 
}


DsG4OpBoundaryProcess::DsG4OpBoundaryProcess(CG4* g4, const G4String& processName, G4ProcessType type)
             : 
             G4VDiscreteProcess(processName, type),
             m_g4(g4),
             m_mlib(g4->getMaterialLib()),
             m_ok(g4->getOpticks()),
             m_dbgflat(m_ok->isDbgFlat()),
             m_boundarystepsigint(m_ok->getBoundaryStepSigInt()),
#ifdef WITH_REFLECT_CHEAT_DEBUG
             m_reflectcheat(m_ok->isReflectCheat()),
#endif
#ifdef SCB_DEBUG
             m_dbg(false),
             m_other(false),
             m_event_id(0),
             m_photon_id(0),
             m_step_id(0),
#endif
             m_DielectricMetal_LambertianReflection(0) 
{


        if ( verboseLevel > 0) {
           G4cout << GetProcessName() << " is created " << G4endl;
        }

        SetProcessSubType(fOpBoundary);

    LOG(LEVEL) << description() ; 

	theStatus = Ds::Undefined;
	theModel = glisur;
	theFinish = polished;
        theReflectivity = 1.;
        theEfficiency   = 0.;

        prob_sl = 0.;
        prob_ss = 0.;
        prob_bs = 0.;

        kCarTolerance = G4GeometryTolerance::GetInstance()
                        ->GetSurfaceTolerance();

        abNormalCounter = 0;


}



// DsG4OpBoundaryProcess::DsG4OpBoundaryProcess(const DsG4OpBoundaryProcess &right)
// {
// }

        ////////////////
        // Destructors
        ////////////////

DsG4OpBoundaryProcess::~DsG4OpBoundaryProcess(){}






G4VParticleChange*
DsG4OpBoundaryProcess::PostStepDoIt(const G4Track& aTrack, const G4Step& aStep)
{

     int step_id = aTrack.GetCurrentStepNumber() - 1 ; 


#ifdef WITH_FLAT_DEBUG
     LOG(LEVEL) << "[ step_id " << step_id ; 
     if( m_boundarystepsigint == step_id ) std::raise(SIGINT) ; 
#endif

    theStatus = Ds::Undefined;

    aParticleChange.Initialize(aTrack);
    G4double startVelocity = aTrack.GetVelocity() ; 
    aParticleChange.ProposeVelocity(startVelocity);  

    // SCB add above velocity proposal from geant4_10_02_p01/source/processes/optical/src/G4OpBoundaryProcess.cc
    //     NB the proposal is trumped by G4Track::CalculateVelocity unless
    //     G4Track::UseGivenVelocity is in force, that is done in CTrackingAction

#ifdef SCB_BND_DEBUG
    {
         //  confirmed wrong material groupvel bug
        float wavelength = CTrack::Wavelength(&aTrack);
        m_mlib->dumpGroupvelMaterial("bndary.PSDIP.beg", wavelength, startVelocity,0.f, m_g4->getStepId(), "startVelocity" );

        G4double calcVelocity = aTrack.CalculateVelocityForOpticalPhoton();
        m_mlib->dumpGroupvelMaterial("bndary.PSDIP.beg", wavelength, calcVelocity,0.f, m_g4->getStepId(), "calcVelocity");
    }
#endif    


#ifdef SCB_DEBUG
    {
        // TODO: move below stuff into CEventAction/CTrackAction and access via m_g4 like above
        const G4Event* event = G4RunManager::GetRunManager()->GetCurrentEvent() ;
        m_event_id = event->GetEventID() ;
        m_photon_id = aTrack.GetTrackID() - 1 ;
        m_step_id = aTrack.GetCurrentStepNumber() - 1 ;

        // debugging via record_id lists with --dindex and --oindex options
        // has the advantage of "knowing the future"
        // so using a single line selection like "TO BT BT BT BT SA" with ana/evt.py 
        // can just dump a single code path 
        //
        //     tconcentric-i::
        //
        //        In [2]: ab.b.sel = "TO BT BT BT BT SA"
        // 
        //        In [3]: ab.b.psel_dindex(limit=10, reverse=True)
        //        Out[3]: '--dindex=999999,999997,999996,999995,999994,999993,999992,999991,999990,999989'
        //

        m_dbg   = m_ok->isDbgPhoton(m_event_id, m_photon_id);     // eg --dindex=10,11,12
        m_other = m_ok->isOtherPhoton(m_event_id, m_photon_id);   // eg --oindex=15,16,17

        if(m_dbg || m_other) 
        LOG(info) 
            << " event_id " << std::setw(7) << m_event_id
            << " photon_id " << std::setw(7) << m_photon_id
            << " step_id " << std::setw(4) << m_step_id 
            << ( m_dbg ? "DBG " : " " )
            << ( m_other ? "OTHER " : " " )
            ;
    }
#endif    



    G4StepPoint* pPreStepPoint  = aStep.GetPreStepPoint();
    G4StepPoint* pPostStepPoint = aStep.GetPostStepPoint();

    if (pPostStepPoint->GetStepStatus() != fGeomBoundary)
    {
	    theStatus = Ds::NotAtBoundary;
#ifdef WITH_FLAT_DEBUG
        LOG(LEVEL) << "] NotAtBoundary" ; 
#endif
	    return G4VDiscreteProcess::PostStepDoIt(aTrack, aStep);
	}
	if (aTrack.GetStepLength()<=kCarTolerance/2)
    {
	    theStatus = Ds::StepTooSmall;

        G4VParticleChange* change = G4VDiscreteProcess::PostStepDoIt(aTrack, aStep);

        //LOG(info) << " StepTooSmall" ;   // (*lldb*) StepTooSmall

#ifdef WITH_FLAT_DEBUG
        LOG(LEVEL) << "] StepTooSmall" 
                   << " StepLength " << aTrack.GetStepLength()
                   << " tol/2 " << kCarTolerance/2  
                   ; 
#endif
	    return change ; 
	}

	Material1 = pPreStepPoint  -> GetMaterial();
	Material2 = pPostStepPoint -> GetMaterial();

    const G4DynamicParticle* aParticle = aTrack.GetDynamicParticle();

	thePhotonMomentum = aParticle->GetTotalMomentum();
    OldMomentum       = aParticle->GetMomentumDirection();
	OldPolarization   = aParticle->GetPolarization();

    G4ThreeVector theGlobalPoint = pPostStepPoint->GetPosition();

    G4Navigator* theNavigator =
                     G4TransportationManager::GetTransportationManager()->
                                              GetNavigatorForTracking();

    G4ThreeVector theLocalPoint = theNavigator->
                                      GetGlobalToLocalTransform().
                                      TransformPoint(theGlobalPoint);

    G4ThreeVector theLocalNormal;   // Normal points back into volume

    G4bool valid;
    theLocalNormal = theNavigator->GetLocalExitNormal(&valid);

    if (valid) 
    {
        theLocalNormal = -theLocalNormal;
    }
    else 
    {
         G4cerr << " DsG4OpBoundaryProcess/PostStepDoIt(): "
                << " The Navigator reports that it returned an invalid normal"
                << G4endl;
    }

    theGlobalNormal = theNavigator->GetLocalToGlobalTransform().TransformAxis(theLocalNormal);

    if(theGlobalNormal.mag() == 0) 
    {
        abNormalCounter++;
        std::cout << "Because of normal = 0, the number of the killed optical photons is " << abNormalCounter << std::endl;
        aParticleChange.ProposeTrackStatus(fStopAndKill);

#ifdef WITH_FLAT_DEBUG
        LOG(LEVEL) << "] abNormal StopAndKill " ; 
#endif
        return G4VDiscreteProcess::PostStepDoIt(aTrack, aStep);
    }

    if (OldMomentum * theGlobalNormal > 0.0) 
    {
#ifdef G4DEBUG_OPTICAL
         G4cerr << " DsG4OpBoundaryProcess/PostStepDoIt(): "
                << " theGlobalNormal points the wrong direction "
                << G4endl;
#endif
         theGlobalNormal = -theGlobalNormal;
    }

	G4MaterialPropertiesTable* aMaterialPropertiesTable;
    G4MaterialPropertyVector* Rindex;

	aMaterialPropertiesTable = Material1->GetMaterialPropertiesTable();
    if (aMaterialPropertiesTable) 
    {
	    Rindex = aMaterialPropertiesTable->GetProperty("RINDEX");
	}
	else 
    {
	    theStatus = Ds::NoRINDEX;
		aParticleChange.ProposeTrackStatus(fStopAndKill);

#ifdef WITH_FLAT_DEBUG
        LOG(LEVEL) << "] NoRINDEX StopAndKill " ; 
#endif
		return G4VDiscreteProcess::PostStepDoIt(aTrack, aStep);
	}

    if (Rindex) 
    {
#if ( G4VERSION_NUMBER > 1000 )
		Rindex1 = Rindex->Value(thePhotonMomentum);

        //LOG(info) << "DsG4OpBoundaryProcess::PostStepDoIt" 
        //          << " thePhotonMomentum (eV) " << thePhotonMomentum/eV
        //          << " Rindex1 " << Rindex1
        //          ; 

#else
		Rindex1 = Rindex->GetProperty(thePhotonMomentum);
#endif
	}
	else 
    {
	    theStatus = Ds::NoRINDEX;
        aParticleChange.ProposeTrackStatus(fStopAndKill);

#ifdef WITH_FLAT_DEBUG
        LOG(LEVEL) << "] NoRINDEX StopAndKill " ; 
#endif

		return G4VDiscreteProcess::PostStepDoIt(aTrack, aStep);
	}

    theModel = glisur;
    theFinish = polished;

    G4SurfaceType type = dielectric_dielectric;

    Rindex = NULL;
    OpticalSurface = NULL;

    G4LogicalSurface* Surface = NULL;

    Surface = G4LogicalBorderSurface::GetSurface(pPreStepPoint ->GetPhysicalVolume(),pPostStepPoint->GetPhysicalVolume());

    if (Surface == NULL)
    {
	    G4bool enteredDaughter=(pPostStepPoint->GetPhysicalVolume()->GetMotherLogical() == pPreStepPoint->GetPhysicalVolume()->GetLogicalVolume());
	    if(enteredDaughter)
        {
	         Surface = G4LogicalSkinSurface::GetSurface(pPostStepPoint->GetPhysicalVolume()->GetLogicalVolume());
	         if(Surface == NULL)
	               Surface = G4LogicalSkinSurface::GetSurface(pPreStepPoint->GetPhysicalVolume()->GetLogicalVolume());
	    }
	    else 
        {
	         Surface = G4LogicalSkinSurface::GetSurface(pPreStepPoint->GetPhysicalVolume()->GetLogicalVolume());
	         if(Surface == NULL)
	               Surface = G4LogicalSkinSurface::GetSurface(pPostStepPoint->GetPhysicalVolume()->GetLogicalVolume());
	    }
	}

	if (Surface) OpticalSurface = dynamic_cast <G4OpticalSurface*> (Surface->GetSurfaceProperty());

	if (OpticalSurface) 
    {
#ifdef SCB_BND_DEBUG
          if(m_dbg || m_other)
          LOG(info) << "found OpticalSurface " << OpticalSurface->GetName() ; 
#endif

          type      = OpticalSurface->GetType();
	      theModel  = OpticalSurface->GetModel();
	      theFinish = OpticalSurface->GetFinish();

	      aMaterialPropertiesTable = OpticalSurface->GetMaterialPropertiesTable();

          if (aMaterialPropertiesTable) 
          {
              if (theFinish == polishedbackpainted || theFinish == groundbackpainted ) 
              {
                  Rindex = aMaterialPropertiesTable->GetProperty("RINDEX");
	              if (Rindex) 
                  {
#if ( G4VERSION_NUMBER > 1000 )
                      Rindex2 = Rindex->Value(thePhotonMomentum);
#else
                      Rindex2 = Rindex->GetProperty(thePhotonMomentum);
#endif
                   }
                   else 
                   {
		              theStatus = Ds::NoRINDEX;
                      aParticleChange.ProposeTrackStatus(fStopAndKill);


#ifdef WITH_FLAT_DEBUG
                      LOG(LEVEL) << "] NoRINDEX StopAndKill " ; 
#endif

                      return G4VDiscreteProcess::PostStepDoIt(aTrack, aStep);
                  }
              }

              G4MaterialPropertyVector* PropertyPointer;
              G4MaterialPropertyVector* PropertyPointer1;
              G4MaterialPropertyVector* PropertyPointer2;

              PropertyPointer = aMaterialPropertiesTable->GetProperty("REFLECTIVITY");
              PropertyPointer1 = aMaterialPropertiesTable->GetProperty("REALRINDEX");
              PropertyPointer2 = aMaterialPropertiesTable->GetProperty("IMAGINARYRINDEX");

              iTE = 1;
              iTM = 1;

              if (PropertyPointer) 
              {

#if ( G4VERSION_NUMBER > 1000 )
                 theReflectivity = PropertyPointer->Value(thePhotonMomentum);
#else
                 theReflectivity = PropertyPointer->GetProperty(thePhotonMomentum);
#endif

/*
   // SCB comment out ESRAir reflectivity diddle
                 if(OpticalSurface->GetName().contains("ESRAir")) 
                 {
                      G4double inciAngle = GetIncidentAngle();
                      //ESR in air
                      if(inciAngle*180./pi > 40) 
                      {
                          theReflectivity = (theReflectivity - 0.993) + 0.973572 + 9.53233e-04*(inciAngle*180./pi) - 1.22184e-05*((inciAngle*180./pi))*((inciAngle*180./pi));
                      }
                   //ESR in oil
                   //if(inciAngle*180./pi > 40 && inciAngle*180./pi <= 50) {
                   //  theReflectivity = (theReflectivity - 0.993) + (inciAngle*180./pi - 40)*0.99/10. + (50 - inciAngle*180./pi)*0.993/10.;
                   //}
                   //else if (inciAngle*180./pi > 50 && inciAngle*180./pi <= 60) {
                   //  theReflectivity = (theReflectivity - 0.993) + (inciAngle*180./pi - 50)*0.94/10. + (60 - inciAngle*180./pi)*0.99/10.;
                   //}
                   //else if (inciAngle*180./pi > 60 && inciAngle*180./pi <= 65) {
                   //  theReflectivity = (theReflectivity - 0.993) + (inciAngle*180./pi - 60)*0.62/5. + (65 - inciAngle*180./pi)*0.94/5.;
                   //}
                   //else if (inciAngle*180./pi > 65 && inciAngle*180./pi <= 80) {
                   //  theReflectivity = (theReflectivity - 0.993) + 0.62;
                   //}
                   //else if (inciAngle*180./pi > 80 && inciAngle*180./pi <= 85) {
                   //  theReflectivity = (theReflectivity - 0.993) + (inciAngle*180./pi - 80)*0.72/5. + (85 - inciAngle*180./pi)*0.62/5.;
                   //}
                   //else if (inciAngle*180./pi > 85 && inciAngle*180./pi < 90) {
                   //  theReflectivity = (theReflectivity - 0.993) + 0.72;
                   //}
                 }
*/

              } else if (PropertyPointer1 && PropertyPointer2) {

#if ( G4VERSION_NUMBER > 1000 )
                 G4double RealRindex = PropertyPointer1->Value(thePhotonMomentum);
                 G4double ImaginaryRindex = PropertyPointer2->Value(thePhotonMomentum);
#else
                 G4double RealRindex = PropertyPointer1->GetProperty(thePhotonMomentum);
                 G4double ImaginaryRindex = PropertyPointer2->GetProperty(thePhotonMomentum);
#endif

                 // calculate FacetNormal
                 if ( theFinish == ground ) {
                    theFacetNormal = GetFacetNormal(OldMomentum, theGlobalNormal);
                 } else {
                    theFacetNormal = theGlobalNormal;
                 }

                 G4double PdotN = OldMomentum * theFacetNormal;
                 cost1 = -PdotN;

                 if (std::abs(cost1) < 1.0 - kCarTolerance) {
                    sint1 = std::sqrt(1. - cost1*cost1);
                 } else {
                    sint1 = 0.0;
                 }

                 G4ThreeVector A_trans, A_paral, E1pp, E1pl;
                 G4double E1_perp, E1_parl;

                 if (sint1 > 0.0 ) {
                    A_trans = OldMomentum.cross(theFacetNormal);
                    A_trans = A_trans.unit();
                    E1_perp = OldPolarization * A_trans;
                    E1pp    = E1_perp * A_trans;
                    E1pl    = OldPolarization - E1pp;
                    E1_parl = E1pl.mag();
                 }
                 else {
                    A_trans  = OldPolarization;
                    // Here we Follow Jackson's conventions and we set the
                    // parallel component = 1 in case of a ray perpendicular
                    // to the surface
                    E1_perp  = 0.0;
                    E1_parl  = 1.0;
                 }

                 //calculate incident angle
                 G4double incidentangle = GetIncidentAngle();

                 //calculate the reflectivity depending on incident angle,
                 //polarization and complex refractive

                 theReflectivity =
                            GetReflectivity(E1_perp, E1_parl, incidentangle,
                                                 RealRindex, ImaginaryRindex);

              } else {
                 theReflectivity = 1.0;
              }

              PropertyPointer = aMaterialPropertiesTable->GetProperty("EFFICIENCY");
              if (PropertyPointer) 
              {
#if ( G4VERSION_NUMBER > 1000 )
                      theEfficiency = PropertyPointer->Value(thePhotonMomentum);
#else
                      theEfficiency = PropertyPointer->GetProperty(thePhotonMomentum);
#endif


/*
                      LOG(info) << "OpticalSurface "
                                << " name " << OpticalSurface->GetName() 
                                << " thePhotonMomentum (eV) " << thePhotonMomentum/eV
                                << " theReflectivity " << theReflectivity
                                << " theEfficiency " << theEfficiency
                                << ( theModel == unified ? " unified " : "-" )
                                << ( theModel == glisur ?  " glisur " : "-" )
                                << ( type == dielectric_dielectric ? " dielectric_dielectric " : "." )
                                << ( type == dielectric_metal ? " dielectric_metal " : "." )
                                << ( theFinish == ground ? " ground " : "-" )
                                << ( theFinish == polished ? " polished " : "-" )
                                << " m1 " << Material1->GetName()
                                << " m2 " << Material2->GetName()
                                ; 
*/

              } 
              else 
              {
                  theEfficiency = 0.0;
              }

	          if ( theModel == unified ) 
              {
	              PropertyPointer = aMaterialPropertiesTable->GetProperty("SPECULARLOBECONSTANT");
	              if (PropertyPointer) 
                  {
#if ( G4VERSION_NUMBER > 1000 )
                      prob_sl = PropertyPointer->Value(thePhotonMomentum);
#else
                      prob_sl = PropertyPointer->GetProperty(thePhotonMomentum);
#endif
                  } 
                  else 
                  {
                      prob_sl = 0.0;
                  }

	              PropertyPointer = aMaterialPropertiesTable->GetProperty("SPECULARSPIKECONSTANT");
	              if (PropertyPointer) 
                  {
#if ( G4VERSION_NUMBER > 1000 )
                      prob_ss = PropertyPointer->Value(thePhotonMomentum);
#else
                      prob_ss = PropertyPointer->GetProperty(thePhotonMomentum);
#endif
                  } 
                  else 
                  {
                      prob_ss = 0.0;
                  }

	              PropertyPointer = aMaterialPropertiesTable->GetProperty("BACKSCATTERCONSTANT");
	              if (PropertyPointer) 
                  {
#if ( G4VERSION_NUMBER > 1000 )
                      prob_bs = PropertyPointer->Value(thePhotonMomentum);
#else
                      prob_bs = PropertyPointer->GetProperty(thePhotonMomentum);
#endif
                  } 
                  else 
                  {
                      prob_bs = 0.0;
                  }
	          }  // unified
	   }           // OpticalSurface has MPT
       else if (theFinish == polishedbackpainted || theFinish == groundbackpainted ) 
       {
               aParticleChange.ProposeTrackStatus(fStopAndKill);

#ifdef WITH_FLAT_DEBUG
                LOG(LEVEL) << "] painted StopAndKill " ; 
#endif

               return G4VDiscreteProcess::PostStepDoIt(aTrack, aStep);
       }
   }    // boundary has OpticalSurface




        if (type == dielectric_dielectric ) 
        {
            if (theFinish == polished || theFinish == ground )   // polished is default even without OpticalSurface
            {
	            if (Material1 == Material2)
                {
		            theStatus = Ds::SameMaterial;

#ifdef WITH_FLAT_DEBUG
                    LOG(LEVEL) << "] SameMaterial " ; 
#endif

		            return G4VDiscreteProcess::PostStepDoIt(aTrack, aStep);
 	            }
                aMaterialPropertiesTable = Material2->GetMaterialPropertiesTable();
                if (aMaterialPropertiesTable) Rindex = aMaterialPropertiesTable->GetProperty("RINDEX");

                if (Rindex) 
                {
#if ( G4VERSION_NUMBER > 1000 )
                    Rindex2 = Rindex->Value(thePhotonMomentum);
#else
                    Rindex2 = Rindex->GetProperty(thePhotonMomentum);
#endif
                }
                else 
                {
		            theStatus = Ds::NoRINDEX;
                    aParticleChange.ProposeTrackStatus(fStopAndKill);

#ifdef WITH_FLAT_DEBUG
                    LOG(LEVEL) << "] NoRINDEX " ; 
#endif

                    return G4VDiscreteProcess::PostStepDoIt(aTrack, aStep);
	            }
            }
        }

        if ( verboseLevel > 0 ) 
        {
            G4cout << " Photon at Boundary! " << G4endl;
            G4cout << " Old Momentum Direction: " << OldMomentum     << G4endl;
            G4cout << " Old Polarization:       " << OldPolarization << G4endl;
        }

	    if (type == dielectric_metal) 
        {
	        DielectricMetal();
            // Uncomment the following lines if you wish to have 
            //         Transmission instead of Absorption
            // if (theStatus == Absorption) {
            //    return G4VDiscreteProcess::PostStepDoIt(aTrack, aStep);
            // }
	    }
        else if (type == dielectric_dielectric) 
        {
            if ( theFinish == polishedfrontpainted || theFinish == groundfrontpainted ) 
            {


#ifdef WITH_ALIGN_DEV_DEBUG
#ifdef WITH_REFLECT_CHEAT_DEBUG
                G4double _u = m_reflectcheat ? m_g4->getCtxRecordFraction()  : CG4UniformRand("DiDiAbsorbDetectReflect", -1) ;   // --reflectcheat 
#else
                G4double _u = CG4UniformRand("DiDiAbsorbDetectReflect", -1) ;  
#endif
                bool _reflect = _u < theReflectivity ;

                if( m_dbgflat )
                { 
                    m_g4->addRandomCut( "Reflectivity", theReflectivity );  
                    m_g4->addRandomNote( "reflect", _reflect );  
                }

                if( !_reflect )    // (*lldb*) DiDiAbsorbDetectReflect
#else
                if( !G4BooleanRand(theReflectivity) ) 
#endif
                {
                    DoAbsorption();
                }
                else 
                {
                    if ( theFinish == groundfrontpainted ) theStatus = Ds::LambertianReflection;
                    DoReflection();
                }
            }
            else 
            {
                DielectricDielectric();
            }
        }
        else 
        {
	        G4cerr << " Error: G4BoundaryProcess: illegal boundary type " << G4endl;

#ifdef WITH_FLAT_DEBUG
            LOG(LEVEL) << "] Illegal " ; 
#endif

	        return G4VDiscreteProcess::PostStepDoIt(aTrack, aStep);
	    }

        NewMomentum = NewMomentum.unit();
        NewPolarization = NewPolarization.unit();



        if ( verboseLevel > 0) 
        {
		    G4cout << " New Momentum Direction: " << NewMomentum     << G4endl;
		    G4cout << " New Polarization:       " << NewPolarization << G4endl;
		    if ( theStatus == Ds::Undefined ) G4cout << " *** Undefined *** " << G4endl;
		    if ( theStatus == Ds::FresnelRefraction ) G4cout << " *** FresnelRefraction *** " << G4endl;
		    if ( theStatus == Ds::FresnelReflection )
			    G4cout << " *** FresnelReflection *** " << G4endl;
		    if ( theStatus == Ds::TotalInternalReflection )
			    G4cout << " *** TotalInternalReflection *** " << G4endl;
		    if ( theStatus == Ds::LambertianReflection )
			    G4cout << " *** LambertianReflection *** " << G4endl;
		    if ( theStatus == Ds::LobeReflection )
			    G4cout << " *** LobeReflection *** " << G4endl;
		    if ( theStatus == Ds::SpikeReflection )
			G4cout << " *** SpikeReflection *** " << G4endl;
		if ( theStatus == Ds::BackScattering )
			G4cout << " *** BackScattering *** " << G4endl;
		if ( theStatus == Ds::Absorption )
			G4cout << " *** Absorption *** " << G4endl;
		if ( theStatus == Ds::Detection )
			G4cout << " *** Detection *** " << G4endl;
                if ( theStatus == Ds::NotAtBoundary )
                        G4cout << " *** NotAtBoundary *** " << G4endl;
                if ( theStatus == Ds::SameMaterial )
                        G4cout << " *** SameMaterial *** " << G4endl;
                if ( theStatus == Ds::StepTooSmall )
                        G4cout << " *** StepTooSmall *** " << G4endl;
                if ( theStatus == Ds::NoRINDEX )
                        G4cout << " *** NoRINDEX *** " << G4endl;
        }

	aParticleChange.ProposeMomentumDirection(NewMomentum);
	aParticleChange.ProposePolarization(NewPolarization);


#ifdef GEANT4_BT_GROUPVEL_FIX
       // from /usr/local/opticks/externals/g4/geant4_10_02_p01/source/processes/optical/src/G4OpBoundaryProcess.cc
       //if ( theStatus == FresnelRefraction || theStatus == Transmission ) 
       if ( theStatus == Ds::FresnelRefraction ) 
       {
           G4MaterialPropertyVector* groupvel = Material2->GetMaterialPropertiesTable()->GetProperty("GROUPVEL");
           G4double finalVelocity = groupvel->Value(thePhotonMomentum);
           aParticleChange.ProposeVelocity(finalVelocity);

#ifdef SCB_BND_DEBUG
           m_mlib->dumpGroupvelMaterial("bndary.PSDIP.end", CTrack::Wavelength(thePhotonMomentum), finalVelocity, 0.f, m_g4->getStepId());
#endif
        }    
#endif

#ifdef WITH_FLAT_DEBUG
        LOG(LEVEL) << "] Tail " ; 
#endif
        return G4VDiscreteProcess::PostStepDoIt(aTrack, aStep);   // (*lldb*) ExitPostStepDoIt
}

G4ThreeVector
DsG4OpBoundaryProcess::GetFacetNormal(const G4ThreeVector& Momentum,
			            const G4ThreeVector&  Normal ) const
{
        G4ThreeVector FacetNormal;

	if (theModel == unified) {

	/* This function code alpha to a random value taken from the
           distribution p(alpha) = g(alpha; 0, sigma_alpha)*std::sin(alpha),
           for alpha > 0 and alpha < 90, where g(alpha; 0, sigma_alpha)
           is a gaussian distribution with mean 0 and standard deviation
           sigma_alpha.  */

	   G4double alpha;

	   G4double sigma_alpha = 0.0;
	   if (OpticalSurface) sigma_alpha = OpticalSurface->GetSigmaAlpha();

	   G4double f_max = std::min(1.0,4.*sigma_alpha);

	   do {
	      do {
	         alpha = G4RandGauss::shoot(0.0,sigma_alpha);
	      } while (CG4UniformRand(__FILE__,__LINE__)*f_max > std::sin(alpha) || alpha >= halfpi );

	      G4double phi = CG4UniformRand(__FILE__,__LINE__)*twopi;

	      G4double SinAlpha = std::sin(alpha);
	      G4double CosAlpha = std::cos(alpha);
              G4double SinPhi = std::sin(phi);
              G4double CosPhi = std::cos(phi);

              G4double unit_x = SinAlpha * CosPhi;
              G4double unit_y = SinAlpha * SinPhi;
              G4double unit_z = CosAlpha;

	      FacetNormal.setX(unit_x);
	      FacetNormal.setY(unit_y);
	      FacetNormal.setZ(unit_z);

	      G4ThreeVector tmpNormal = Normal;

	      FacetNormal.rotateUz(tmpNormal);
	   } while (Momentum * FacetNormal >= 0.0);
	}
	else {

	   G4double  polish = 1.0;
	   if (OpticalSurface) polish = OpticalSurface->GetPolish();

           if (polish < 1.0) {
              do {
                 G4ThreeVector smear;
                 do {
                    smear.setX(2.*CG4UniformRand(__FILE__,__LINE__)-1.0);
                    smear.setY(2.*CG4UniformRand(__FILE__,__LINE__)-1.0);
                    smear.setZ(2.*CG4UniformRand(__FILE__,__LINE__)-1.0);
                 } while (smear.mag()>1.0);
                 smear = (1.-polish) * smear;
                 FacetNormal = Normal + smear;
              } while (Momentum * FacetNormal >= 0.0);
              FacetNormal = FacetNormal.unit();
	   }
           else {
              FacetNormal = Normal;
           }
	}
	return FacetNormal;
}

void DsG4OpBoundaryProcess::DielectricMetal()
{
        G4int n = 0;

	do {

           n++;

           if( !G4BooleanRand(theReflectivity) && n == 1 ) {

             // Comment out DoAbsorption and uncomment theStatus = Absorption;
             // if you wish to have Transmission instead of Absorption

             DoAbsorption();
             // theStatus = Absorption;
             break;

           }
           else {

             if ( theModel == glisur || theFinish == polished ) {

                DoReflection();

             } else {

                if ( n == 1 ) ChooseReflection();   //  this assumes unified model with prob_ss prob_bs prob_sl ...
                                                                                
                if ( theStatus == Ds::LambertianReflection ) 
                {
                   m_DielectricMetal_LambertianReflection += 1 ; 
                   //LOG(info) << "DsG4OpBoundaryProcess::DielectricMetal LambertianReflection " << m_DielectricMetal_LambertianReflection ;
                   DoReflection();
                }
                else if ( theStatus == Ds::BackScattering ) 
                {
                   NewMomentum = -OldMomentum;
                   NewPolarization = -OldPolarization;
                }
                else {

                   if(theStatus==Ds::LobeReflection)theFacetNormal = GetFacetNormal(OldMomentum,theGlobalNormal);

                   G4double PdotN = OldMomentum * theFacetNormal;
                   NewMomentum = OldMomentum - (2.*PdotN)*theFacetNormal;
                   G4double EdotN = OldPolarization * theFacetNormal;

                   G4ThreeVector A_trans, A_paral;

                   if (sint1 > 0.0 ) {
                      A_trans = OldMomentum.cross(theFacetNormal);
                      A_trans = A_trans.unit();
                   } else {
                      A_trans  = OldPolarization;
                   }
                   A_paral   = NewMomentum.cross(A_trans);
                   A_paral   = A_paral.unit();

                   if(iTE>0&&iTM>0) {
                     NewPolarization = 
                           -OldPolarization + (2.*EdotN)*theFacetNormal;
                   } else if (iTE>0) {
                     NewPolarization = -A_trans;
                   } else if (iTM>0) {
                     NewPolarization = -A_paral;
                   }

                }

             }

             OldMomentum = NewMomentum;
             OldPolarization = NewPolarization;

	   }

	} while (NewMomentum * theGlobalNormal < 0.0);
}

void DsG4OpBoundaryProcess::DielectricDielectric()
{
	G4bool Inside = false;
	G4bool Swap = false;

	leap:

        G4bool Through = false;
	G4bool Done = false;

	do {

	   if (Through) {
	      Swap = !Swap;
	      Through = false;
	      theGlobalNormal = -theGlobalNormal;

#ifdef SCB_BND_DEBUG
          if(m_dbg)
          {
               LOG(info) << "DsG4OpBoundaryProcess::DielectricDielectric (before swap)"
                         << " m1 " << std::setw(20) << Material1->GetName()
                         << " m2 " << std::setw(20) << Material2->GetName()
                         ;
         }
#endif
	      G4SwapPtr(Material1,Material2);
#ifdef SCB_BND_DEBUG
          if(m_dbg)
          {
               LOG(info) << "DsG4OpBoundaryProcess::DielectricDielectric (after swap)"
                         << " m1 " << std::setw(20) << Material1->GetName()
                         << " m2 " << std::setw(20) << Material2->GetName()
                         ;
         }
#endif
	      G4SwapObj(&Rindex1,&Rindex2);
	   }

	   if ( theFinish == ground || theFinish == groundbackpainted ) {
		theFacetNormal = 
			     GetFacetNormal(OldMomentum,theGlobalNormal);
	   }
	   else {
		theFacetNormal = theGlobalNormal;
	   }

	   G4double PdotN = OldMomentum * theFacetNormal;
	   G4double EdotN = OldPolarization * theFacetNormal;

	   cost1 = - PdotN;
	   if (std::abs(cost1) < 1.0-kCarTolerance){
	      sint1 = std::sqrt(1.-cost1*cost1);
	      sint2 = sint1*Rindex1/Rindex2;     // *** Snell's Law ***
	   }
	   else {
	      sint1 = 0.0;
	      sint2 = 0.0;
	   }

	   if (sint2 >= 1.0) {

	      // Simulate total internal reflection

	      if (Swap) Swap = !Swap;

              theStatus = Ds::TotalInternalReflection;

	      if ( theModel == unified && theFinish != polished )
						     ChooseReflection();

	      if ( theStatus == Ds::LambertianReflection ) {
		 DoReflection();
	      }
	      else if ( theStatus == Ds::BackScattering ) {
		 NewMomentum = -OldMomentum;
		 NewPolarization = -OldPolarization;
	      }
	      else {

                 PdotN = OldMomentum * theFacetNormal;
		 NewMomentum = OldMomentum - (2.*PdotN)*theFacetNormal;
		 EdotN = OldPolarization * theFacetNormal;
		 NewPolarization = -OldPolarization + (2.*EdotN)*theFacetNormal;

	      }
	   }
	   else if (sint2 < 1.0) {

	      // Calculate amplitude for transmission (Q = P x N)

	      if (cost1 > 0.0) {
	         cost2 =  std::sqrt(1.-sint2*sint2);
	      }
	      else {
	         cost2 = -std::sqrt(1.-sint2*sint2);
	      }

	      G4ThreeVector A_trans, A_paral, E1pp, E1pl;
	      G4double E1_perp, E1_parl;

	      if (sint1 > 0.0) {
	         A_trans = OldMomentum.cross(theFacetNormal);
                 A_trans = A_trans.unit();
	         E1_perp = OldPolarization * A_trans;
                 E1pp    = E1_perp * A_trans;
                 E1pl    = OldPolarization - E1pp;
                 E1_parl = E1pl.mag();
              }
	      else {
	         A_trans  = OldPolarization;
	         // Here we Follow Jackson's conventions and we set the
	         // parallel component = 1 in case of a ray perpendicular
	         // to the surface
	         E1_perp  = 0.0;
	         E1_parl  = 1.0;
	      }

              G4double s1 = Rindex1*cost1;
              G4double E2_perp = 2.*s1*E1_perp/(Rindex1*cost1+Rindex2*cost2);
              G4double E2_parl = 2.*s1*E1_parl/(Rindex2*cost1+Rindex1*cost2);
              G4double E2_total = E2_perp*E2_perp + E2_parl*E2_parl;
              G4double s2 = Rindex2*cost2*E2_total;

              G4double TransCoeff;

	      if (cost1 != 0.0) {
	         TransCoeff = s2/s1;
	      }
	      else {
	         TransCoeff = 0.0;
	      }

	      G4double E2_abs, C_parl, C_perp;

#ifdef WITH_ALIGN_DEV_DEBUG
#ifdef WITH_REFLECT_CHEAT_DEBUG 
          G4double _u = m_reflectcheat ? m_g4->getCtxRecordFraction()  : CG4UniformRand("DiDiTransCoeff",-1) ;   // --reflectcheat 
#else
          G4double _u = CG4UniformRand("DiDiTransCoeff",-1) ;  
#endif
          bool _reflect = _u > TransCoeff ;  

          if(m_dbgflat)
          {
              m_g4->addRandomNote("reflect", _reflect ) ; 
              m_g4->addRandomCut("TransCoeff", TransCoeff ) ; 
          }

	      if ( _reflect ) {    // (*lldb*) DiDiTransCoeff
#else
          if ( !G4BooleanRand(TransCoeff) ) {
#endif

	         // Simulate reflection

                 if (Swap) Swap = !Swap;

		 theStatus = Ds::FresnelReflection;

		 if ( theModel == unified && theFinish != polished )
						     ChooseReflection();

		 if ( theStatus == Ds::LambertianReflection ) {
		    DoReflection();
		 }
		 else if ( theStatus == Ds::BackScattering ) {
		    NewMomentum = -OldMomentum;
		    NewPolarization = -OldPolarization;
		 }
		 else {

                    PdotN = OldMomentum * theFacetNormal;
	            NewMomentum = OldMomentum - (2.*PdotN)*theFacetNormal;

	            if (sint1 > 0.0) {   // incident ray oblique

		       E2_parl   = Rindex2*E2_parl/Rindex1 - E1_parl;
		       E2_perp   = E2_perp - E1_perp;
		       E2_total  = E2_perp*E2_perp + E2_parl*E2_parl;
                       A_paral   = NewMomentum.cross(A_trans);
                       A_paral   = A_paral.unit();
		       E2_abs    = std::sqrt(E2_total);
		       C_parl    = E2_parl/E2_abs;
		       C_perp    = E2_perp/E2_abs;

                       NewPolarization = C_parl*A_paral + C_perp*A_trans;

	            }

	            else {               // incident ray perpendicular

	               if (Rindex2 > Rindex1) {
		          NewPolarization = - OldPolarization;
	               }
	               else {
	                  NewPolarization =   OldPolarization;
	               }

	            }
	         }
	      }
	      else { // photon gets transmitted

	         // Simulate transmission/refraction

		 Inside = !Inside;
		 Through = true;
		 theStatus = Ds::FresnelRefraction;

	         if (sint1 > 0.0) {      // incident ray oblique

		    G4double alpha = cost1 - cost2*(Rindex2/Rindex1);
		    NewMomentum = OldMomentum + alpha*theFacetNormal;
		    NewMomentum = NewMomentum.unit();
		    PdotN = -cost2;
                    A_paral = NewMomentum.cross(A_trans);
                    A_paral = A_paral.unit();
		    E2_abs     = std::sqrt(E2_total);
		    C_parl     = E2_parl/E2_abs;
		    C_perp     = E2_perp/E2_abs;

                    NewPolarization = C_parl*A_paral + C_perp*A_trans;

	         }
	         else {                  // incident ray perpendicular

		    NewMomentum = OldMomentum;
		    NewPolarization = OldPolarization;

	         }
	      }
	   }

	   OldMomentum = NewMomentum.unit();
	   OldPolarization = NewPolarization.unit();

	   if (theStatus == Ds::FresnelRefraction) {
	      Done = (NewMomentum * theGlobalNormal <= 0.0);
	   } 
	   else {
	      Done = (NewMomentum * theGlobalNormal >= 0.0);
	   }

	} while (!Done);

	if (Inside && !Swap) 
    {
          if( theFinish == polishedbackpainted || theFinish == groundbackpainted ) 
          {

	          if( !G4BooleanRand(theReflectivity) ) 
              {
		          DoAbsorption();
              }
	          else 
              {
		           if (theStatus != Ds::FresnelRefraction ) 
                   {
		               theGlobalNormal = -theGlobalNormal;
	               }
	               else 
                   {
		               Swap = !Swap;
		               G4SwapPtr(Material1,Material2);
		               G4SwapObj(&Rindex1,&Rindex2);
	               }
		           if ( theFinish == groundbackpainted )
				 	     theStatus = Ds::LambertianReflection;

	               DoReflection();

	               theGlobalNormal = -theGlobalNormal;
		           OldMomentum = NewMomentum;

	               goto leap;
	         }
	     }
	}
}

// GetMeanFreePath
// ---------------
//
G4double DsG4OpBoundaryProcess::GetMeanFreePath(const G4Track& ,
                                              G4double ,
                                              G4ForceCondition* condition)
{
	*condition = Forced;

	return DBL_MAX;
}

G4double DsG4OpBoundaryProcess::GetIncidentAngle() 
{
    G4double PdotN = OldMomentum * theFacetNormal;
    G4double magP= OldMomentum.mag();
    G4double magN= theFacetNormal.mag();
    G4double incidentangle = pi - std::acos(PdotN/(magP*magN));

    return incidentangle;
}

G4double DsG4OpBoundaryProcess::GetReflectivity(G4double E1_perp,
                                              G4double E1_parl,
                                              G4double incidentangle,
                                              G4double RealRindex,
                                              G4double ImaginaryRindex)
{

  G4complex Reflectivity, Reflectivity_TE, Reflectivity_TM;
  G4complex N(RealRindex, ImaginaryRindex);
  G4complex CosPhi;

  G4complex u(1,0);           //unit number 1

  G4complex numeratorTE;      // E1_perp=1 E1_parl=0 -> TE polarization
  G4complex numeratorTM;      // E1_parl=1 E1_perp=0 -> TM polarization
  G4complex denominatorTE, denominatorTM;
  G4complex rTM, rTE;

  // Following two equations, rTM and rTE, are from: "Introduction To Modern
  // Optics" written by Fowles

  CosPhi=std::sqrt(u-((std::sin(incidentangle)*std::sin(incidentangle))/(N*N)));

  numeratorTE   = std::cos(incidentangle) - N*CosPhi;
  denominatorTE = std::cos(incidentangle) + N*CosPhi;
  rTE = numeratorTE/denominatorTE;

  numeratorTM   = N*std::cos(incidentangle) - CosPhi;
  denominatorTM = N*std::cos(incidentangle) + CosPhi;
  rTM = numeratorTM/denominatorTM;

  // This is my calculaton for reflectivity on a metalic surface
  // depending on the fraction of TE and TM polarization
  // when TE polarization, E1_parl=0 and E1_perp=1, R=abs(rTE)^2 and
  // when TM polarization, E1_parl=1 and E1_perp=0, R=abs(rTM)^2

  Reflectivity_TE =  (rTE*conj(rTE))*(E1_perp*E1_perp)
                    / (E1_perp*E1_perp + E1_parl*E1_parl);
  Reflectivity_TM =  (rTM*conj(rTM))*(E1_parl*E1_parl)
                    / (E1_perp*E1_perp + E1_parl*E1_parl);
  Reflectivity    = Reflectivity_TE + Reflectivity_TM;

  do {
     if(CG4UniformRand(__FILE__,__LINE__)*real(Reflectivity) > real(Reflectivity_TE))iTE = -1;
     if(CG4UniformRand(__FILE__,__LINE__)*real(Reflectivity) > real(Reflectivity_TM))iTM = -1;
  } while(iTE<0&&iTM<0);

  return real(Reflectivity);
}

/**
DsG4OpBoundaryProcess::DoAbsorption
----------------------------------------

Contrast with counterpart oxrap/cu/propagate.h:propagate_at_surface

**/

void DsG4OpBoundaryProcess::DoAbsorption()
{
    //LOG(info) << "DsG4OpBoundaryProcess::DoAbsorption"
    //          << " theEfficiency " << theEfficiency
    //          ; 

    theStatus = Ds::Absorption;

#ifdef WITH_ALIGN_DEV_DEBUG
    G4double _u = CG4UniformRand("DoAbsorption", -1) ;  
    bool _detect = _u < theEfficiency ;

    if(m_dbgflat)
    { 
        m_g4->addRandomCut("Efficiency", theEfficiency ) ; 
        m_g4->addRandomNote("detect", _detect );
    }

    if( _detect ) 
#else
    if ( G4BooleanRand(theEfficiency) ) 
#endif
    {
        // EnergyDeposited =/= 0 means: photon has been detected
        theStatus = Ds::Detection;
        aParticleChange.ProposeLocalEnergyDeposit(thePhotonMomentum);
    }
    else 
    {
        aParticleChange.ProposeLocalEnergyDeposit(0.0);
    }

    NewMomentum = OldMomentum;
    NewPolarization = OldPolarization;

//  aParticleChange.ProposeEnergy(0.0);
    aParticleChange.ProposeTrackStatus(fStopAndKill);
}


