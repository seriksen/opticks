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

#pragma once

#include <string>

#include "plog/Severity.h"
#include "NGLM.hpp"

template<typename T> class NPY ; 

#include "NPY_API_EXPORT.hh"
#include "NPY_HEAD.hh"


/*

GenstepNPY
============

Base class of FabStepNPY and TorchStepNPY 


   *setZenithAzimuth*

   Photons directions are generated using two random numbers in range 0:1 
   which are used scale the zenith and azimuth ranges.
   Default is a uniform sphere. Changing zenith ranges allows cones or
   rings to be generated and changing azimuth range allows 
   to chop the cone, ring or sphere.

                       mapped to 0:2pi of azimuth angle    
                    -------
           (0.f,1.f,0.f,1.f)
            --------
              mapped to 0:pi of zenith angle
*/


class NPY_API GenstepNPY {
   public:  
       static const plog::Severity LEVEL ; 
   public:  
       GenstepNPY(unsigned genstep_type, unsigned num_step=1, const char* config=NULL, bool is_default=false); 
       void addStep(bool verbose=false); // increments m_step_index
       unsigned getNumStep();

   public:
       // slots used by Geant4 only (not Opticks) from cfg4- 
       unsigned int getNumG4Event();
       unsigned int getNumPhotonsPerG4Event(); 
       void setNumPhotonsPerG4Event(unsigned int n);
   public:


       NPY<float>* getNPY();
       void         addActionControl(unsigned long long  action_control);

       virtual void update() = 0 ;   // <-- provided by subclasses such as TorchstepNPY

       virtual void dump(const char* msg="GenstepNPY::dump");
       void dumpBase(const char* msg="GenstepNPY::dumpBase");
   public:  
       void setNumPhotons(const char* s );
       void setNumPhotons(unsigned int num_photons );
       void setMaterialLine(unsigned int ml);
       unsigned int getNumPhotons();
       unsigned int getMaterialLine();
   private:
       void setGenstepType(unsigned genstep_type);  
       // invoked by addStep using the ctor argument type 
       // genstep types identify what to generate eg: TORCH, CERENKOV, SCINTILLATION
       // currently limited to all gensteps within a GenstepNPY instance having same type

   public:  
       // target setting needs external info regarding geometry 
       void setFrame(const char* s );
       void setFrame(unsigned vindex );
       glm::ivec4&  getFrame();
       int getFrameIndex();
       void setFrameTransform(glm::mat4& transform);
       // targetting needs frame transform info which is done by GGeo::targetTorchStep(torchstep)

       void setFrameTransform(const char* s );       // directly from string of 16 comma delimited floats 
       void setFrameTargetted(bool targetted=true);
       bool isFrameTargetted();
       bool isDummyFrame();
       const glm::mat4& getFrameTransform();

       std::string brief(); 
   public:  
        // methods invoked by update after frame transform is available
       void setPosition(const glm::vec4& pos );
       void setDirection(const glm::vec3& dir );
       void setPolarization(const glm::vec4& pol );

       void setDirection(const char* s );
       void setZenithAzimuth(const char* s );
       void setWavelength(const char* s );
       void setWeight(const char* s );
       void setTime(const char* s );
       void setRadius(const char* s );
       void setDistance(const char* s );

       void setRadius(float radius );
       void setDistance(float distance);
       void setBaseMode(unsigned umode);
       void setBaseType(unsigned utype);
   public:  
       glm::vec3 getPosition();
       glm::vec3 getDirection();
       glm::vec3 getPolarization();
       glm::vec4 getZenithAzimuth();

       float getTime();
       float getRadius();
       float getWavelength();


       unsigned getBaseMode();
       unsigned getBaseType();
   public:  
       // need external help to set the MaterialLine
       void setMaterial(const char* s );
       const char* getConfig();
       const char* getMaterial();
       bool isDefault() const ; 
  private:
       unsigned int m_genstep_type ; 
       unsigned int m_num_step ; 
       const char*  m_config ;
       bool         m_is_default ; 
  private:
       const char*  m_material ;
       NPY<float>*  m_npy ; 
       unsigned int m_step_index ; 
  private:
       // 6 transport quads that are copied into the genstep buffer by addStep
       glm::ivec4   m_ctrl ;
       glm::vec4    m_post ;
       glm::vec4    m_dirw ;
       glm::vec4    m_polw ;
       glm::vec4    m_zeaz ;
       glm::vec4    m_beam ; 
  private:
       glm::ivec4   m_frame ;
       glm::mat4    m_frame_transform ; 
       bool         m_frame_targetted ; 
  private:
       unsigned int m_num_photons_per_g4event ;
 

};

#include "NPY_TAIL.hh"




