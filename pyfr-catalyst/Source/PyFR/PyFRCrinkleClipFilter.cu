#include "PyFRCrinkleClipFilter.h"

#include <string>
#include <vector>

#include <vtkm/BinaryPredicates.h>
#include <vtkm/ImplicitFunctions.h>
#include <vtkm/cont/cuda/DeviceAdapterCuda.h>

#include <vtkPlane.h>
#include <vtkSphere.h>

#include "CrinkleClip.h"
#include "PyFRData.h"

void PyFRCrinkleClipFilter::operator ()(PyFRData* inputData,PyFRData* outputData,vtkSphere* sphere) const
{
  typedef ::vtkm::cont::DeviceAdapterTagCuda CudaTag;
  typedef vtkm::worklet::CrinkleClip<CudaTag> CrinkleClip;
  typedef PyFRData::Vec3ArrayHandle CoordinateArrayHandle;
  typedef vtkm::ListTagBase<PyFRData::CellSet> CellSetTag;
  typedef vtkm::Sphere ImplicitFunction;

ImplicitFunction func(vtkm::Vec<FPType,3>(sphere->GetCenter()[0],
                                          sphere->GetCenter()[1],
                                          sphere->GetCenter()[2]),
                      sphere->GetRadius());

  const vtkm::cont::DataSet& input = inputData->GetDataSet();
  vtkm::cont::DataSet& output = outputData->GetDataSet();

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

void PyFRCrinkleClipFilter::operator ()(PyFRData* inputData,PyFRData* outputData,vtkPlane* plane) const
{
  typedef ::vtkm::cont::DeviceAdapterTagCuda CudaTag;
  typedef vtkm::worklet::CrinkleClip<CudaTag> CrinkleClip;
  typedef PyFRData::Vec3ArrayHandle CoordinateArrayHandle;
  typedef vtkm::ListTagBase<PyFRData::CellSet> CellSetTag;
  typedef vtkm::Plane ImplicitFunction;

  ImplicitFunction func(vtkm::Vec<FPType,3>(plane->GetOrigin()[0],
                                            plane->GetOrigin()[1],
                                            plane->GetOrigin()[2]),
                        vtkm::Vec<FPType,3>(plane->GetNormal()[0],
                                            plane->GetNormal()[1],
                                            plane->GetNormal()[2]));

  const vtkm::cont::DataSet& input = inputData->GetDataSet();
  vtkm::cont::DataSet& output = outputData->GetDataSet();

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
