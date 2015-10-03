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
include(ParaViewPlugins)

find_package(VTKm REQUIRED)
include(VTKmMacros)
vtkm_configure_device(Serial REQUIRED)
vtkm_configure_device(Cuda REQUIRED)

find_package(BoostHeaders ${VTKm_REQUIRED_BOOST_VERSION} REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})

set(Adaptor_CUDA_SRCS
  PyFRContourData.cu
  PyFRData.cu
  PyFRDataContourFilter.cu
  PyFRDataWriter.cu
)

set(Adaptor_SRCS
  vtkPyFRAdaptor.cxx
  vtkPyFRContourData.cxx
  vtkPyFRContourDataAlgorithm.cxx
  vtkPyFRData.cxx
  vtkPyFRDataAlgorithm.cxx
  vtkPyFRDataContourFilter.cxx
  vtkPyFRPipeline.cxx
  vtkXMLPyFRContourDataWriter.cxx
  vtkXMLPyFRDataWriter.cxx
  )

set( CMAKE_CXX_FLAGS_ORIGINAL  "${CMAKE_CXX_FLAGS}" )

set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS_ORIGINAL} -DFPType=float" )

cuda_add_library(pyfr_fp32 SHARED ${Adaptor_CUDA_SRCS})
add_paraview_plugin(pyfr_plugin_fp32 "1.0" SERVER_MANAGER_XML PyFR.xml SERVER_MANAGER_SOURCES ${Adaptor_SRCS})
add_library(pyfr_catalyst_fp32 SHARED ${Adaptor_SRCS})
target_link_libraries(pyfr_plugin_fp32 PRIVATE vtkPVCatalyst vtkPVVTKExtensionsDefault vtkPVClientServerCoreCore pyfr_fp32 ${CUDA_LIBRARIES})
target_link_libraries(pyfr_catalyst_fp32 PRIVATE vtkPVCatalyst vtkPVVTKExtensionsDefault vtkPVClientServerCoreCore ${VTK_LIBRARIES} pyfr_fp32 ${CUDA_LIBRARIES})

SET( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS_ORIGINAL} -DFPType=double" )

cuda_add_library(pyfr_fp64 SHARED ${Adaptor_CUDA_SRCS})
add_paraview_plugin(pyfr_plugin_fp64 "1.0" SERVER_MANAGER_XML PyFR.xml SERVER_MANAGER_SOURCES ${Adaptor_SRCS})
add_library(pyfr_catalyst_fp64 SHARED ${Adaptor_SRCS})
target_link_libraries(pyfr_plugin_fp64 PRIVATE vtkPVCatalyst vtkPVVTKExtensionsDefault vtkPVClientServerCoreCore pyfr_fp64 ${CUDA_LIBRARIES})
target_link_libraries(pyfr_catalyst_fp64 PRIVATE vtkPVCatalyst vtkPVVTKExtensionsDefault vtkPVClientServerCoreCore ${VTK_LIBRARIES} pyfr_fp64 ${CUDA_LIBRARIES})

install(TARGETS pyfr_fp32 pyfr_plugin_fp32 pyfr_catalyst_fp32 pyfr_fp64 pyfr_plugin_fp64 pyfr_catalyst_fp64 DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)
