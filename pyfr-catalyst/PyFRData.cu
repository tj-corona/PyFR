#include "PyFRData.h"

#include <algorithm>
#include <iostream>
#include <iterator>
#include <sstream>
#include <stdexcept>

#include <vtkObjectFactory.h>

#include <vtkm/TopologyElementTag.h>
#include <vtkm/cont/ArrayHandle.h>
#include <vtkm/cont/DeviceAdapter.h>
#include <vtkm/cont/Field.h>
#include <vtkm/cont/DataSet.h>
#include <vtkm/cont/internal/DeviceAdapterTag.h>
#include <vtkm/cont/cuda/ArrayHandleCuda.h>
#include <vtkm/cont/cuda/internal/DeviceAdapterTagCuda.h>

vtkStandardNewMacro(PyFRData);

//------------------------------------------------------------------------------
PyFRData::PyFRData() : field(NULL), typesArray(NULL)
{

}

//------------------------------------------------------------------------------
PyFRData::~PyFRData()
{
  delete[] this->typesArray;
  this->typesArray = NULL;
}

//------------------------------------------------------------------------------
void PyFRData::Init(void* fld)
{
  this->field = static_cast<struct Field*>(fld);

  const size_t xyz_len = this->field->nel*3;
  const size_t con_len = this->field->na*field->nb;
  const size_t off_len = this->field->nb*8;
  const size_t typ_len = this->field->nb*8;
  const float* vbuf = static_cast<const float*>(field->verts);

  std::cout<<"are there "<<xyz_len<<" or "<<con_len<<" points?"<<std::endl;

    {
    size_t counter = 0;
    float xyz[this->field->na][3][this->field->nb];
    for (size_t i=0;i<this->field->na;i++)
      {
      for (size_t j=0;j<3;j++)
        {
        for (size_t k=0;k<this->field->nb;k++)
          {
          xyz[i][j][k] = vbuf[counter++];
          }
        }
      }

    counter = 0;

    for (size_t i=0;i<this->field->na;i++)
      {
      for (size_t k=0;k<this->field->nb;k++)
        {
        std::cout<<"point "<<counter++<<": [ ";
        for (size_t j=0;j<3;j++)
          {
          std::cout<<xyz[i][j][k]<<" ";
          }
        std::cout<<" ]"<<std::endl;
        }
      }
    }


  vtkm::cont::ArrayHandle<float> vert =
    vtkm::cont::make_ArrayHandle(vbuf, xyz_len);
  vtkm::cont::ArrayHandle<int32_t> connectivity =
    vtkm::cont::make_ArrayHandle(field->con, con_len);
  vtkm::cont::ArrayHandle<int32_t> offsets =
    vtkm::cont::make_ArrayHandle(field->off, off_len);

  // VTKm requires the type array to be its vtkm::Id type, which is 64bit by
  // default but 32bit with a special compile option.  We use that option
  // anyway because our connectivity etc. is 32bit.
  // Upcast into a new 32bit array.
  this->typesArray = new int32_t[typ_len];
  for(size_t i=0; i < typ_len; ++i)
    {
    this->typesArray[i] = (int32_t)field->type[i];
    }
  vtkm::cont::ArrayHandle<int32_t> types =
    vtkm::cont::make_ArrayHandle(this->typesArray, typ_len);

  enum ElemType { CONSTANT=0, LINEAR=1, QUADRATIC=2 };
  vtkm::cont::Field xyz("xyz", LINEAR, vtkm::cont::Field::ASSOC_POINTS, vert);

  vtkm::cont::CellSetExplicit<> cset(field->nel, "cells", 3);
  cset.Fill(types, offsets, connectivity);

  this->dataSet.AddField(xyz);
  this->dataSet.AddCellSet(cset);
}

//------------------------------------------------------------------------------
void PyFRData::Update()
{
  vtkm::cont::cuda::ArrayHandleCuda<float>::type solution =
    vtkm::cont::cuda::make_ArrayHandle(
      static_cast<float*>(this->field->solution),this->field->nel);

  enum ElemType { CONSTANT=0, LINEAR=1, QUADRATIC=2 };
  vtkm::cont::Field rho("rho",LINEAR,vtkm::cont::Field::ASSOC_POINTS,solution);
  this->dataSet.AddField(rho);
}
