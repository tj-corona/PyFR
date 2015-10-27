/*=========================================================================

  Program:   Visualization Toolkit

  Copyright (c) 2015 NVIDIA
  All rights reserved.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#ifndef vtkmOpenGLVBO_h
#define vtkmOpenGLVBO_h

#define VTKM_DEVICE_ADAPTER VTKM_DEVICE_ADAPTER_CUDA
#include <vtkOpenGLVertexBufferObject.h>
#include "PyFRContourData.h"

/**
 * @brief OpenGL vertex buffer object for data from VTKm.
 *
 * Here be dragons.  You shouldn't be using this outside of the VTKm/PyFR demo.
 */

class VTKRENDERINGOPENGL2_EXPORT vtkmOpenGLVBO :
  public vtkOpenGLVertexBufferObject
{
public:
  static vtkmOpenGLVBO *New();
  vtkTypeMacro(vtkmOpenGLVBO, vtkOpenGLBufferObject)
  void PrintSelf(ostream& os, vtkIndent indent);

  // Doesn't do anything.  Exists for compatibility reasons.  You must
	// use the alternate version below.
	// Terrible hack.
  void CreateVBO(vtkPoints *points, unsigned int numPoints,
      vtkDataArray *normals,
      vtkDataArray *tcoords,
      unsigned char *colors, int colorComponents);

  typedef typename PyFRContourData::Vec3ArrayHandle vec3;
  typedef typename PyFRContourData::ScalarDataArrayHandle scalar;
	void Vertices(const vec3& vertices);
  void Normals(const vec3& normals);
  void Scalars(const scalar& scalars);
	size_t nindices() const; ///< returns the number of indices

	// Binds the VBO.  Call 'CreateVBO' first.  Always pair with Release.
	bool Bind();
	bool Release();
	void ReleaseGraphicsResources();

	// For compatibility.  Aborts if called.
  void AppendVBO(vtkPoints *points, unsigned int numPoints,
      vtkDataArray *normals,
      vtkDataArray *tcoords,
      unsigned char *colors, int colorComponents);

protected:
  vtkmOpenGLVBO();
  ~vtkmOpenGLVBO();

private:
  vtkmOpenGLVBO(const vtkmOpenGLVBO&); // Not implemented
  void operator=(const vtkmOpenGLVBO&); // Not implemented

	unsigned vao;
	unsigned vertices, normals, scalars, idx;
	size_t nidx;
};
#endif
