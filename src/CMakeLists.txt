##
## VGG Image Search Engine (VISE)
## vise_server
##
## Author: Abhishek Dutta <adutta@robots.ox.ac.uk>
## 18 June 2018
##

cmake_minimum_required( VERSION 3.10 )
project( 15cillustration )

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(EXECUTABLE_OUTPUT_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../bin")
set( BUILD_SHARED_LIBS "OFF" )

############################## MPI settings
message("=========================== MPI: ON")
ADD_DEFINITIONS(-DRR_MPI)
set( REQURE_BOOST_MPI "mpi" )
message("=========================== Registration: ON")
ADD_DEFINITIONS(-DRR_REGISTER)
ADD_DEFINITIONS(-DRR_MAGICK)
set( cREGISTER 1 )

## MPI
find_package(MPI REQUIRED)
if (MPI_FOUND)
    include_directories(SYSTEM ${MPI_INCLUDE_PATH})
else (MPI_FOUND)
    message(SEND_ERROR "This application cannot compile without MPI")
endif (MPI_FOUND)

## Boost
find_package(Boost 1.64.0 COMPONENTS mpi filesystem system thread atomic log serialization REQUIRED)
if(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
	set( Boost_USE_MULTITHREADED ON )
  set( Boost_USE_DEBUG_RUNTIME OFF )
  set( Boost_USE_STATIC_RUNTIME OFF )
  set( Boost_USE_STATIC_LIBS OFF )
  set( BOOST_ALL_DYN_LINK ON)
  set( Boost_DEBUG OFF )
  add_definitions(-DBOOST_ALL_NO_LIB)
  add_definitions(${Boost_LIB_DIAGNOSTIC_DEFINITIONS})
endif()
link_directories( ${Boost_LIBRARY_DIRS} )
include_directories( ${Boost_INCLUDE_DIRS} )

## ImageMagick
ADD_DEFINITIONS(-DMAGICKCORE_HDRI_ENABLE=0)
ADD_DEFINITIONS(-DMAGICKCORE_QUANTUM_DEPTH=8)
find_package( ImageMagick COMPONENTS Magick++ REQUIRED )
# @todo: on alienware, find_package() points to "/usr/lib/x86_64-linux-gnu/libMagick++-6.Q16.so"
# and does not detect the library available at "/home/tlm/deps/imcomp/lib/lib/libMagick++-6.Q8.so"
#set(ImageMagick_LIBRARIES "/home/tlm/deps/vise/lib/lib/libMagick++-6.Q8.so")
include_directories( ${ImageMagick_INCLUDE_DIRS} )

# Eigen
# requires EIGEN_DIR to be set to the location where Eigen3 is built using cmake
find_package (Eigen3 3.3 REQUIRED NO_MODULE)
include_directories( ${EIGEN3_INCLUDE_DIR} )

## Protocol Buffer
## required by "search_engine/relja_retrival"
include(FindProtobuf)
find_package( Protobuf REQUIRED )
include_directories( ${PROTOBUF_INCLUDE_DIR} )

## James's FASTANN (download from robots/software)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/external/")
find_package(fastann REQUIRED)
include_directories( ${fastann_INCLUDE_DIRS} )

set(CMAKE_INCLUDE_CURRENT_DIR ON)

include_directories( api )
include_directories( compression )
include_directories( evaluation )
include_directories( external )
include_directories( external/libsvm )
include_directories( external/eigen )
include_directories( external/KMCode_relja/exec/detect_points )
include_directories( external/KMCode_relja/exec/compute_descriptors )
include_directories( preprocessing )
include_directories( indexing )
include_directories( matching )
include_directories( matching/det_ransac )
include_directories( matching/registration )
include_directories( nn )
include_directories( represent )
include_directories( retrieval )
#include_directories( tests ) # does not compile (for some reason)
include_directories( util )
include_directories( v2/api )
include_directories( v2/dataset )
include_directories( v2/embedding )
include_directories( v2/evaluation )
include_directories( v2/indexing )
include_directories( v2/indexing/train )
include_directories( v2/retrieval )

# for protobufs
include_directories( ${PROJECT_BINARY_DIR}/util )
include_directories( ${PROJECT_BINARY_DIR}/v2/dataset )
include_directories( ${PROJECT_BINARY_DIR}/v2/embedding )
include_directories( ${PROJECT_BINARY_DIR}/v2/evaluation )
include_directories( ${PROJECT_BINARY_DIR}/v2/indexing )
include_directories( ${PROJECT_BINARY_DIR}/v2/retrieval )


add_subdirectory( api )
add_subdirectory( compression )
add_subdirectory( evaluation )
add_subdirectory( external )
add_subdirectory( indexing )
add_subdirectory( matching )
add_subdirectory( nn )
add_subdirectory( preprocessing )
add_subdirectory( represent )
add_subdirectory( retrieval )
#add_subdirectory( tests ) # does not compile (for some reason)
add_subdirectory( util )
add_subdirectory( v2 )
