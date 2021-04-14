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

struct BConfig ; 

#include "NPY_API_EXPORT.hh"

/**
NSnapConfig
============

Principal consumer is OpTracer::snap


**/



struct NPY_API NSnapConfig 
{
    static const plog::Severity LEVEL ; 
    static const float NEGATIVE_ZERO ; 

    NSnapConfig(const char* cfg);
    BConfig* bconfig ;  
    void dump(const char* msg="NSnapConfig::dump") const ; 

    int verbosity ; 
    int steps ; 
    int fmtwidth ; 

    float x0 ;   // formerly eyestartx
    float y0 ; 
    float z0 ;
 
    float x1 ;   // formerly eyestopx
    float y1 ; 
    float z1 ; 

    std::string prefix ; 
    std::string ext ;    // eg .jpg


    const char* getSnapPath(const char* dir, const char* reldir, int index) const ;
    std::string getSnapName(int index) const ;
    static std::string SnapIndex(int index, unsigned width);
    std::string desc() const ; 



};

