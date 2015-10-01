#ifndef PYFRCONTOURDATA_H
#define PYFRCONTOURDATA_H

#include <vtkDataObject.h>

//State that the default backend for this code is CUDA
//not serial
#define VTKM_DEVICE_ADAPTER VTKM_DEVICE_ADAPTER_CUDA
//Disable treading support in our array handle
//needed for nvcc to stop complaining.
#define BOOST_SP_DISABLE_THREADS

#include "ArrayHandleExposed.h"

class PyFRContourData : public vtkDataObject
{
public:
  static PyFRContourData* New();
  vtkTypeMacro(PyFRContourData, vtkDataObject)

  typedef vtkm::cont::ArrayHandleExposed<vtkm::Vec<double,3> >
  Double3ArrayHandle;
  typedef vtkm::cont::ArrayHandleExposed<double> DoubleArrayHandle;

  Double3ArrayHandle Vertices;
  Double3ArrayHandle Normals;
  DoubleArrayHandle Density;
  DoubleArrayHandle Velocity_u;
  DoubleArrayHandle Velocity_v;
  DoubleArrayHandle Velocity_w;
  DoubleArrayHandle Pressure;

protected:
  PyFRContourData();
  virtual ~PyFRContourData();

};

vtkObject* NewPyFRContourData();

#endif
