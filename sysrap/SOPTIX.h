#pragma once


#include <optix.h>
#include <optix_function_table_definition.h>
#include <optix_stubs.h>

#define _xstr(s) _str(s)
#define _str(s) #s

#include "CUDA_CHECK.h"
#include "OPTIX_CHECK.h"


struct SOPTIX
{  
    static void LogCB( unsigned int level, const char* tag, const char* message, void* /*cbdata */) ;     

    const char* VERSION ; 
    OptixDeviceContext context ; 

    std::string desc() const ;
    SOPTIX(); 
    void init(); 
};

inline void SOPTIX::LogCB( unsigned int level, const char* tag, const char* message, void* /*cbdata */)  // static 
{   
    std::stringstream ss ; 
    ss << "[" << std::setw(2) << level << "][" << std::setw( 12 ) << tag << "]: " << message ;
    std::string line = ss.str() ;
    std::cout << line ;
}

inline std::string SOPTIX::desc() const
{
    std::stringstream ss ; 
    ss << "SOPTIX::desc OPTIX_VERSION " << OPTIX_VERSION ;  
    std::string str = ss.str(); 
    return str ; 
}

inline SOPTIX::SOPTIX()
    :
    context(nullptr),
    VERSION(_xstr(OPTIX_VERSION))
{
    init();
}

/**
SOPTIX::init
------------

Initialize CUDA and create OptiX context

**/

inline void SOPTIX::init()
{
    CUDA_CHECK(cudaFree( 0 ) );

    CUcontext cuCtx = 0;  // zero means take the current context
    OPTIX_CHECK( optixInit() );
    OptixDeviceContextOptions options = {};
    options.logCallbackFunction       = &SOPTIX::LogCB ;
    options.logCallbackLevel          = 4;
    //options.validationMode = OPTIX_DEVICE_CONTEXT_VALIDATION_MODE_ALL ; 

    OPTIX_CHECK( optixDeviceContextCreate( cuCtx, &options, &context ) );
}

