#ifndef vtkPYFrVertexBufferObject_h
#define vtkPYFrVertexBufferObject_h

#include <vtkOpenGLVertexBufferObject.h>
#include "vtkPyFRContourData.h"


class VTKRENDERINGOPENGL2_EXPORT vtkPYFrVertexBufferObject :
  public vtkOpenGLVertexBufferObject
{
public:
  static vtkPYFrVertexBufferObject *New();
  vtkTypeMacro(vtkPYFrVertexBufferObject, vtkOpenGLBufferObject)
  void PrintSelf(ostream& os, vtkIndent indent);

  //Create VBO's for the contour data
  void CreateVBO(vtkPyFRContourData* data);

  typedef typename PyFRContourData::Vec3ArrayHandle vec3;
  typedef typename PyFRContourData::ScalarDataArrayHandle scalar;
  void Vertices(const vec3& vertices);
  void Normals(const vec3& normals);
  void Scalars(const scalar& scalars);
  size_t vertsize() const; ///< returns the number of vertices set

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
  vtkPYFrVertexBufferObject();
  ~vtkPYFrVertexBufferObject();

private:
  vtkPYFrVertexBufferObject(const vtkPYFrVertexBufferObject&); // Not implemented
  void operator=(const vtkPYFrVertexBufferObject&); // Not implemented

  unsigned vao;
  unsigned vertices, normals, scalars, idx;
  size_t nverts;
};
#endif
