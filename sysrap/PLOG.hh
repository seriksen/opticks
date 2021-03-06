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

// NB DANGEROUS DEFINES : INCLUDE AS THE LAST HEADER
//    OTHERWISE RANDOMLY REPLACES STRINGS IN SYSTEM HEADERS

#include <cstddef>
#include <plog/Log.h>

// translate from boost log levels to plog 
using plog::fatal ;
using plog::error ;
using plog::warning ;
using plog::info ;
using plog::debug ;
using plog::verbose ;

// hmm dangerous but what alternative 
//#define trace plog::verbose 

#include "SYSRAP_API_EXPORT.hh"

/**

PLOG : Logging Infrastructure
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Opticks executables follow the following pattern.

.. code-block:: c


   #include "SYSRAP_LOG.hh"  // headers for each projects logger
   #include "BRAP_LOG.hh"
   #include "NPY_LOG.hh"
   #include "OKCORE_LOG.hh"

   #include "PLOG.hh"        // infrastructure header

   int main(int argc, char** argv)
   {
       PLOG_(argc, argv);  // pass arguments to PLOG_ macro

       SYSRAP_LOG__ ;     // setup loggers for all projects you want to see output from
       BRAP_LOG__ ; 
       NPY_LOG__ ;       
       OKCORE_LOG__ ;       

       Opticks ok(argc, argv);

       //  ...  exercise Opticks  ... 

       return 0 ; 
   }


*PLOG* parses command line arguments and configures the 
logging level of each project, for example:

.. code-block:: sh

   OpticksResourceTest --sysrap trace --npy info   # lower cased tags identify the projects
   GGeoViewTest --npy debug    

Available logging levels are:

* *trace* :  most verbose level, providing a great deal of output  
* *debug*
* *info* : normal default logging level 
* *warning*
* *error*
* *fatal*  


The tags for each project are listed below.

=======================  ============================
 Project Folder           Tag 
=======================  ============================
              sysrap                         SYSRAP 
            boostrap                           BRAP 
          opticksnpy                            NPY 
         optickscore                         OKCORE 
                ggeo                           GGEO 
           assimprap                         ASIRAP 
         openmeshrap                        MESHRAP 
          opticksgeo                          OKGEO 
              oglrap                         OGLRAP 
             cudarap                        CUDARAP 
           thrustrap                          THRAP 
            optixrap                          OXRAP 
           opticksop                           OKOP 
           opticksgl                           OKGL 
                  ok                             OK 
                cfg4                           CFG4 
=======================  ============================


The somewhat bizarre usage implementation based on preprocessor
macros allows the static logger symbols to be planted within 
each of the shared objects in a manner that works on Mac, Linux 
and Windows.


**/

struct PLOG ; 



/**
Delta
------
::

    enum Severity
    {   
        none = 0,
        fatal = 1,
        error = 2,
        warning = 3,
        info = 4,
        debug = 5,
        verbose = 6 
    };  

**/




#include "SAr.hh"

struct SYSRAP_API PLOG 
{
    static const int MAXARGC ; 

    SAr         args ; 
    int         level ; 
    const char* filename ; 
    int         maxFileSize ;    // bytes
    int         maxFiles ; 

    static plog::Severity Delta(plog::Severity level_, int delta); 
    static plog::Severity EnvLevel( const char* key, const char* fallback); 

    PLOG(int argc, char** argv, const char* fallback="VERBOSE", const char* prefix=NULL );

    const char* name(); 
    const char* exename() const ;
    const char* cmdline() const ;
    const char* get_arg_after(const char* option, const char* fallback) const ;

    int parse( const char* fallback);
    int parse( plog::Severity _fallback);

    int prefixlevel_parse( int fallback, const char* prefix);
    int prefixlevel_parse( const char* fallback, const char* prefix);
    int prefixlevel_parse( plog::Severity _fallback, const char* prefix);

    static int  _parse(int argc, char** argv, const char* fallback);
    static int  _prefixlevel_parse(int argc, char** argv, const char* fallback, const char* prefix);
    static void _dump(const char* msg, int argc, char** argv);
    static const char* _name(plog::Severity severity);
    static const char* _name(int level);
    static const char* _logpath_parse_problematic(int argc, char** argv);
    static const char* _logpath();


    static PLOG* instance ; 
};

#define pLOG(severity, delta)     LOG_(PLOG_DEFAULT_INSTANCE, PLOG::Delta(severity,delta))

#include "PLOG_INIT.hh"


