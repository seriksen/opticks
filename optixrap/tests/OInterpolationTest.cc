#include "SSys.hh"
#include "BFile.hh"
#include "NPY.hpp"

#include "GBndLib.hh"
#include "GMaterialLib.hh"
#include "GSurfaceLib.hh"

#include "OBndLib.hh"
#include "OLaunchTest.hh"
#include "OContext.hh"
#include "Opticks.hh"
#include "OpticksHub.hh"

#include "OScene.hh"


#include "SYSRAP_LOG.hh"
#include "OKCORE_LOG.hh"
#include "GGEO_LOG.hh"
#include "OXRAP_LOG.hh"

#include "PLOG.hh"

/**
OInterpolationTest
===================

See also OInterpolationIdentityTest 

**/



void interpolationTest( Opticks* ok, OContext* ocontext, OBndLib* obnd, optix::Context& context)
{
    unsigned nb = obnd->getNumBnd(); 
    unsigned nx = 820 - 60 + 1 ;     // 761 : 1 nm steps 
    unsigned ny = obnd->getHeight(); // total number of float4 props

    LOG(info) 
             << " interpolation_buffer "
             << " nb " << nb 
             << " nx " << nx 
             << " ny " << ny
             ; 

    optix::Buffer interpolationBuffer = context->createBuffer(RT_BUFFER_OUTPUT, RT_FORMAT_FLOAT4, nx, ny);
    context["out_buffer"]->setBuffer(interpolationBuffer);   

    OLaunchTest ott(ocontext, ok, "OInterpolationTest.cu.ptx", "OInterpolationTest", "exception");
    ott.setWidth(  nx );   
    ott.setHeight( ny/8 );   // kernel loops over eight for matsur and num_float4
    ott.launch();

    NPY<float>* out = NPY<float>::make(nx, ny, 4);
    out->read( interpolationBuffer->map() );
    interpolationBuffer->unmap(); 
    out->save("$TMP/InterpolationTest/OInterpolationTest_interpol.npy");
}



int main(int argc, char** argv)
{
    PLOG_(argc, argv);    

    SYSRAP_LOG__ ; 
    OKCORE_LOG__ ; 
    GGEO_LOG__ ; 
    OXRAP_LOG__ ; 

    Opticks ok(argc, argv);
    OpticksHub hub(&ok);
    OScene sc(&hub);

    LOG(info) << " ok " ; 

    OContext* ocontext = sc.getOContext();
    optix::Context context = ocontext->getContext();

    OBndLib* obnd = sc.getOBndLib();

    interpolationTest( &ok, ocontext, obnd, context) ;

    std::string path = BFile::FormPath("$OPTICKS_HOME/optixrap/tests/OInterpolationTest_interpol.py");
    return SSys::exec("python",path.c_str());
}

