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

#include <sstream>

#include "SLog.hh"
#include "BTimes.hh"

// optickscore-
#include "Opticks.hh"
#include "OpticksEvent.hh"
#include "OpticksBufferControl.hh"

// opticksgeo-
#include "OpticksHub.hh"

// optixrap-
#include "OContext.hh"
#include "OpticksEntry.hh"
#include "OConfig.hh"
#include "OEvent.hh"
#include "OBuf.hh"
#include "OPropagator.hh"
#include "ORng.hh"

// optix-
#include <optixu/optixu.h>
#include <optixu/optixu_math_stream_namespace.h>
using namespace optix ; 


// cudawrap-  NB needs to be after namespace optix
//#include "cuRANDWrapper.hh"

#include "PLOG.hh"


const plog::Severity OPropagator::LEVEL = PLOG::EnvLevel("OPropagator", "debug") ; 

void OPropagator::setOverride(unsigned int override_)
{
    m_propagateoverride = override_ ; 
}
void OPropagator::setEntry(unsigned int entry_index)
{
    m_entry_index = entry_index;
}

OPropagator::OPropagator(Opticks* ok, OEvent* oevt, OpticksEntry* entry) 
   :
    m_log(new SLog("OPropagator::OPropagator","", LEVEL)),
    m_ok(ok),
    m_oevt(oevt),
    m_ocontext(m_oevt->getOContext()),
    m_context(m_ocontext->getContext()),
    m_orng(new ORng(m_ok, m_ocontext)),
    m_propagateoverride(m_ok->getPropagateOverride()),
    m_nopropagate(false),
    m_entry(entry),
    m_entry_index(entry->getIndex()),
    m_prelaunch(false),
    m_count(0),
    m_width(0),
    m_height(0),
    m_launch_acc(m_ok->accumulateAdd("OPropagator::launch")),
    m_launch_lis(m_ok->lisAdd("OPropagator::launch"))
{
    init();
    (*m_log)("DONE");
}


std::string OPropagator::brief()
{
    std::stringstream ss ; 
    ss << m_count << " : (" << m_entry_index << ";" << m_width << "," << m_height << ") " ; 
    return ss.str();
}


void OPropagator::init()
{
    initParameters();
}


void OPropagator::initParameters()
{
    m_context[ "propagate_epsilon"]->setFloat( m_ok->getEpsilon() );       // TODO: check impact of changing propagate_epsilon
    m_context[ "utaildebug" ]->setUint( m_ok->isUTailDebug() ? 1 : 0 );    //   --utaildebug 
    m_context[ "production" ]->setUint( m_ok->isProduction() ? 1 : 0 );    //   --production 
    m_context[ "bounce_max" ]->setUint( m_ok->getBounceMax() );
    m_context[ "record_max" ]->setUint( m_ok->getRecordMax() );

    m_context[ "RNUMQUAD" ]->setUint( 2 );   // quads per record 
    m_context[ "PNUMQUAD" ]->setUint( 4 );   // quads per photon
    m_context[ "GNUMQUAD" ]->setUint( 6 );   // quads per genstep
    m_context["SPEED_OF_LIGHT"]->setFloat(299.792458f) ;   // mm/ns


    unsigned reflectcheat = m_ok->isReflectCheat() ? 1 : 0 ; 
    if(reflectcheat > 0 )
        LOG(error) <<  "OPropagator::initParameters --reflectcheat ENABLED "  ;
         

    optix::uint4 debugControl = optix::make_uint4(m_ocontext->getDebugPhoton(),0,0, reflectcheat);
    LOG(debug) << "OPropagator::init debugControl " 
              << " x " << debugControl.x 
              << " y " << debugControl.y
              << " z " << debugControl.z 
              << " w " << debugControl.w 
              ;

    m_context["debug_control"]->setUint(debugControl); 
 
    const glm::vec4& ce = m_ok->getSpaceDomain();
    const glm::vec4& td = m_ok->getTimeDomain();

    m_context["center_extent"]->setFloat( make_float4( ce.x, ce.y, ce.z, ce.w ));
    m_context["time_domain"]->setFloat(   make_float4( td.x, td.y, td.z, td.w ));
}


/**
OPropagator::setSize
-----------------------

Canonically invoked by OPropagator::prelaunch with width the number of photons
from the OpticksEvent and height 1 


**/

void OPropagator::setSize(unsigned width, unsigned height)
{
    m_width = width ; 
    m_height = height ; 
}

void OPropagator::setNoPropagate(bool nopropagate)
{
    m_nopropagate = nopropagate ; 
}


void OPropagator::prelaunch()
{
    //assert(m_prelaunch == false);
    m_prelaunch = true ; 

    bool entry = m_entry_index > -1 ; 
    if(!entry) LOG(fatal) << "MISSING entry " ;
    assert(entry);

    OpticksEvent* evt = m_oevt->getEvent(); 

    unsigned numPhotons = evt->getNumPhotons(); 
    setSize( m_propagateoverride > 0 ? m_propagateoverride : numPhotons ,  1 );
    setNoPropagate(m_ok->hasOpt("nopropagate"));

    BTimes* prelaunch_times = evt->getPrelaunchTimes() ;

    OK_PROFILE("_OPropagator::prelaunch");
    m_ocontext->launch( OContext::VALIDATE|OContext::COMPILE|OContext::PRELAUNCH,  m_entry_index ,  0, 0, prelaunch_times ); 
    OK_PROFILE("OPropagator::prelaunch");

    m_count += 1 ; 

    LOG(info) << brief()  ;
    prelaunch_times->dump("OPropagator::prelaunch");
}


/**
OPropagator::launch
----------------------

Launch times may be collected into BTimes instance held by OpticksEvent.


**/

void OPropagator::launch()
{
    if(m_prelaunch == false) prelaunch();

    if(m_nopropagate)
    {
        LOG(warning) << "OPropagator::launch SKIP due to --nopropagate " ; 
        return ; 
    }

    OpticksEvent* evt = m_oevt->getEvent(); 
    BTimes* launch_times = evt->getLaunchTimes() ;

    const char* llogpath = m_ocontext->getPrintIndexLogPath() ; 

    LOG(info) << "LAUNCH NOW " << ( llogpath ? llogpath : "-" ) ; 

    OK_PROFILE("_OPropagator::launch");
    double dt = m_ocontext->launch( OContext::LAUNCH,  m_entry_index,  m_width, m_height, launch_times);
    OK_PROFILE("OPropagator::launch");

    m_ok->accumulateSet(m_launch_acc, dt ); 
    m_ok->lisAppend(m_launch_lis, dt ); 

    LOG(info) << "LAUNCH DONE" ; 

    LOG(info) << brief() ;
    launch_times->dump("OPropagator::launch");
}

