#ifndef vtkPYFrVertexBufferObject_h
#define vtkPYFrVertexBufferObject_h

#include <vtkOpenGLVertexBufferObject.h>
#include "vtkPyFRContourData.h"


class VTKRENDERINGOPENGL2_EXPORT vtkPYFrVertexBufferObject :
  public vtkOpenGLVertexBufferObject
{
public:
  static vtkPYFrVertexBufferObject *New();
  vtkTypeMacro(vtkPYFrVertexBufferObject, vtkOpenGLVertexBufferObject)
  void PrintSelf(ostream& os, vtkIndent indent);

  //Create VBO's for the contour data
  void CreateVerticesVBO(vtkPyFRContourData* data, int index);
  void CreateNormalsVBO(vtkPyFRContourData* data, int index);
  void CreateColorsVBO(vtkPyFRContourData* data, int index);

protected:
  vtkPYFrVertexBufferObject();
  ~vtkPYFrVertexBufferObject();

private:
  vtkPYFrVertexBufferObject(const vtkPYFrVertexBufferObject&); // Not implemented
  void operator=(const vtkPYFrVertexBufferObject&); // Not implemented
};
#endif
