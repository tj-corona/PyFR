#include "PyFRParallelSliceFilter.h"

#include <vtkm/ImplicitFunctions.h>
#include <vtkm/cont/cuda/DeviceAdapterCuda.h>

#include "CrinkleClip.h"
#include "IsosurfaceHexahedra.h"
#include "PyFRData.h"
#include "PyFRContourData.h"

//----------------------------------------------------------------------------
PyFRParallelSliceFilter::PyFRParallelSliceFilter() : NPlanes(1), Spacing(1.)
{
  this->Origin[0] = this->Origin[1] = this->Origin[2] = 0.;
  this->Normal[0] = this->Normal[1] = 0.;
  this->Normal[2] = 1.;
}

//----------------------------------------------------------------------------
PyFRParallelSliceFilter::~PyFRParallelSliceFilter()
{
}

//----------------------------------------------------------------------------
void PyFRParallelSliceFilter::SetPlane(const FPType* origin,
                                       const FPType* normal)
{
  for (unsigned i=0;i<3;i++)
    {
    this->Origin[i] = origin[i];
    this->Normal[i] = normal[i];
    }
}

//----------------------------------------------------------------------------
void PyFRParallelSliceFilter::operator()(PyFRData* input,
                                         PyFRContourData* output) const
{
  typedef ::vtkm::cont::DeviceAdapterTagCuda CudaTag;
  typedef PyFRData::Vec3ArrayHandle CoordinateArrayHandle;
  typedef vtkm::worklet::IsosurfaceFilterHexahedra<FPType,CudaTag>
    IsosurfaceFilter;
  typedef std::vector<vtkm::cont::ArrayHandle<vtkm::Vec<FPType,3> > >
    Vec3HandleVec;
  typedef std::vector<vtkm::cont::ArrayHandle<FPType> > ScalarDataHandleVec;
  typedef std::vector<FPType> DataVec;
  typedef vtkm::worklet::CrinkleClipTraits<typename PyFRData::CellSet>::CellSet CellSet;

  const vtkm::cont::DataSet& dataSet = input->GetDataSet();

  CoordinateArrayHandle coords = dataSet.GetCoordinateSystem().GetData()
    .CastToArrayHandle(CoordinateArrayHandle::ValueType(),
                       CoordinateArrayHandle::StorageTag());

  vtkm::Plane func(vtkm::Vec<FPType,3>(this->Origin[0],
                                       this->Origin[1],
                                       this->Origin[2]),
                   vtkm::Vec<FPType,3>(this->Normal[0],
                                       this->Normal[1],
                                       this->Normal[2]));

  vtkm::ImplicitFunctionValue<vtkm::Plane> function(func);

  vtkm::cont::ArrayHandleTransform<FPType,CoordinateArrayHandle,
    vtkm::ImplicitFunctionValue<vtkm::Plane> > dataArray(coords,function);

  DataVec dataVec;
  Vec3HandleVec verticesVec;
  Vec3HandleVec normalsVec;
  output->SetNumberOfContours(this->NPlanes);
  for (unsigned i=0;i<output->GetNumberOfContours();i++)
    {
    dataVec.push_back(i*this->Spacing);
    verticesVec.push_back(output->GetContour(i).GetVertices());
    normalsVec.push_back(output->GetContour(i).GetNormals());
    }

  IsosurfaceFilter isosurfaceFilter;
  isosurfaceFilter.Run(dataVec,
                       dataSet.GetCellSet().CastTo(CellSet()),
                       dataSet.GetCoordinateSystem(),
                       dataArray,
                       verticesVec,
                       normalsVec);

  std::string fields[5] = {"density",
                           "pressure",
                           "velocity_u",
                           "velocity_v",
                           "velocity_w"};

  for (unsigned i=0;i<5;i++)
    {
    ScalarDataHandleVec scalarDataHandleVec;
    for (unsigned j=0;j<output->GetNumberOfContours();j++)
      {
      PyFRContour::ScalarDataArrayHandle scalars_out =
        output->GetContour(j).GetScalarData(fields[i]);
      scalarDataHandleVec.push_back(scalars_out);
      }
    vtkm::cont::Field projectedField = dataSet.GetField(fields[i]);

    PyFRData::ScalarDataArrayHandle projectedArray = projectedField.GetData()
      .CastToArrayHandle(PyFRData::ScalarDataArrayHandle::ValueType(),
                         PyFRData::ScalarDataArrayHandle::StorageTag());

    isosurfaceFilter.MapFieldOntoIsosurfaces(projectedArray,
                                             scalarDataHandleVec);
    }
}
