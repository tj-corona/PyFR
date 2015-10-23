#ifndef VTKPYFRCRINKLECLIPFILTER_H
#define VTKPYFRCRINKLECLIPFILTER_H

#include <string>

#include "vtkPyFRDataAlgorithm.h"

class vtkImplicitFunction;
class vtkPlane;

class VTK_EXPORT vtkPyFRCrinkleClipFilter : public vtkPyFRDataAlgorithm
{
public:
  vtkTypeMacro(vtkPyFRCrinkleClipFilter,vtkPyFRDataAlgorithm)
  static vtkPyFRCrinkleClipFilter* New();
  virtual void PrintSelf(ostream& os, vtkIndent indent);

  virtual void SetClipFunction(vtkImplicitFunction*);
  vtkGetObjectMacro(ClipFunction,vtkImplicitFunction);

  void SetInputData(vtkDataObject*);
  void SetInputData(int,vtkDataObject*);
  int RequestData(vtkInformation*,vtkInformationVector**,vtkInformationVector*);
  int FillInputPortInformation(int,vtkInformation*);

  unsigned long GetMTime();

protected:
  vtkImplicitFunction *ClipFunction;
  unsigned long LastExecuteTime;

  vtkPyFRCrinkleClipFilter();
  virtual ~vtkPyFRCrinkleClipFilter();

private:
  static int PyFRDataTypeRegistered;
  static int RegisterPyFRDataType();

  vtkPyFRCrinkleClipFilter(const vtkPyFRCrinkleClipFilter&); // Not implemented
  void operator=(const vtkPyFRCrinkleClipFilter&); // Not implemented
};
#endif
