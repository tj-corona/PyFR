include_directories(${PROJECT_SOURCE_DIR}/Source/PyFR)

set(vtkPyFR_SRCS
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

set( vtkPyFRLibs )
set( CMAKE_CXX_FLAGS_ORIGINAL  "${CMAKE_CXX_FLAGS}" )
foreach (fpType IN LISTS fpList)
  set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS_ORIGINAL} -DFPType=${fpType}" )
  set (pyfrLib "pyfr_${libSuffix_${fpType}}")
  set (pluginLib "pyfr_plugin_${libSuffix_${fpType}}")
  set (catalystLib "pyfr_catalyst_${libSuffix_${fpType}}")

  add_paraview_plugin(${pluginLib} "1.0" SERVER_MANAGER_XML PyFR.xml SERVER_MANAGER_SOURCES ${vtkPyFR_SRCS})
  target_link_libraries(${pluginLib} PRIVATE vtkPVCatalyst vtkPVVTKExtensionsDefault vtkPVClientServerCoreCore ${pyfrLib} ${CUDA_LIBRARIES})
  list( APPEND vtkPyFRLibs ${pluginLib} )

  add_library(${catalystLib} SHARED ${vtkPyFR_SRCS})
  target_link_libraries(${catalystLib} PRIVATE vtkPVCatalyst vtkPVVTKExtensionsDefault vtkPVClientServerCoreCore ${VTK_LIBRARIES} ${pyfrLib} ${CUDA_LIBRARIES})
  list( APPEND vtkPyFRLibs ${catalystLib} )
endforeach()

install(TARGETS ${vtkPyFRLibs} DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)
