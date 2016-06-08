#include "OpticksEngine.hh"
#include "Opticks.hh"
#include "OpticksCfg.hh"

#include "BLog.hh"

void OpticksEngine::init()
{
    m_cfg = m_opticks->getCfg();  
  
    LOG(info) << "OpticksEngine::init" ; 

    assert(m_cfg);
}




