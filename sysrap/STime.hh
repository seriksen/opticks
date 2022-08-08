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
#include "SYSRAP_API_EXPORT.hh"

struct SYSRAP_API STime 
{
    static const char* FMT ;   // 
    static int EpochSeconds(); 
    static std::string Format(int epochseconds=0, const char* fmt=nullptr ); 
    static std::string Stamp(); 
    static std::string mtime(const char* base, const char* name); 
    static std::string mtime(const char* path); 

};


