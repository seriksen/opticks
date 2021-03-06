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

#ifdef __clang__

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wshadow"

#elif defined(__GNUC__) || defined(__GNUG__)

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wshadow"

#elif defined(_MSC_VER)

#pragma warning(push)
// nonstandard extension used: nameless struct/union  (from glm )
#pragma warning( disable : 4201 )
// members needs to have dll-interface to be used by clients
#pragma warning( disable : 4251 )

#endif


#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>  
#include <glm/gtx/quaternion.hpp>  
#include <glm/gtc/matrix_transform.hpp>  
#include <glm/gtc/type_ptr.hpp>

#include <glm/gtx/transform.hpp>

#ifndef __CUDACC__
#include <glm/gtx/string_cast.hpp>
#endif


inline float gmaxf(const float a, const float b)
{
   return a > b ? a : b ; 
}
inline float gminf(const float a, const float b)
{
   return a > b ? b : a ; 
}

inline float gmaxf( const glm::vec3& v )
{
    return gmaxf( gmaxf(v.x, v.y), v.z );
}


inline void gmaxf(glm::vec4& r, const glm::vec4& a, const glm::vec4& b )
{
    r.x = gmaxf( a.x, b.x );
    r.y = gmaxf( a.y, b.y );
    r.z = gmaxf( a.z, b.z );
    r.w = gmaxf( a.w, b.w );
}
inline void gminf(glm::vec4& r, const glm::vec4& a, const glm::vec4& b )
{
    r.x = gminf( a.x, b.x );
    r.y = gminf( a.y, b.y );
    r.z = gminf( a.z, b.z );
    r.w = gminf( a.w, b.w );
}


inline glm::vec4 gminf(const glm::vec4& a, const glm::vec4& b )
{
    glm::vec4 r ; 
    gminf(r, a, b );
    return r ;
}
inline glm::vec4 gmaxf(const glm::vec4& a, const glm::vec4& b )
{
    glm::vec4 r ; 
    gmaxf(r, a, b );
    return r ;
}

//  development here is not healthy, as causes recompilation 
//  of all NGLM users, which is most everything downstream
//  ... so put new developments in NGLMExt.hpp/NGLMExt.cpp 



#ifdef __clang__

#pragma clang diagnostic pop

#elif defined(__GNUC__) || defined(__GNUG__)

#pragma GCC diagnostic pop

#elif defined(_MSC_VER)

#pragma warning(pop)

#endif



