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

// TEST=SArTest om-t

#include <iostream>
#include <cassert>

#include "SAr.hh"
#include "SSys.hh"



int main(int argc, char** argv)
{
    std::cout << "start" << std::endl ; 

    SAr a(argc, argv );
    std::cout << "a instanciated " << std::endl ; 
    a.dump();
    std::cout << "a dumped " << std::endl ; 

    const char* option = "--gdmlpath" ; 
    const char* fallback = NULL ; 
    const char* value = a.get_arg_after(option, fallback) ; 
    std::cout 
        << " option " << option
        << " value " << ( value ? value : "-" ) 
        << std::endl 
        ;



    const char* key = "SAR_TEST" ; 
    const char* val = "--trace --SYSRAP warning red green blue" ; 
    bool overwrite = true ; 
    SSys::setenvvar( key, val , overwrite ) ;  

    SAr b(0,0, key, ' ') ;
    std::cout << "b instanciated " << std::endl ; 
    b.dump() ; 
    std::cout << "b dumped " << std::endl ; 

    assert( b._argc == 7 ); 


    std::cout << " exepath() " << a.exepath() << std::endl ; 
    std::cout << " exename() " << a.exename() << std::endl ; 
    std::cout << " cmdline() " << a.cmdline() << std::endl ; 


    return 0 ; 
}
