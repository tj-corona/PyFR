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
include(VTKmMacros)
vtkm_configure_device(Serial REQUIRED)
vtkm_configure_device(Cuda REQUIRED)

find_package(BoostHeaders ${VTKm_REQUIRED_BOOST_VERSION} REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})

set(Adaptor_HDRS
  ArrayHandleExposed.h
  vtkPyFRContourDataAlgorithm.h
  vtkPyFRDataAlgorithm.h
  vtkXMLPyFRContourDataWriter.h
  vtkXMLPyFRDataWriter.h
  vtkCPVTKmPipeline.h
  vtkPyFRDataContourFilter.h
  PyFRContourData.h
  PyFRData.h
  PyFRAdaptor.h
  )

set(Adaptor_SRCS
  vtkPyFRContourDataAlgorithm.cu
  vtkPyFRDataAlgorithm.cu
  vtkXMLPyFRContourDataWriter.cu
  vtkXMLPyFRDataWriter.cu
  vtkCPVTKmPipeline.cu
  vtkPyFRDataContourFilter.cu
  PyFRContourData.cu
  PyFRData.cu
  PyFRAdaptor.cu
  )


SET( CMAKE_CXX_FLAGS_ORIGINAL  "${CMAKE_CXX_FLAGS}" )

SET( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS_ORIGINAL} -DFPType=float" )
cuda_add_library(pyfr-catalyst-fp32 SHARED ${Adaptor_SRCS})
target_link_libraries(pyfr-catalyst-fp32 vtkPVCatalyst vtkPVVTKExtensionsDefault vtkPVClientServerCoreCore ${CUDA_LIBRARIES})

SET( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS_ORIGINAL} -DFPType=double" )
cuda_add_library(pyfr-catalyst-fp64 SHARED ${Adaptor_SRCS})
target_link_libraries(pyfr-catalyst-fp64 vtkPVCatalyst vtkPVVTKExtensionsDefault vtkPVClientServerCoreCore ${CUDA_LIBRARIES})

install(TARGETS pyfr-catalyst-fp32 pyfr-catalyst-fp64 DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)
