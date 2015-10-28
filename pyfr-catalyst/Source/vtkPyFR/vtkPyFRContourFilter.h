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

  void SetNumberOfContours(int i);
  void SetContourValue(int i,double value);

  void SetContourField(int i);
  void SetMappedField(int i);

protected:
  vtkPyFRContourFilter();
  virtual ~vtkPyFRContourFilter();

  std::vector<double> ContourValues;
  int ContourField;
  int MappedField;

private:
  static int PyFRDataTypesRegistered;
  static int RegisterPyFRDataTypes();

  vtkPyFRContourFilter(const vtkPyFRContourFilter&);
  void operator=(const vtkPyFRContourFilter&);
};
#endif
