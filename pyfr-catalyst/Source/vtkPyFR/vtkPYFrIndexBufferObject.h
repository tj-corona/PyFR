#ifndef vtkPYFrIndexBufferObject_h
#define vtkPYFrIndexBufferObject_h

#include <vtkOpenGLIndexBufferObject.h>
#include "vtkPyFRContourData.h"

#include <vector>

class VTKRENDERINGOPENGL2_EXPORT vtkPYFrIndexBufferObject :
  public vtkOpenGLIndexBufferObject
{
public:
  static vtkPYFrIndexBufferObject *New();
  vtkTypeMacro(vtkPYFrIndexBufferObject, vtkOpenGLIndexBufferObject)
  void PrintSelf(ostream& os, vtkIndent indent);

  std::size_t CreateIndexBuffer(vtkPyFRContourData* data, int index);
  std::size_t CreateTriangleLineIndexBuffer(vtkPyFRContourData* data, int index);


protected:
  vtkPYFrIndexBufferObject();
  ~vtkPYFrIndexBufferObject();

private:
  vtkPYFrIndexBufferObject(const vtkPYFrIndexBufferObject&); // Not implemented
  void operator=(const vtkPYFrIndexBufferObject&); // Not implemented


  //hack to pre-compute the indexArray once, since it is just an
  //explicit array handle counting
  std::vector<unsigned int> IndexArray;

};
#endif
