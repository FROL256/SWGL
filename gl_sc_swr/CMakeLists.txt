cmake_minimum_required(VERSION 3.10)

project(swgl)

set(CMAKE_CXX_STANDARD 11)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mavx512f -mavx512cd -mavx512dq ") # disable this for arm later ...
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -funroll-loops")

if ( CMAKE_BUILD_TYPE STREQUAL "Release" )
  add_definitions( -DNDEBUG )
endif()

set(SOURCE_LIB
        HWPureCpp.cpp
        swgl.cpp
        BatchExport.cpp
        gl_sc_minimal.cpp
        gl_sc_standart.cpp
        glx_sc.cpp
        gl_standart.cpp
        Timer.cpp
        HWBlock4x4.cpp HWBlock8x2.cpp HWBlock16x1.cpp)

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