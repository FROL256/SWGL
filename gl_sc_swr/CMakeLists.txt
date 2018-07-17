cmake_minimum_required(VERSION 3.10)

project(swgl)


set(CMAKE_CXX_STANDARD 11)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -msse4.1") # disable this for arm later ...

set(SOURCE_LIB
        alloc16.h
        gl_sc.h
        gl_std.h
        HWAbstractionLayer.h
        PureCpp.cpp
        swgl.cpp
        TexSampler.h
        TriRaster.h
        BatchExport.cpp
        gl_sc_minimal.cpp
        LiteMath.h
        PureCpp.h
        swgl.h
        tri_raster_shared.h
        gl_sc_standart.cpp
        glx_sc.cpp
        PureCppBlock4x4.cpp
        SSE1Impl.cpp
        SWGL_TiledFrameBuffer.cpp
        Timer.h
        config.h
        gl_standart.cpp
        glx_sc.h
        pugiconfig.hpp
        PureCppBlock4x4.h
        SSE1Impl.h
        SWGL_TiledFrameBuffer.h
        TimerLinux.cpp
        )

MESSAGE ( STATUS "Searching for X11..." )
FIND_PACKAGE ( X11 REQUIRED )
IF ( X11_FOUND )
    INCLUDE_DIRECTORIES ( ${X11_INCLUDE_DIR} )
    LINK_LIBRARIES ( ${X11_LIBRARIES} )
    MESSAGE ( STATUS " X11_INCLUDE_DIR: " ${X11_INCLUDE_DIR} )
    MESSAGE ( STATUS " X11_LIBRARIES: " ${X11_LIBRARIES} )
ENDIF ( X11_FOUND )

add_library(swgl STATIC ${SOURCE_LIB})

set_target_properties(swgl PROPERTIES LINKER_LANGUAGE CXX)
target_compile_options(swgl PUBLIC -std=c++11 -fpermissive -w -Wall)