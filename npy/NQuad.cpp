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

#include <cstring>
#include <sstream>
#include "NQuad.hpp"



void nuvec4::dump(const char* msg) const 
{
    printf("%20s : %10u %10u %10u %10u \n",msg, x,y,z,w ); 
}
void nivec4::dump(const char* msg) const 
{
    printf("%20s : %10d %10d %10d %10d \n",msg, x,y,z,w ); 
}

void nvec3::dump(const char* msg) const 
{
    printf("%20s : %10.4f %10.4f %10.4f  \n",msg, x,y,z ); 
}

glm::vec3 nvec3::as_vec3() const 
{
    return glm::vec3(x,y,z);
}

bool nvec3::is_zero(float eps)
{
    return fabs(x) < eps && fabs(y) < eps && fabs(z) < eps ; 
}

nvec3 nvec3::from_vec4( const nvec4& v ) // static 
{
    return nvec3( v.x, v.y, v.z ); 
}




const char* nvec3::desc() const
{
    char _desc[64];
    snprintf(_desc, 64, " (%7.2f %7.2f %7.2f) ", x,y,z );
    return strdup(_desc);
}

const char* nvec3::descg() const
{
    char _desc[64];
    snprintf(_desc, 64, " (%7.2g %7.2g %7.2g) ", x,y,z );
    return strdup(_desc);
}


const char* nivec3::desc() const 
{
    char _desc[64];
    snprintf(_desc, 64, " (%3d %3d %3d) ", x,y,z );
    return strdup(_desc);
}
const char* nivec4::desc() const 
{
    char _desc[64];
    snprintf(_desc, 64, " (%4d %4d %4d %4d) ", x,y,z,w );
    return strdup(_desc);
}

const char* nuvec3::desc() const 
{
    char _desc[64];
    snprintf(_desc, 64, " (%5u %5u %5u) ", x,y,z );
    return strdup(_desc);
}


template <typename T>
const char* ntvec3<T>::desc() const
{
    std::stringstream ss ; 
    ss << " (" << x << " " << y << " " << z << ") " ;    
    std::string desc = ss.str();
    return strdup(desc.c_str());
}

template <typename T>
const char* ntvec4<T>::desc() const
{
    std::stringstream ss ; 
    ss << " (" << x << " " << y << " " << z << " " <<  w << ") " ;    
    std::string desc = ss.str();
    return strdup(desc.c_str());
}





const char* nvec4::desc() const 
{
    char _desc[64];
    snprintf(_desc, 64, " (%7.2f %7.2f %7.2f %7.2f) ", x,y,z,w );
    return strdup(_desc);
}


void nvec4::dump(const char* msg) const 
{
    printf("%20s : %10.4f %10.4f %10.4f %10.4f \n",msg, x,y,z,w ); 
}
void nquad::dump(const char* msg) const 
{
    printf("%s\n", msg);
    f.dump("f");
    u.dump("u");
    i.dump("i");
}



template struct NPY_API ntvec3<float>;
template struct NPY_API ntvec3<double>;
template struct NPY_API ntvec3<short>;
template struct NPY_API ntvec3<int>;
template struct NPY_API ntvec3<unsigned int>;
template struct NPY_API ntvec3<unsigned long>;
template struct NPY_API ntvec3<unsigned long long>;


template struct NPY_API ntvec4<float>;
template struct NPY_API ntvec4<double>;
template struct NPY_API ntvec4<short>;
template struct NPY_API ntvec4<int>;
template struct NPY_API ntvec4<unsigned int>;
template struct NPY_API ntvec4<unsigned long>;
template struct NPY_API ntvec4<unsigned long long>;




