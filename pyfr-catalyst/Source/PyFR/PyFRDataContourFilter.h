#ifndef PYFRDATACONTOURFILTER_H
#define PYFRDATACONTOURFILTER_H

#include <string>

//State that the default backend for this code is CUDA
//not serial
// #define VTKM_DEVICE_ADAPTER VTKM_DEVICE_ADAPTER_CUDA
//Disable treading support in our array handle
//needed for nvcc to stop complaining.
#define BOOST_SP_DISABLE_THREADS

class PyFRData;
class PyFRContourData;

class PyFRDataContourFilter
{
public:
  PyFRDataContourFilter();
  virtual ~PyFRDataContourFilter();

  void operator ()(PyFRData*,PyFRContourData*) const;

  void SetContourValue(FPType value) { this->ContourValue = value; }

  void SetContourField(int i);
  void SetContourFieldToDensity()    { this->ContourField = "density"; }
  void SetContourFieldToPressure()   { this->ContourField = "pressure"; }
  void SetContourFieldToVelocity_u() { this->ContourField = "velocity_u"; }
  void SetContourFieldToVelocity_v() { this->ContourField = "velocity_v"; }
  void SetContourFieldToVelocity_w() { this->ContourField = "velocity_w"; }

  void ProjectFieldOntoContour(int i);
  void ProjectDensityOntoContour()    { this->ProjectedField = "density"; }
  void ProjectPressureOntoContour()   { this->ProjectedField = "pressure"; }
  void ProjectVelocity_uOntoContour() { this->ProjectedField = "velocity_u"; }
  void ProjectVelocity_vOntoContour() { this->ProjectedField = "velocity_v"; }
  void ProjectVelocity_wOntoContour() { this->ProjectedField = "velocity_w"; }

protected:
  FPType ContourValue;
  std::string ContourField;
  std::string ProjectedField;
};
#endif
