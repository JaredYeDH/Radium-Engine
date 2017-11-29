set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fexceptions")
# here is defined the way we want to import assimp
ExternalProject_Add(
        openmesh
        # where the source will live
        SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/3rdPartyLibraries/OpenMesh"

        # override default behaviours
        UPDATE_COMMAND ""

        # set the installatin to installed/openmesh
        INSTALL_DIR "${RADIUM_SUBMODULES_INSTALL_DIRECTORY}"
        CMAKE_ARGS
            -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
            -DBUILD_APPS=OFF
            -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
            -DOPENMESH_BUILD_SHARED=TRUE
            -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
            -DCMAKE_CXX_FLAGS=${CMAKE_CXX_FLAGS}
            -DCMAKE_BUILD_TYPE=${RADIUM_SUBMODULES_BUILD_TYPE}
)

add_custom_target(openmesh_lib
    DEPENDS openmesh
    )

# ----------------------------------------------------------------------------------------------------------------------
set( OPENMESH_INCLUDE_DIR ${RADIUM_SUBMODULES_INSTALL_DIRECTORY}/include )
if( APPLE )
    set( OPENMESH_LIBRARIES "${RADIUM_SUBMODULES_INSTALL_DIRECTORY}/lib/libOpenMeshCore.dylib")
elseif ( UNIX )
    set( OPENMESH_LIBRARIES "${RADIUM_SUBMODULES_INSTALL_DIRECTORY}/lib/libOpenMeshCore.so")
elseif (MINGW)
    set( OPENMESH_LIBRARIES "${RADIUM_SUBMODULES_INSTALL_DIRECTORY}/lib/libOpenMeshCore.dll.a")
elseif( MSVC )
    # in order to prevent DLL hell, each of the DLLs have to be suffixed with the major version and msvc prefix
    if( MSVC70 OR MSVC71 )
        set(MSVC_PREFIX "vc70")
    elseif( MSVC80 )
        set(MSVC_PREFIX "vc80")
    elseif( MSVC90 )
        set(MSVC_PREFIX "vc90")
    elseif( MSVC10 )
        set(MSVC_PREFIX "vc100")
    elseif( MSVC11 )
        set(MSVC_PREFIX "vc110")
    elseif( MSVC12 )
        set(MSVC_PREFIX "vc120")
    else()
        set(MSVC_PREFIX "vc140")
    endif()

    set(OPENMESH_LIBRARIES optimized "${RADIUM_SUBMODULES_INSTALL_DIRECTORY}/lib/openmesh-${MSVC_PREFIX}-mt.lib")

endif()