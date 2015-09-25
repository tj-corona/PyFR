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

#include <vtkm/CellShape.h>
#include <vtkm/TopologyElementTag.h>
#include <vtkm/cont/ArrayHandle.h>
#include <vtkm/cont/ArrayHandleCast.h>
#include <vtkm/cont/CellSetSingleType.h>
#include <vtkm/cont/CoordinateSystem.h>
#include <vtkm/cont/DeviceAdapter.h>
#include <vtkm/cont/DataSet.h>
#include <vtkm/cont/Field.h>
#include <vtkm/cont/internal/DeviceAdapterTag.h>
#include <vtkm/cont/cuda/ArrayHandleCuda.h>
#include <vtkm/cont/cuda/internal/DeviceAdapterTagCuda.h>

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
void PyFRData::Init(void* data)
{
  this->catalystData = static_cast<struct CatalystData*>(data);

  // we only take data from the first stored cell type (i.e. hexahedra)
  MeshDataForCellType* meshData = &(this->catalystData->meshData[0]);
  SolutionDataForCellType* solutionData =
    &(this->catalystData->solutionData[0]);

  const size_t nFieldsPerVertex = 3;
  const size_t xyz_len = (meshData->nCells*
                          meshData->nVerticesPerCell*
                          nFieldsPerVertex);
  const size_t con_len = (meshData->nCells*
                          meshData->nVerticesPerCell);
  const size_t off_len = meshData->nCells;
  const size_t typ_len = meshData->nCells;
  const double* vbuf = meshData->vertices;

  vtkm::cont::ArrayHandle<vtkm::Vec<double,3> > vertices;
    {
    vtkm::cont::ArrayHandle<vtkm::Vec<double,3> > tmp = vtkm::cont::ArrayHandle<vtkm::Vec<double,3>,vtkm::cont::StorageTagBasic>(vtkm::cont::internal::Storage<vtkm::Vec<double,3>,vtkm::cont::StorageTagBasic>(reinterpret_cast<const vtkm::Vec<double,3>*>(vbuf),xyz_len));
    vtkm::cont::DeviceAdapterAlgorithm<VTKM_DEFAULT_DEVICE_ADAPTER_TAG>().
      Copy(tmp, vertices);
    }

  vtkm::cont::ArrayHandle<vtkm::Id> connectivity;
    {
    vtkm::cont::ArrayHandle<int32_t> tmp =
      vtkm::cont::make_ArrayHandle(meshData->con, con_len);
    vtkm::cont::ArrayHandleCast<vtkm::Id,
      vtkm::cont::ArrayHandle<int32_t> > cast(tmp);
    vtkm::cont::DeviceAdapterAlgorithm<VTKM_DEFAULT_DEVICE_ADAPTER_TAG>().
      Copy(cast, connectivity);
    }

  vtkm::cont::ArrayHandle<vtkm::UInt8> types;
    {
      std::vector<vtkm::UInt8> tmp(meshData->nCells,vtkm::CELL_SHAPE_HEXAHEDRON);
      vtkm::cont::ArrayHandle<vtkm::UInt8> tmp2 =
        vtkm::cont::make_ArrayHandle(tmp);
      vtkm::cont::DeviceAdapterAlgorithm<VTKM_DEFAULT_DEVICE_ADAPTER_TAG>().
        Copy(tmp2, types);
    }

  vtkm::cont::ArrayHandle<vtkm::Int32> nVertices;
    {
      std::vector<vtkm::Int32> tmp(meshData->nCells,8);
      vtkm::cont::ArrayHandle<vtkm::Int32> tmp2 =
        vtkm::cont::make_ArrayHandle(tmp);
      vtkm::cont::DeviceAdapterAlgorithm<VTKM_DEFAULT_DEVICE_ADAPTER_TAG>().
        Copy(tmp2, nVertices);
    }

  vtkm::cont::CellSetExplicit<> cset(meshData->nCells, "cells", 3);
  cset.Fill(types, nVertices, connectivity);

  // vtkm::cont::CellSetSingleType<> cset(vtkm::CellShapeTagHexahedron(),"cells");
  // cset.Fill(connectivity);

  vtkm::cont::cuda::ArrayHandleCuda<double>::type densityArray =
    vtkm::cont::cuda::make_ArrayHandle(
      static_cast<double*>(solutionData->solution),
      meshData->nCells*meshData->nVerticesPerCell);

  vtkm::cont::cuda::ArrayHandleCuda<double>::type velocity_uArray =
    vtkm::cont::cuda::make_ArrayHandle(
      static_cast<double*>(solutionData->solution +
                           1*meshData->nCells*meshData->nVerticesPerCell),
      meshData->nCells*meshData->nVerticesPerCell);

  vtkm::cont::cuda::ArrayHandleCuda<double>::type velocity_vArray =
    vtkm::cont::cuda::make_ArrayHandle(
      static_cast<double*>(solutionData->solution +
                           2*meshData->nCells*meshData->nVerticesPerCell),
      meshData->nCells*meshData->nVerticesPerCell);

  vtkm::cont::cuda::ArrayHandleCuda<double>::type velocity_wArray =
    vtkm::cont::cuda::make_ArrayHandle(
      static_cast<double*>(solutionData->solution +
                           3*meshData->nCells*meshData->nVerticesPerCell),
      meshData->nCells*meshData->nVerticesPerCell);

  vtkm::cont::cuda::ArrayHandleCuda<double>::type pressureArray =
    vtkm::cont::cuda::make_ArrayHandle(
      static_cast<double*>(solutionData->solution +
                           4*meshData->nCells*meshData->nVerticesPerCell),
      meshData->nCells*meshData->nVerticesPerCell);

  enum ElemType { CONSTANT=0, LINEAR=1, QUADRATIC=2 };
  vtkm::cont::Field density("density",LINEAR,vtkm::cont::Field::ASSOC_POINTS,densityArray);
  vtkm::cont::Field velocity_u("velocity_u",LINEAR,vtkm::cont::Field::ASSOC_POINTS,velocity_uArray);
  vtkm::cont::Field velocity_v("velocity_v",LINEAR,vtkm::cont::Field::ASSOC_POINTS,velocity_vArray);
  vtkm::cont::Field velocity_w("velocity_w",LINEAR,vtkm::cont::Field::ASSOC_POINTS,velocity_wArray);
  vtkm::cont::Field pressure("pressure",LINEAR,vtkm::cont::Field::ASSOC_POINTS,pressureArray);

  this->dataSet.AddCoordinateSystem(vtkm::cont::CoordinateSystem("coordinates",
                                                                 1,vertices));
  this->dataSet.AddField(density);
  this->dataSet.AddField(velocity_u);
  this->dataSet.AddField(velocity_v);
  this->dataSet.AddField(velocity_w);
  this->dataSet.AddField(pressure);
  this->dataSet.AddCellSet(cset);
}

//------------------------------------------------------------------------------
void PyFRData::Update()
{
}
