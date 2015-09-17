#ifndef PYFRDATA_H
#define PYFRDATA_H

#include <vtkDataObject.h>

//State that the default backend for this code is CUDA
//not serial
#define VTKM_DEVICE_ADAPTER VTKM_DEVICE_ADAPTER_CUDA
//Disable treading support in our array handle
//needed for nvcc to stop complaining.
#define BOOST_SP_DISABLE_THREADS

#include <vtkm/cont/DataSet.h>
#include "Field.h"

/*
 * A VTK-digestable representation of PyFR output data.
 *
 * This class was adapted from the Isosurface class from Tom Fogal's
 * visualization plugin.
 */
class PyFRData : public vtkDataObject
{
public:
  static PyFRData* New();
  vtkTypeMacro(PyFRData, vtkDataObject)

  void Init(void* field);

  vtkm::cont::DataSet& GetDataSet() { return dataSet; }

  void Update();

protected:
  PyFRData();
  virtual ~PyFRData();

private:
  struct Field* field;
  int32_t* typesArray;
  vtkm::cont::DataSet dataSet;
};
#endif
