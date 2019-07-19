cmake_minimum_required(VERSION 3.10)

project(swgl)

set(CMAKE_CXX_STANDARD 14)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mavx") # disable this for arm later ...
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -funroll-loops")

set(SOURCE_LIB
        alloc16.h
        gl_sc.h
        gl_std.h
        HWAbstractionLayer.h
        HWPureCpp.cpp
        swgl.cpp
        TexSampler.h
        TriRaster.h
        BatchExport.cpp
        gl_sc_minimal.cpp
        LiteMath.h
        HWPureCpp.h
        swgl.h
        tri_raster_shared.h
        gl_sc_standart.cpp
        glx_sc.cpp
        SWGL_TiledFrameBuffer.cpp
        Timer.h
        config.h
        gl_standart.cpp
        glx_sc.h
        pugiconfig.hpp
        HWBlockLine4x4.h
        SWGL_TiledFrameBuffer.h
        Timer.cpp
        HW_VROP.h HW_VROP_FIXP.h
        vfloat4_x64.h vfloat4_gcc.h vfloat8_x64.h vfloat8_gcc.h
        TriRasterHalfSpaceBlockLineFixp.h
        HWBlockLine4x4.cpp HWBlockLine8x8.cpp HWBlockLine4x4Fixp.cpp)

include_directories("..")

#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -fsanitize-address-use-after-scope -fno-omit-frame-pointer -fsanitize=leak -fsanitize=undefined -fsanitize=bounds-strict")

MESSAGE(STATUS "Searching for X11..." )
FIND_PACKAGE ( X11 REQUIRED )
IF ( X11_FOUND )
    INCLUDE_DIRECTORIES ( ${X11_INCLUDE_DIR} )
    LINK_LIBRARIES ( ${X11_LIBRARIES} )
    MESSAGE ( STATUS " X11_INCLUDE_DIR: " ${X11_INCLUDE_DIR} )
    MESSAGE ( STATUS " X11_LIBRARIES: " ${X11_LIBRARIES} )
ENDIF ( X11_FOUND )


find_package(OpenMP REQUIRED)
find_package(Threads REQUIRED)

add_library(swgl STATIC ${SOURCE_LIB})

set_target_properties(swgl PROPERTIES LINKER_LANGUAGE CXX)
target_compile_options(swgl PUBLIC -std=c++11 -fpermissive -w -Wall ${OpenMP_CXX_FLAGS}) # ${OpenMP_CXX_FLAGS}