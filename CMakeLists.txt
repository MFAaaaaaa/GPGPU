# In top-level or relevant subdir CMakeLists.txt
add_library(gpu_sm
src/cpu/gpu_sm/gpu_sm_param.h
src/cpu/gpu_sm/warp.h
src/cpu/gpu_sm/gpu_sm_cpu.h
src/cpu/gpu_sm/gpu_sm_cpu.cpp
src/dev/gpu/gpu_device.h
src/dev/gpu/gpu_device.cpp
)

# Ensure include dirs
target_include_directories(gpu_sm PUBLIC src)


# Link with your common targets
target_link_libraries(gpu_sm PRIVATE core cache bus sys) # TODO(integrate)



project(NullRVSim)

cmake_minimum_required(VERSION 3.00)

set(CMAKE_CXX_STANDARD 20)

if (NOT DEFINED CMAKE_BUILD_TYPE)
        set(CMAKE_BUILD_TYPE "Debug")
endif()

if (CMAKE_BUILD_TYPE STREQUAL "Release")
        set(CMAKE_CXX_FLAGS "-std=c++17 -O3 -DNDEBUG")
else ()
        set(CMAKE_CXX_FLAGS "-std=c++17 -g -O2")
endif ()

include_directories(
        src
        )

file(GLOB_RECURSE ALL_CXX_SRC
        src/*.cpp
        )

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/conf/ DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/conf/ FILES_MATCHING PATTERN "*.ini")

add_executable(nullrvsim ${ALL_CXX_SRC})

add_subdirectory(rvsrc/ecallproxy)
add_subdirectory(example)
