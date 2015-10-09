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
  set (pyfrLib "pyfr_${libSuffix_${fpType}}")
  set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS_ORIGINAL} -DFPType=${fpType}" )
  cuda_add_library(${pyfrLib} SHARED ${PyFR_SRCS})
  target_link_libraries(${pyfrLib} ${MPI_LIBRARIES})
  if(MPI_COMPILE_FLAGS)
    set_target_properties(${pyfrLib} PROPERTIES
      COMPILE_FLAGS "${MPI_COMPILE_FLAGS}")
  endif()
  if(MPI_LINK_FLAGS)
    set_target_properties(${pyfrLib} PROPERTIES
      LINK_FLAGS "${MPI_LINK_FLAGS}")
  endif()
  list( APPEND PyFRLibs ${pyfrLib} )
endforeach()

install( TARGETS ${PyFRLibs} DESTINATION ${CMAKE_INSTALL_PREFIX}/lib )
