#ifndef VTKPYFRDATACONTOURFILTER_H
#define VTKPYFRDATACONTOURFILTER_H

#include <string>
#include <vector>

#include "vtkPyFRContourDataAlgorithm.h"

class PyFRParallelSliceFilter;

class VTK_EXPORT vtkPyFRParallelSliceFilter : public vtkPyFRContourDataAlgorithm
{
public:
  vtkTypeMacro(vtkPyFRParallelSliceFilter,vtkPyFRContourDataAlgorithm)
  static vtkPyFRParallelSliceFilter* New();
  virtual void PrintSelf(ostream& os, vtkIndent indent);

  int RequestData(vtkInformation*,vtkInformationVector**,vtkInformationVector*);

  int FillInputPortInformation(int,vtkInformation*);

  vtkSetVector3Macro(Origin,double);
  vtkGetVectorMacro(Origin,double,3);

  vtkSetVector3Macro(Normal,double);
  vtkGetVectorMacro(Normal,double,3);

  vtkSetMacro(Spacing,double);
  vtkGetMacro(Spacing,double);

  vtkSetMacro(NumberOfPlanes,int);
  vtkGetMacro(NumberOfPlanes,int);

  vtkSetMacro(MappedField,int);
  vtkGetMacro(MappedField,int);

protected:
  vtkPyFRParallelSliceFilter();
  virtual ~vtkPyFRParallelSliceFilter();

  double Origin[3];
  double Normal[3];
  double Spacing;
  int NumberOfPlanes;
  int MappedField;

  unsigned long LastExecuteTime;

  PyFRParallelSliceFilter* Filter;

private:
  static int PyFRDataTypesRegistered;
  static int RegisterPyFRDataTypes();

  vtkPyFRParallelSliceFilter(const vtkPyFRParallelSliceFilter&);
  void operator=(const vtkPyFRParallelSliceFilter&);
};
#endif
