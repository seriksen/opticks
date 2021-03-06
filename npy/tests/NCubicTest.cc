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


#include <cstdlib>
#include "NGLMExt.hpp"

#include "NCubic.hpp"
#include "NPart.hpp"
#include "NBBox.hpp"

#include "OPTICKS_LOG.hh"

void test_part()
{
    LOG(info) << "test_part" ; 
    ncubic* n = make_cubic(1,1,1,1,-1,1);
    npart p = n->part();
    p.dump("cubic");
}

void test_bbox()
{
    LOG(info) << "test_bbox" ; 
    ncubic* n = make_cubic(1,1,1,1,-1,1);
    nbbox bb = n->bbox();
    bb.dump("hyp");
}

int main(int argc, char** argv)
{
    OPTICKS_LOG(argc, argv);

    test_part();
    test_bbox();

    return 0 ; 
}




