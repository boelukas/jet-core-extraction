CMAKE_MINIMUM_REQUIRED(VERSION 2.8.0)

PROJECT(jet CXX C)


#if(POLICY CMP0043)
#  # http://www.cmake.org/cmake/help/v3.0/policy/CMP0043.html
#  cmake_policy(SET CMP0043 OLD)
#endif()

if(UNIX) 
  set(CMAKE_CXX_STANDARD 17)
  set(CMAKE_CXX_STANDARD_REQUIRED ON)
  Set(CMAKE_BUILD_TYPE Release)
  set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -DNDEBUG")
  # set(CMAKE_REQUIRED_FLAGS -std=c++17 -lstdc++fs)
endif()
if (WIN32)
  set(CMAKE_CXX_STANDARD 17)
  set(CMAKE_CXX_STANDARD_REQUIRED ON)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
  add_compile_options(-Zc:__cplusplus)
endif()
######################################
# defining the flags
if(NOT DEFINED USE_QT)
  set(USE_QT false CACHE BOOL "Enable Qt support?")
endif(NOT DEFINED USE_QT)
######################################

SET (CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/cmake)
###############################################################################Experiment
#SET(CMAKE_BUILD_TYPE Debug)

###############################################################################Experiment


find_package(VTK REQUIRED)
if(WIN32)
include(${VTK_USE_FILE})    
endif()

SET(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR})
SET(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR})

IF(WIN32)
  ADD_DEFINITIONS(-DNOMINMAX)
  #ADD_DEFINITIONS(-DWIN32_LEAN_AND_MEAN)
  #ADD_DEFINITIONS(-D_USE_MATH_DEFINES)
  #ADD_DEFINITIONS(-DNODEFAULTLIB)
  #ADD_DEFINITIONS(-DNOGDI)
  #ADD_DEFINITIONS(-D_VARIADIC_MAX=10)  
  add_compile_options(-bigobj)
ENDIF()

FIND_PACKAGE(OpenMP)
IF(OPENMP_FOUND)
  MESSAGE(STATUS "Using OpenMP parallelization")
  SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
  SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")
ELSE()
  MESSAGE(STATUS "Not using OpenMP parallelization")
ENDIF()

INCLUDE_DIRECTORIES(${CMAKE_HOME_DIRECTORY})

ADD_SUBDIRECTORY(core)
ADD_SUBDIRECTORY(cmd)

if (USE_QT)
ADD_SUBDIRECTORY(viewer)
endif()
