#ifndef VTKPYFRDATACONTOURFILTER_H
#define VTKPYFRDATACONTOURFILTER_H

#include <string>

#include "vtkPyFRContourDataAlgorithm.h"

//State that the default backend for this code is CUDA
//not serial
#define VTKM_DEVICE_ADAPTER VTKM_DEVICE_ADAPTER_CUDA
//Disable treading support in our array handle
//needed for nvcc to stop complaining.
#define BOOST_SP_DISABLE_THREADS

class vtkPyFRDataContourFilter : public vtkPyFRContourDataAlgorithm
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

  void SetContourValue(FPType value) { this->ContourValue = value; }

  void SetContourFieldToDensity()    { this->ContourField = "density"; }
  void SetContourFieldToPressure()   { this->ContourField = "pressure"; }
  void SetContourFieldToVelocity_u() { this->ContourField = "velocity_u"; }
  void SetContourFieldToVelocity_v() { this->ContourField = "velocity_v"; }
  void SetContourFieldToVelocity_w() { this->ContourField = "velocity_w"; }

protected:
  vtkPyFRDataContourFilter();
  virtual ~vtkPyFRDataContourFilter();

  FPType ContourValue;
  std::string ContourField;

private:
  vtkPyFRDataContourFilter(const vtkPyFRDataContourFilter&);
  void operator=(const vtkPyFRDataContourFilter&);
};
#endif
