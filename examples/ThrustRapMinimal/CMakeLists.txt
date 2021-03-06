cmake_minimum_required(VERSION 3.5 FATAL_ERROR)
set(name ThrustRapMinimal)
project(${name} VERSION 0.1.0)
include(OpticksBuildOptions)

find_package(CUDARap  REQUIRED CONFIG)
 
set(SOURCES
    expandTest.cu
)

CUDA_ADD_EXECUTABLE( ${name} ${SOURCES} OPTIONS )

get_target_property(_cal_ill ${name} INTERFACE_LINK_LIBRARIES)
message(STATUS "${name} CUDA_ADD_LIBRARY.INTERFACE_LINK_LIBRARIES : ${_cal_ill} ") 
set_target_properties(${name}  PROPERTIES INTERFACE_LINK_LIBRARIES "" ) # kludge clear CAUSES GRIEF FOR INTEGRATED BUILD
target_link_libraries( ${name} Opticks::CUDARap)
target_include_directories( ${name} PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}> )

echo_target_std(${name})

install(TARGETS ${name} DESTINATION lib)


#[=[
Scrubbing INTERFACE_LINK_LIBRARIES causes all thrap tests to fail with the below exception in integrated build, but not for the 
proj by proj build, see thrap- for laborious debug that discovered this::

  libc++abi.dylib: terminating with uncaught exception of type thrust::system::system_error: device free failed: CUDA driver version is insufficient for CUDA runtime version

Somehow the integrated build uses::

   /Developer/NVIDIA/CUDA-9.1/lib/libcudart_static.a;-Wl,-rpath,/Developer/NVIDIA/CUDA-9.1/lib

When it should be using::

   /Developer/NVIDIA/CUDA-9.1/lib/libcudart_static.a;-Wl,-rpath,/usr/local/cuda/lib 

-- ThrustRap CUDA_ADD_LIBRARY.INTERFACE_LINK_LIBRARIES : /Developer/NVIDIA/CUDA-9.1/lib/libcudart_static.a;-Wl,-rpath,/usr/local/cuda/lib 

#]=]

