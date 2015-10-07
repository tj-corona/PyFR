set(PyFR_SRCS
  PyFRContourData.cu
  PyFRData.cu
  PyFRContourFilter.cu
  PyFRConverter.cu
  PyFRWriter.cu
)

set( PyFRLibs )
set( CMAKE_CXX_FLAGS_ORIGINAL  "${CMAKE_CXX_FLAGS}" )
foreach (fpType IN LISTS fpList)
  set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS_ORIGINAL} -DFPType=${fpType}" )
  cuda_add_library("pyfr_${libSuffix_${fpType}}" SHARED ${PyFR_SRCS} )
  list( APPEND PyFRLibs "pyfr_${libSuffix_${fpType}}" )
endforeach()

install( TARGETS ${PyFRLibs} DESTINATION ${CMAKE_INSTALL_PREFIX}/lib )
