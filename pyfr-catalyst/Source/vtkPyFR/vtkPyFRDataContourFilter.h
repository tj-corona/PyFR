#ifndef VTKPYFRDATACONTOURFILTER_H
#define VTKPYFRDATACONTOURFILTER_H

#include <string>

#include "vtkPyFRContourDataAlgorithm.h"

class VTK_EXPORT vtkPyFRDataContourFilter : public vtkPyFRContourDataAlgorithm
{
public:
  vtkTypeMacro(vtkPyFRDataContourFilter,vtkPyFRContourDataAlgorithm)
  static vtkPyFRDataContourFilter* New();
  virtual void PrintSelf(ostream& os, vtkIndent indent);

  int RequestData(
    vtkInformation *,
    vtkInformationVector **,
    vtkInformationVector *);

  int FillInputPortInformation(int,vtkInformation*);

  void SetContourValue(double value) { this->ContourValue = value; }

  void SetContourField(int i) { this->ContourField = i; }

protected:
  vtkPyFRDataContourFilter();
  virtual ~vtkPyFRDataContourFilter();

  FPType ContourValue;
  int ContourField;

private:
  static int PyFRDataTypesRegistered;
  static int RegisterPyFRDataTypes();

  vtkPyFRDataContourFilter(const vtkPyFRDataContourFilter&);
  void operator=(const vtkPyFRDataContourFilter&);
};
#endif
