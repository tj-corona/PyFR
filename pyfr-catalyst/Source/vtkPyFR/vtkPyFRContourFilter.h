#ifndef VTKPYFRDATACONTOURFILTER_H
#define VTKPYFRDATACONTOURFILTER_H

#include <string>
#include <vector>

#include "vtkPyFRContourDataAlgorithm.h"

class VTK_EXPORT vtkPyFRContourFilter : public vtkPyFRContourDataAlgorithm
{
public:
  vtkTypeMacro(vtkPyFRContourFilter,vtkPyFRContourDataAlgorithm)
  static vtkPyFRContourFilter* New();
  virtual void PrintSelf(ostream& os, vtkIndent indent);

  int RequestData(vtkInformation*,vtkInformationVector**,vtkInformationVector*);

  int FillInputPortInformation(int,vtkInformation*);

  void SetNumberOfContours(int i) { this->ContourValues.resize(i); }
  void SetContourValue(int i,double value) { this->ContourValues[i] = value; }

  void SetContourField(int i) { this->ContourField = i; }

protected:
  vtkPyFRContourFilter();
  virtual ~vtkPyFRContourFilter();

  std::vector<FPType> ContourValues;
  int ContourField;

private:
  static int PyFRDataTypesRegistered;
  static int RegisterPyFRDataTypes();

  vtkPyFRContourFilter(const vtkPyFRContourFilter&);
  void operator=(const vtkPyFRContourFilter&);
};
#endif
