/**
Tests individual trees::

    NCSGLoadTest $TMP/tboolean-csg-two-box-minus-sphere-interlocked-py-/1

**/

#include <iostream>

#include "BStr.hh"

#include "NPY.hpp"
#include "NCSG.hpp"
#include "NNode.hpp"
#include "NGLMExt.hpp"
#include "GLMFormat.hpp"

#include "NPY_LOG.hh"
#include "PLOG.hh"


 
void test_LoadTree(const char* treedir)
{
    int verbosity = 2 ; 
    NCSG* csg = NCSG::LoadTree(treedir, verbosity );
    assert(csg);
}



/*
    char* scan_ = getenv("SCAN") ;
    std::string scan = scan_ ? scan_ : "10,10,100,0" ;  
    std::vector<float> f ; 
    BStr::fsplit(f, scan_ ? scan_ : "0,0,127,0,0,1,-2,2,0.001", ',');

    if(f.size() != 9)
       LOG(fatal) << "NCSGLoadTest::NCSGLoadTest"
                  << " scan string needs 9 elements " << scan_
                   ;

    assert(f.size() == 9 );


    nnode* root = csg.getRoot();

    glm::vec3 origin(    f[0],f[1],f[2] );
    glm::vec3 direction( f[3],f[4],f[5] );
    glm::vec3 range(     f[6],f[7],f[8] );

    nnode::Scan(*root, origin, direction, range );

*/


int main(int argc, char** argv)
{
    PLOG_(argc, argv);
    NPY_LOG__ ;  

    LOG(info) << " argc " << argc << " argv[0] " << argv[0] ;  

    test_LoadTree( argc > 1 ? argv[1] : "$TMP/csg_py/1" );

    return 0 ; 
}


