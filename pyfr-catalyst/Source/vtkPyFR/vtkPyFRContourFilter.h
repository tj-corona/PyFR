#ifndef VTKPYFRDATACONTOURFILTER_H
#define VTKPYFRDATACONTOURFILTER_H

#include <string>

#include "vtkPyFRContourDataAlgorithm.h"

class VTK_EXPORT vtkPyFRContourFilter : public vtkPyFRContourDataAlgorithm
{
public:
  vtkTypeMacro(vtkPyFRContourFilter,vtkPyFRContourDataAlgorithm)
  static vtkPyFRContourFilter* New();
  virtual void PrintSelf(ostream& os, vtkIndent indent);

  int RequestData(vtkInformation*,vtkInformationVector**,vtkInformationVector*);

  int FillInputPortInformation(int,vtkInformation*);

  void SetContourValue(double value) { this->ContourValue = value; }

  void SetContourField(int i) { this->ContourField = i; }

protected:
  vtkPyFRContourFilter();
  virtual ~vtkPyFRContourFilter();

  FPType ContourValue;
  int ContourField;

private:
  static int PyFRDataTypesRegistered;
  static int RegisterPyFRDataTypes();

  vtkPyFRContourFilter(const vtkPyFRContourFilter&);
  void operator=(const vtkPyFRContourFilter&);
};
#endif
