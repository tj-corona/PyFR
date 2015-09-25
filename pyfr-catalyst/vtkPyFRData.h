#ifndef PYFRDATA_H
#define PYFRDATA_H

#include <vtkDataObject.h>

//State that the default backend for this code is CUDA
//not serial
#define VTKM_DEVICE_ADAPTER VTKM_DEVICE_ADAPTER_CUDA
//Disable treading support in our array handle
//needed for nvcc to stop complaining.
#define BOOST_SP_DISABLE_THREADS

#include "ArrayHandleExposed.h"

/*
 * A VTK-digestable representation of PyFR output data.
 */
class vtkPyFRData : public vtkDataObject
{
public:
  static vtkPyFRData* New();
  vtkTypeMacro(vtkPyFRData, vtkDataObject)

  typedef vtkm::cont::ArrayHandleExposed<vtkm::Vec<double,3> >
  Double3ArrayHandleExposed;
  typedef vtkmc::ArrayHandleExposed<double> DoubleArrayHandleExposed;

  Double3ArrayHandleExposed vertices;
  Double3ArrayHandleExposed normals;
  DoubleArrayHandleExposed density;
  DoubleArrayHandleExposed velocity_u;
  DoubleArrayHandleExposed velocity_v;
  DoubleArrayHandleExposed velocity_w;
  DoubleArrayHandleExposed pressure;

protected:
  PyFRData();
  virtual ~PyFRData();

private:
  struct CatalystData* catalystData;
  vtkm::cont::DataSet dataSet;
};
#endif
