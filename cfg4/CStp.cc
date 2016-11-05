#include <sstream>

#include "G4Step.hh"
#include "G4StepPoint.hh"
#include "CStp.hh"
#include "CRecorder.hh"
#include "OpticksFlags.hh"
#include "OpStatus.hh"

#include "Format.hh"


#ifdef USE_CUSTOM_BOUNDARY
CStp::CStp(const G4Step* step, int step_id, DsG4OpBoundaryProcessStatus boundary_status, unsigned premat, unsigned postmat, unsigned preflag, unsigned postflag, CStage::CStage_t stage, int action, const G4ThreeVector& origin) 
#else
CStp::CStp(const G4Step* step, int step_id,   G4OpBoundaryProcessStatus boundary_status, unsigned premat, unsigned postmat, unsigned preflag, unsigned postflag, CStage::CStage_t stage, int action, const G4ThreeVector& origin) 
#endif
   :
   m_step(new G4Step(*step)), 
   m_step_id(step_id),
   m_boundary_status(boundary_status),
   m_premat(premat),
   m_postmat(postmat),
   m_preflag(preflag),
   m_postflag(postflag),
   m_stage(stage),
   m_action(action),
   m_origin(origin)
{
}


std::string CStp::origin()
{
    std::stringstream ss ; 
    ss 
       << ::Format(m_origin, "Ori", 4 ) 
       ;

    return ss.str(); 
}


std::string CStp::description()
{
    std::stringstream ss ; 

    ss 
       << " " << OpticksFlags::Abbrev(m_preflag) << "/" << OpticksFlags::Abbrev(m_postflag) 
       << "   " << std::setw(5) << OpBoundaryAbbrevString(m_boundary_status) 
       << "   " << std::setw(50) << CRecorder::Action(m_action) 
       << std::endl 
       << "[" 
       << std::setw(4) << m_step_id 
       << "]"       
       << ::Format(m_step, m_origin, "Stp" ) 
       ;

    return ss.str(); 
}

