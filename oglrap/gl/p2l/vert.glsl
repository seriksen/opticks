#version 410 core
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


// p2l passthrough to geometry shader

uniform mat4 ModelViewProjection ;
uniform mat4 ModelView ;

layout(location = 0) in vec4 vpos ;
layout(location = 1) in vec4 vdir ;

out vec3 colour;
out vec3 direction ; 


void main () 
{
    colour = vec3(1.0,0.0,0.0) ;
    direction = vdir.xyz ;
    gl_Position = vec4( vpos.xyz, 1.0);
}


