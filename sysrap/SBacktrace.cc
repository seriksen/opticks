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


/*

https://oroboro.com/stack-trace-on-crash/
https://panthema.net/2008/0901-stacktrace-demangled/cxa_demangle.html

https://eli.thegreenplace.net/2015/programmatic-access-to-the-call-stack-in-c/

   suggests libunwind


*/


#include <cstdio>
#include <cstdlib>
#include <string.h>
#include <ostream>
#include <iostream>

#include "SFrame.hh"
#include "SBacktrace.hh"

#include <execinfo.h>
#include <errno.h>

void SBacktrace::Dump() 
{ 
    std::ostream& out = std::cout ;
    Dump(out);  
}

void SBacktrace::Dump(std::ostream& out) 
{
   unsigned max_frames = 63 ; 
   void* addrlist[max_frames+1];
   unsigned addrlen = backtrace( addrlist, sizeof(addrlist)/sizeof(void*));

   out << "SBacktrace::Dump"
       << " addrlen " << addrlen 
       << std::endl 
       ;

   if(addrlen == 0) return;
 
   char** symbollist = backtrace_symbols( addrlist, addrlen );

   bool raw = false ; 
   if(raw) for ( unsigned i = 0 ; i < addrlen; i++ )
       out << symbollist[i] 
           << " : "
           << addrlist[i]
           << std::endl 
           ; 

   out << "SFrames..\n" ; 

   for ( unsigned i = 0 ; i < addrlen; i++ )
   {
       SFrame f(symbollist[i]) ; 
       f.dump(out); 
   }

   free(symbollist);
}


/**
SBacktrace::CallSite
---------------------

For a call stack like the below with call "::flat()" return the line starting with 3::

    SFrames..
    0   libSysRap.dylib                     0x000000010b991662 SBacktrace::Dump()                                                                                   + 114      
    1   libCFG4.dylib                       0x000000010026909d CMixMaxRng::flat()                                                                                   + 445      
    2   libCFG4.dylib                       0x0000000100269169 non-virtual thunk to CMixMaxRng::flat()                                                              + 25       
    3   libG4processes.dylib                0x0000000102ba3ee5 G4VEmProcess::PostStepGetPhysicalInteractionLength(G4Track const&, double, G4ForceCondition*)        + 661      
    4   libG4tracking.dylib                 0x00000001023e8ff0 G4VProcess::PostStepGPIL(G4Track const&, double, G4ForceCondition*)                                  + 80       
    5   libG4tracking.dylib                 0x00000001023e8a1a G4SteppingManager::DefinePhysicalStepLength()                                                        + 298      
    6   libG4tracking.dylib                 0x00000001023e5c3a G4SteppingManager::Stepping()                                                                        + 394      


**/

const char* SBacktrace::CallSite(const char* call, bool addr )
{
   const char* site = NULL ;  
   unsigned max_frames = 63 ; 
   void* addrlist[max_frames+1];
   unsigned addrlen = backtrace( addrlist, sizeof(addrlist)/sizeof(void*));
   if(addrlen == 0) return site ;
 
   char** symbollist = backtrace_symbols( addrlist, addrlen );
   int state = -1 ; 
   for ( unsigned i = 0 ; i < addrlen; i++ )
   {
       SFrame f(symbollist[i]) ; 
       //f.dump(); 

       char* p = f.func ? strstr( f.func, call ) : NULL ; 
       if(p) state++ ; 
 
       if(!p && state > -1 )    // pick first line without call string following a line with it 
       {
           char out[256]; 
           if(addr)
           {
               snprintf( out, 256, "%16p %10s %s", addrlist[i], f.offset, f.func ) ;  
               // addresses different for each executable so not good for comparisons, but real handy
               // for looking up source line in debugger with : 
               //      (lldb) source list  -a 0x000...
           }
           else
           {
               snprintf( out, 256, " %10s %s", f.offset, f.func ) ;
           }
           site = strdup(out) ;
           break ; 
       }    
   }

   free(symbollist);
   return site ; 
}
  


