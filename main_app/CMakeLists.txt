cmake_minimum_required(VERSION 3.10)

project(test_gl)

set(CMAKE_CXX_STANDARD 11)

find_package(OpenMP REQUIRED)
find_package(Threads REQUIRED)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mavx")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -fsanitize-address-use-after-scope -fno-omit-frame-pointer -fsanitize=leak -fsanitize=undefined -fsanitize=bounds-strict")

add_executable(test_gl
               tests_sc1.h
               tests_sc1.cpp
               tests_sc2.cpp
               HydraExport.h
               HydraExport.cpp
               main_linux.cpp)

target_link_libraries(test_gl swgl ${OpenMP_CXX_FLAGS}) #${OpenMP_CXX_FLAGS}

set_target_properties(test_gl PROPERTIES LINKER_LANGUAGE CXX)
target_compile_options(test_gl PUBLIC -std=c++11 -fpermissive -w -Wall ${OpenMP_CXX_FLAGS}) # ${OpenMP_CXX_FLAGS}
