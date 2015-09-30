#ifndef PYFRDATA_H
#define PYFRDATA_H

#include <vtkDataObject.h>

//State that the default backend for this code is CUDA
//not serial
#define VTKM_DEVICE_ADAPTER VTKM_DEVICE_ADAPTER_CUDA
//Disable treading support in our array handle
//needed for nvcc to stop complaining.
#define BOOST_SP_DISABLE_THREADS

#include <vtkm/cont/ArrayHandle.h>
#include <vtkm/cont/ArrayHandleCast.h>
#include <vtkm/cont/ArrayHandleImplicit.h>
#include <vtkm/cont/ArrayHandlePermutation.h>
#include <vtkm/cont/cuda/ArrayHandleCuda.h>
#include <vtkm/cont/DataSet.h>

#include "CatalystData.h"

struct StridedDataFunctor
{
  vtkm::Id NumberOfCells;
  vtkm::Id NVerticesPerCell;
  vtkm::Id NSolutionTypes;
  vtkm::Id SolutionType;
  vtkm::Id CellStride;
  vtkm::Id VertexStride;

  VTKM_EXEC_CONT_EXPORT
  vtkm::Id operator()(vtkm::Id index) const
  {
    vtkm::Id cell = index/NVerticesPerCell;
    vtkm::Id vertex = index%NVerticesPerCell;

    return cell + CellStride*SolutionType + VertexStride*vertex;
  }
};

/*
 * A VTK-digestable representation of PyFR output data.
 *
 * This class was adapted from the Isosurface class from Tom Fogal's
 * visualization plugin.
 */
class PyFRData : public vtkDataObject
{
public:
  typedef vtkm::cont::ArrayHandle<vtkm::Vec<double,3> > Vec3ArrayHandle;

  typedef vtkm::cont::ArrayHandleImplicit<vtkm::Id, StridedDataFunctor>
  DataIndexArrayType;

  typedef vtkm::cont::cuda::ArrayHandleCuda<double>::type RawDataArrayType;

  typedef vtkm::cont::ArrayHandlePermutation<DataIndexArrayType,
    RawDataArrayType> DataArrayType;

  typedef vtkm::cont::ArrayHandle<
    typename RawDataArrayType::ValueType,
    vtkm::cont::internal::StorageTagPermutation<DataIndexArrayType, RawDataArrayType> > DataArrayParentType;

  static PyFRData* New();
  vtkTypeMacro(PyFRData, vtkDataObject)

  void Init(void* field);

  vtkm::cont::DataSet& GetDataSet() { return dataSet; }

  void Update();

protected:
  PyFRData();
  virtual ~PyFRData();

private:
  struct CatalystData* catalystData;
  vtkm::cont::DataSet dataSet;
};
#endif
