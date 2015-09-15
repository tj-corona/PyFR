cmake_minimum_required(VERSION 2.8.8)
project(pyfr-catalyst)

# Always use a full RPATH
if(APPLE)
  set(CMAKE_MACOSX_RPATH ON)
  set(CMAKE_SKIP_BUILD_RPATH FALSE)
  set(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)
  set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
  set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
  list(FIND CMAKE_PLATFORM_IMPLICIT_LINK_DIRECTORIES
    "${CMAKE_INSTALL_PREFIX}/lib" isSystemDir)
  if("${isSystemDir}" STREQUAL "-1")
    set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
  endif()
endif()

find_package(ParaView 4.1 REQUIRED COMPONENTS vtkPVCatalyst vtkPVVTKExtensionsDefault vtkPVClientServerCoreCore)
if(NOT PARAVIEW_USE_MPI)
  message(SEND_ERROR "ParaView must be built with MPI enabled")
endif()
include("${PARAVIEW_USE_FILE}")

find_package(VTKm REQUIRED)
include_directories(${VTKm_INCLUDE_DIRS})

find_package(BoostHeaders ${VTKm_REQUIRED_BOOST_VERSION} REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})

find_package(CUDA REQUIRED)
include_directories(${CUDA_INCLUDE_DIRS})

set(Adaptor_SRCS
  vtkCPVTKPipeline.cxx
  FieldData.cxx
  PyFRAdaptor.cxx
  )
add_library(pyfr-catalyst SHARED ${Adaptor_SRCS})
target_link_libraries(pyfr-catalyst vtkPVCatalyst vtkPVVTKExtensionsDefault vtkPVClientServerCoreCore ${CUDA_LIBRARIES})
install(TARGETS pyfr-catalyst DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)
