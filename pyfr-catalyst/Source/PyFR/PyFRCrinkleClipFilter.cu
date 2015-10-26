#include "PyFRCrinkleClipFilter.h"

#include <string>
#include <vector>

#include <vtkm/BinaryPredicates.h>
#include <vtkm/ImplicitFunctions.h>
#include <vtkm/cont/cuda/DeviceAdapterCuda.h>

#include "CrinkleClip.h"
#include "PyFRData.h"

void PyFRCrinkleClipFilter::operator ()(PyFRData* inputData,PyFRData* outputData,const FPType* origin,const FPType* normal) const
{
  typedef ::vtkm::cont::DeviceAdapterTagCuda CudaTag;
  typedef vtkm::worklet::CrinkleClip<CudaTag> CrinkleClip;
  typedef PyFRData::Vec3ArrayHandle CoordinateArrayHandle;
  typedef vtkm::ListTagBase<PyFRData::CellSet> CellSetTag;
  typedef vtkm::Plane ImplicitFunction;

  ImplicitFunction func(vtkm::Vec<FPType,3>(origin[0],
                                            origin[1],
                                            origin[2]),
                        vtkm::Vec<FPType,3>(normal[0],
                                            normal[1],
                                            normal[2]));

  const vtkm::cont::DataSet& input = inputData->GetDataSet();
  vtkm::cont::DataSet& output = outputData->GetDataSet();
  output.Clear();

  CoordinateArrayHandle coords = input.GetCoordinateSystem().GetData()
    .CastToArrayHandle(CoordinateArrayHandle::ValueType(),
                       CoordinateArrayHandle::StorageTag());

  vtkm::ImplicitFunctionValue<ImplicitFunction> function(func);

  vtkm::cont::ArrayHandleTransform<FPType,CoordinateArrayHandle,
    vtkm::ImplicitFunctionValue<ImplicitFunction> > dataArray(coords,function);

  vtkm::cont::ArrayHandleConstant<FPType> clipArray(0.,coords.GetNumberOfValues());

  CrinkleClip crinkleClip;

  crinkleClip.Run(dataArray,
                  clipArray,
                  vtkm::SortLess(),
                  input.GetCellSet().ResetCellSetList(CellSetTag()),
                  input.GetCoordinateSystem(),
                  output);

  for (vtkm::IdComponent i=0;i<input.GetNumberOfFields();i++)
    output.AddField(input.GetField(i));
}
