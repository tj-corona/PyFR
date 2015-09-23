#include "PyFRData.h"

#include <algorithm>
#include <cassert>
#include <iostream>
#include <iterator>
#include <limits>
#include <sstream>
#include <stdexcept>

#include <vtkDataObjectTypes.h>
#include <vtkObjectFactory.h>

#include <vtkm/TopologyElementTag.h>
#include <vtkm/cont/ArrayHandle.h>
#include <vtkm/cont/ArrayHandleCast.h>
#include <vtkm/cont/CoordinateSystem.h>
#include <vtkm/cont/DeviceAdapter.h>
#include <vtkm/cont/DataSet.h>
#include <vtkm/cont/Field.h>
#include <vtkm/cont/internal/DeviceAdapterTag.h>
#include <vtkm/cont/cuda/ArrayHandleCuda.h>
#include <vtkm/cont/cuda/internal/DeviceAdapterTagCuda.h>

namespace
{
  typedef vtkm::Id Id;
  typedef vtkm::Id3 Id3;
  typedef vtkm::Vec<double,3> Double3;
  typedef vtkm::cont::ArrayHandle<Double3> Double3ArrayHandle;
  struct GridData
  {
    Id3 Dimension;
    Double3 Origin;
    Double3 Spacing;
  };

  GridData ComputeGridDimensions(Double3ArrayHandle&);
};

//------------------------------------------------------------------------------
vtkStandardNewMacro(PyFRData);

//------------------------------------------------------------------------------
PyFRData::PyFRData() : catalystData(NULL)
{

}

//------------------------------------------------------------------------------
PyFRData::~PyFRData()
{
}

//------------------------------------------------------------------------------
void PyFRData::Init(vtkIdType datasettypeid, void* data)
{
  this->dataSetTypeId = datasettypeid;
  assert(this->dataSetTypeId = vtkDataObjectTypes::GetTypeIdFromClassName("vtkStructuredGrid"));

  this->catalystData = static_cast<struct CatalystData*>(data);

  MeshDataForCellType* meshData = &(this->catalystData->meshData[0]);
  SolutionDataForCellType* solutionData= &(this->catalystData->solutionData[0]);

  const size_t xyz_len = (meshData->nCells*
                          meshData->nVerticesPerCell);
  const double* vbuf = meshData->vertices;

  vtkm::cont::ArrayHandle<vtkm::Vec<double,3> > vertices =
    vtkm::cont::ArrayHandle<vtkm::Vec<double,3>,vtkm::cont::StorageTagBasic>(vtkm::cont::internal::Storage<vtkm::Vec<double,3>,vtkm::cont::StorageTagBasic>(reinterpret_cast<const vtkm::Vec<double,3>*>(vbuf),xyz_len));
  GridData gridData = ComputeGridDimensions(vertices);
  // std::cout<<"Grid data:"<<std::endl;
  // std::cout<<"  dimension: "<<gridData.Dimension<<std::endl;
  // std::cout<<"  origin:    "<<gridData.Origin<<std::endl;
  // std::cout<<"  spacing:   "<<gridData.Spacing<<std::endl;

  for (int i=0;i<3;i++)
    this->cellDimension[i] = gridData.Dimension[i] - 1;

  vtkm::cont::CellSetStructured<3> cset("cells");
  cset.SetPointDimensions(gridData.Dimension);

  vtkm::cont::cuda::ArrayHandleCuda<double>::type solution =
    vtkm::cont::cuda::make_ArrayHandle(
      static_cast<double*>(solutionData->solution),
      meshData->nCells*meshData->nVerticesPerCell);

  enum ElemType { CONSTANT=0, LINEAR=1, QUADRATIC=2 };
  vtkm::cont::Field rho("rho",LINEAR,vtkm::cont::Field::ASSOC_POINTS,solution);

  this->dataSet.AddCoordinateSystem(
    vtkm::cont::CoordinateSystem("coordinates",
                                 1,
                                 gridData.Dimension,
                                 gridData.Origin,
                                 gridData.Spacing));
  this->dataSet.AddField(rho);
  this->dataSet.AddCellSet(cset);
}

//------------------------------------------------------------------------------
void PyFRData::Update()
{
}

//----------------------------------------------------------------------------
namespace
{
GridData ComputeGridDimensions(Double3ArrayHandle& ptsArray)
{
  const double epsilon = 1.e-6;

  Double3ArrayHandle::PortalConstControl points =
    ptsArray.GetPortalConstControl();

  Id3 xyz;
  xyz[0] = xyz[1] = xyz[2] = 0;
  Double3 sentinel = points.Get(0);
  Double3 spacing;
  for (Id i=0;i<3;i++) spacing[i] = std::numeric_limits<double>::max();
  Double3 point;
  Id counter = 0;
  Id increment = 1;
  Id3 indexing;
  indexing[0] = indexing[1] = indexing[2] = -1;

  while (counter < ptsArray.GetNumberOfValues())
    {
    counter += increment;
    point = points.Get(counter);

    for (Id i=0;i<3;i++)
      {
      if (fabs(point[i]-sentinel[i]) > epsilon &&
          fabs(point[i]-sentinel[i]) < spacing[i])
        spacing[i] = fabs(point[i]-sentinel[i]);
      }

    if (indexing[0] == -1)
      {
      for (Id i=0;i<3;i++)
        {
        if (fabs(point[i]-sentinel[i]) > epsilon)
          {
          for (Id j=0;j<3;j++)
            indexing[j] = (i+j)%3;
          }
        }
      }

    if (xyz[indexing[0]] == 0)
      {
      if (fabs(point[indexing[0]]-sentinel[indexing[0]]) < epsilon)
        {
        xyz[indexing[0]] = increment = counter;
        point = points.Get(counter+increment);
        for (Id i=0;i<3;i++)
          {
          if (i == indexing[0])
            continue;
          if (fabs(point[i]-sentinel[i])>epsilon)
            {
            indexing[1] = i;
            for (Id j=0;j<3;j++)
              {
              if (j != indexing[0] && j != indexing[1])
                indexing[2] = j;
              }
            }
          }
        point = points.Get(counter);
        }
      }

    if (xyz[indexing[0]] != 0 && xyz[indexing[1]] == 0)
      {
      if (fabs(point[indexing[1]]-sentinel[indexing[1]]) < epsilon)
        {
        xyz[indexing[1]] = counter/xyz[indexing[0]];
        increment = counter;
        }
      }

    if (xyz[indexing[1]] != 0 && xyz[indexing[2]] == 0)
      {
      if (fabs(point[indexing[2]]-sentinel[indexing[2]]) < epsilon)
        {
        xyz[indexing[2]] = counter/xyz[indexing[0]]/xyz[indexing[1]];
        break;
        }
      }
    }
  if (xyz[indexing[2]] == 0)
    xyz[indexing[2]] = ptsArray.GetNumberOfValues()/xyz[indexing[0]]/xyz[indexing[1]];

  GridData gridData;
  gridData.Dimension = xyz;
  gridData.Origin = points.Get(xyz[0]*xyz[1]*xyz[2]*3); // ???
  gridData.Spacing = spacing;

  return gridData;
}
}
