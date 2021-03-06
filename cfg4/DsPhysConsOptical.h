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

class Opticks ; 

class DsPhysConsOptical 
{
public:
    DsPhysConsOptical(Opticks* ok);
    void ConstructProcess();
    void dump(const char* msg="DsPhysConsOptical::dump");
private:
    Opticks* m_ok ; 

    bool m_doReemission;              /// ScintDoReemission: Do reemission in scintilator
    bool m_doScintAndCeren;           /// ScintDoScintAndCeren: Do both scintillation and Cerenkov in scintilator
    bool m_useFastMu300nsTrick; 
    bool m_useCerenkov ;
    bool m_useScintillation ;
    bool m_useRayleigh ;
    bool m_useAbsorption;
    bool m_applyWaterQe;              /// wangzhe: Apply QE for water cerenkov process when OP is created?  //     See DsG4Cerenkov and Doc 3925 for details
    double m_cerenPhotonScaleWeight;  /// Number (>= 1.0) used to scale down the mean number of optical photons produced.  
                                      /// For each actual secondary optical photon track produced, it will be given a weight equal to this scale
                                      /// for scaling up if detected later.  Default is 1.0.

    int m_cerenMaxPhotonPerStep;      /// Maximum number of photons per step to limit step size.  This value is independent from PhotonScaleWeight.  Default is 300.

    double m_scintPhotonScaleWeight;    /// Scale down number of produced scintillation photons by this much
    double m_ScintillationYieldFactor;  /// scale the number of produced scintillation photons per MeV by this much.
                                        /// This controls the primary yield of scintillation photons per MeV of deposited energy.
    double m_birksConstant1;           /// Birks constants C1 and C2
    double m_birksConstant2;
    double m_gammaSlowerTime;
    double m_gammaSlowerRatio;
    double m_neutronSlowerTime;
    double m_neutronSlowerRatio;
    double m_alphaSlowerTime;
    double m_alphaSlowerRatio;

};

