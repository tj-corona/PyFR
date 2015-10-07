#ifndef VTKPYFRCONTOURDATA_H
#define VTKPYFRCONTOURDATA_H

#include <vtkDataObject.h>

class PyFRContourData;

class VTK_EXPORT vtkPyFRContourData : public vtkDataObject
{
public:
  static vtkPyFRContourData* New();
  vtkTypeMacro(vtkPyFRContourData, vtkDataObject)
  virtual void PrintSelf(ostream& os, vtkIndent indent);

  void SetData(PyFRContourData* d) { data = d; }
  PyFRContourData* GetData() const { return data; }

  void ReleaseResources();

protected:
  vtkPyFRContourData();
  virtual ~vtkPyFRContourData();

private:
  PyFRContourData* data;
};

vtkObject* New_vtkPyFRContourData();

#endif
