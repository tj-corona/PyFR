#include "PyFRData.h"

#include <algorithm>
#include <iostream>
#include <iterator>
#include <sstream>
#include <stdexcept>

#include <vtkObjectFactory.h>

#include <vtkm/TopologyElementTag.h>
#include <vtkm/cont/ArrayHandle.h>
#include <vtkm/cont/ArrayHandleCast.h>
#include <vtkm/cont/DeviceAdapter.h>
#include <vtkm/cont/Field.h>
#include <vtkm/cont/DataSet.h>
#include <vtkm/cont/internal/DeviceAdapterTag.h>
#include <vtkm/cont/cuda/ArrayHandleCuda.h>
#include <vtkm/cont/cuda/internal/DeviceAdapterTagCuda.h>

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
  this->catalystData = static_cast<struct CatalystData*>(data);

  for (size_t cellType=0;cellType<this->catalystData->nCellTypes;cellType++)
    {
    MeshDataForCellType* meshData = &(this->catalystData->meshData[cellType]);
    SolutionDataForCellType* solutionData =
      &(this->catalystData->solutionData[cellType]);

    const size_t nFieldsPerVertex = 3;
    const size_t xyz_len = (meshData->nCells*
                            meshData->nVerticesPerCell*
                            nFieldsPerVertex);
    const size_t con_len = (meshData->nCells*
                            meshData->nVerticesPerCell);
    const size_t off_len = meshData->nCells;
    const size_t typ_len = meshData->nCells;
    const double* vbuf = meshData->vertices;

    vtkm::cont::ArrayHandle<double> vertices;
      {
      vtkm::cont::ArrayHandle<double> tmp =
        vtkm::cont::make_ArrayHandle(vbuf, xyz_len);
      vtkm::cont::DeviceAdapterAlgorithm<VTKM_DEFAULT_DEVICE_ADAPTER_TAG>().
        Copy(tmp , vertices);
      }

    vtkm::cont::ArrayHandle<vtkm::Id> connectivity;
      {
      vtkm::cont::ArrayHandle<int32_t> tmp =
        vtkm::cont::make_ArrayHandle(meshData->con, con_len);
      vtkm::cont::ArrayHandleCast<vtkm::Id,vtkm::cont::ArrayHandle<int32_t> > cast(tmp);
      vtkm::cont::DeviceAdapterAlgorithm<VTKM_DEFAULT_DEVICE_ADAPTER_TAG>().
        Copy(cast , connectivity);
      }

    vtkm::cont::ArrayHandle<vtkm::Id> offsets;
      {
      vtkm::cont::ArrayHandle<int32_t> tmp =
        vtkm::cont::make_ArrayHandle(meshData->off, off_len);
      vtkm::cont::ArrayHandleCast<vtkm::Id,vtkm::cont::ArrayHandle<int32_t> > cast(tmp);
      vtkm::cont::DeviceAdapterAlgorithm<VTKM_DEFAULT_DEVICE_ADAPTER_TAG>().
        Copy(cast , offsets);
      }

    vtkm::cont::ArrayHandle<vtkm::Id> types;
      {
      vtkm::cont::ArrayHandle<uint8_t> tmp =
        vtkm::cont::make_ArrayHandle(meshData->type, typ_len);
      vtkm::cont::ArrayHandleCast<vtkm::Id,vtkm::cont::ArrayHandle<uint8_t> > cast(tmp);
      vtkm::cont::DeviceAdapterAlgorithm<VTKM_DEFAULT_DEVICE_ADAPTER_TAG>().
        Copy(cast , types);
      }

    std::stringstream name; name << "xyz_" << cellType;
    enum ElemType { CONSTANT=0, LINEAR=1, QUADRATIC=2 };
    vtkm::cont::Field xyz(name.str(), LINEAR, vtkm::cont::Field::ASSOC_POINTS, vertices);

    name.clear();
    name.str("");
    name << "cells_" << cellType;
    vtkm::cont::CellSetExplicit<> cset(meshData->nSubdividedCells, name.str(), 3);
    cset.Fill(types, offsets, connectivity);

    vtkm::cont::cuda::ArrayHandleCuda<double>::type solution =
      vtkm::cont::cuda::make_ArrayHandle(
        static_cast<double*>(solutionData->solution),
        meshData->nSubdividedCells);

    name.clear();
    name.str("");
    name << "rho_" << cellType;
    vtkm::cont::Field rho(name.str(),LINEAR,vtkm::cont::Field::ASSOC_POINTS,solution);

    this->dataSet.AddField(xyz);
    this->dataSet.AddCellSet(cset);
    this->dataSet.AddField(rho);
    }
}

//------------------------------------------------------------------------------
void PyFRData::Update()
{
}
