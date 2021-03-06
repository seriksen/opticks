
set(OpenMesh_MODULE "${CMAKE_CURRENT_LIST_FILE}")

#if(NOT OPTICKS_PREFIX)
#    # this works when this module is included from installed tree
#    get_filename_component(OpenMesh_MODULE_DIR ${CMAKE_CURRENT_LIST_FILE} DIRECTORY)
#    get_filename_component(OpenMesh_MODULE_DIRDIR ${OpenMesh_MODULE_DIR} DIRECTORY)
#    get_filename_component(OpenMesh_MODULE_DIRDIRDIR ${OpenMesh_MODULE_DIRDIR} DIRECTORY)
#    set(OPTICKS_PREFIX ${OpenMesh_MODULE_DIRDIRDIR})
#endif()

set(OpenMesh_PREFIX "${OPTICKS_PREFIX}/externals")

find_path( OpenMesh_INCLUDE_DIR
           NAMES "OpenMesh/Core/Mesh/TriMesh_ArrayKernelT.hh"
           PATHS "${OpenMesh_PREFIX}/include"
)

find_library( OpenMeshCore_LIBRARY 
              NAMES OpenMeshCore${OpenMesh_SUFFIX}
              PATHS ${OpenMesh_PREFIX}/lib )

find_library( OpenMeshTools_LIBRARY 
              NAMES OpenMeshTools${OpenMesh_SUFFIX}
              PATHS ${OpenMesh_PREFIX}/lib )

if(OpenMesh_INCLUDE_DIR AND OpenMeshCore_LIBRARY AND OpenMeshTools_LIBRARY)
  set(OpenMesh_FOUND "YES")
else()
  set(OpenMesh_FOUND "NO")
endif()

if(OpenMesh_VERBOSE)
   message(STATUS "FindOpenMesh.cmake OpenMesh_MODULE     :${OpenMesh_MODULE}  ")
   message(STATUS "FindOpenMesh.cmake OpenMesh_INCLUDE_DIR:${OpenMesh_INCLUDE_DIR}  ")
   message(STATUS "FindOpenMesh.cmake OpenMeshCore_LIBRARY:${OpenMeshCore_LIBRARY}  ")
   message(STATUS "FindOpenMesh.cmake OpenMeshTools_LIBRARY:${OpenMeshTools_LIBRARY}  ")
   message(STATUS "FindOpenMesh.cmake OpenMesh_FOUND:${OpenMesh_FOUND}  ")
endif()

if(OpenMesh_FOUND AND NOT TARGET Opticks::OpenMesh)

    add_library(Opticks::OpenMeshCore UNKNOWN IMPORTED) 
    set_target_properties(Opticks::OpenMeshCore PROPERTIES
        IMPORTED_LOCATION "${OpenMeshCore_LIBRARY}"
        INTERFACE_IMPORTED_LOCATION "${OpenMeshCore_LIBRARY}"
    )

    add_library(Opticks::OpenMeshTools  UNKNOWN IMPORTED) 
    set_target_properties(Opticks::OpenMeshTools PROPERTIES
        IMPORTED_LOCATION "${OpenMeshTools_LIBRARY}"
        INTERFACE_IMPORTED_LOCATION "${OpenMeshTools_LIBRARY}"
    )

    add_library(Opticks::OpenMesh INTERFACE IMPORTED)
    set_target_properties(Opticks::OpenMesh PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES "${OpenMesh_INCLUDE_DIR}"
        INTERFACE_LINK_LIBRARIES "Opticks::OpenMeshCore;Opticks::OpenMeshTools"
    )

    # https://cmake.org/cmake/help/v3.3/prop_tgt/INTERFACE_COMPILE_DEFINITIONS.html

    set(OpenMesh_targets  OpenMeshCore OpenMeshTools OpenMesh)


endif()


