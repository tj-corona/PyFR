#include <algorithm>
#include <iostream>
#include <iterator>
#include <sstream>
#include <stdexcept>
#include <vtkm/TopologyElementTag.h>
#include <vtkm/cont/ArrayHandle.h>
#include <vtkm/cont/DeviceAdapter.h>
#include <vtkm/cont/Field.h>
#include <vtkm/cont/DataSet.h>
#include <vtkm/cont/internal/DeviceAdapterTag.h>
#include <vtkm/cont/cuda/internal/DeviceAdapterTagCuda.h>
#include <vtkm/worklet/DispatcherMapField.h>
#include <vtkm/worklet/ExternalFaces.h>
#include <vtkm/worklet/WorkletMapField.h>
#include "FieldData.h"

FieldData::FieldData(void* field) :
  fld(static_cast<struct field*>(field)), types(NULL)
{
  const size_t xyz_len = fld->nel*3;
  const size_t con_len = fld->na*fld->nb;
  const size_t off_len = fld->nb*8;
  const size_t typ_len = fld->nb*8;
  namespace vm = vtkm::cont;
  const float* vbuf = static_cast<const float*>(fld->verts);
  vm::ArrayHandle<float> vert = vm::make_ArrayHandle(vbuf, xyz_len);
  vm::ArrayHandle<int32_t> connectivity =
    vm::make_ArrayHandle(fld->con, con_len);
  vm::ArrayHandle<int32_t> offsets = vm::make_ArrayHandle(fld->off, off_len);

  // VTKm requires the type array to be its vtkm::Id type, which is 64bit by
  // default but 32bit with a special compile option.  We use that option
  // anyway because our connectivity etc. is 32bit.
  // Upcast into a new 32bit array.
  this->types = new int32_t[typ_len];
  for(size_t i=0; i < typ_len; ++i)
    {
    this->types[i] = (int32_t)fld->type[i];
    }
  vm::ArrayHandle<int32_t> types = vm::make_ArrayHandle(this->types, typ_len);

  enum ElemType { CONSTANT=0, LINEAR=1, QUADRATIC=2 };
  vm::Field xyz("xyz", LINEAR, vm::Field::ASSOC_POINTS, vert);

  vm::CellSetExplicit<> cset(fld->nel, "cells", 3);
  cset.Fill(types, offsets, connectivity);

  this->ds.AddField(xyz);
  this->ds.AddCellSet(cset);
}

FieldData::~FieldData()
{
  delete[] this->types;
  this->types = NULL;
}

void FieldData::Update()
{
  std::cout << "iso step on " << this->n << " fields.\n";
  const size_t nbytes = fld->nel * sizeof(float);
  boost::shared_ptr<float> hsolution((float*)malloc(nbytes), free);
  if(hsolution == NULL)
    {
    throw std::runtime_error("alloc failure");
    }
  const cudaError_t err = cudaMemcpy(hsolution.get(), fld->solution, nbytes,
                                     cudaMemcpyDeviceToHost);
  if(cudaSuccess != err)
    {
    std::stringstream s; s << "memcpy failed: " << err;
    throw std::runtime_error(s.str().c_str());
    }

  namespace vm = vtkm::cont;
  enum ElemType { CONSTANT=0, LINEAR=1, QUADRATIC=2 };
  vm::Field rho("rho", LINEAR, vm::Field::ASSOC_POINTS, hsolution.get(),
                fld->nel);
  this->ds.AddField(rho);

  vm::CellSetExplicit<>& cset =
    this->ds.GetCellSet(0).CastTo<vtkm::cont::CellSetExplicit<> >();
  vm::ArrayHandle<vtkm::Id> shapes =
    cset.GetShapesArray(vtkm::TopologyElementTagPoint(),
                        vtkm::TopologyElementTagCell());
  vm::ArrayHandle<vtkm::Id> nindices =
    cset.GetNumIndicesArray(vtkm::TopologyElementTagPoint(),
                            vtkm::TopologyElementTagCell());
  vm::ArrayHandle<vtkm::Id> conn =
    cset.GetConnectivityArray(vtkm::TopologyElementTagPoint(),
                              vtkm::TopologyElementTagCell());
  vm::ArrayHandle<vtkm::Id> shapes_out;
  vm::ArrayHandle<vtkm::Id> nindices_out;
  vm::ArrayHandle<vtkm::Id> conn_out;

  vtkm::worklet::ExternalFaces<VTKM_DEFAULT_DEVICE_ADAPTER_TAG>().run(
    shapes, nindices, conn, shapes_out, nindices_out, conn_out
  );
  vm::DataSet outds;
  for(size_t i=0; i < (size_t)ds.GetNumberOfCoordinateSystems(); ++i)
    {
    outds.AddCoordinateSystem(ds.GetCoordinateSystem(i));
    }

  std::cout << shapes.GetNumberOfValues() << " input elements, "
            << shapes_out.GetNumberOfValues() << " output elements.\n";

  vm::CellSetExplicit<> outcset(shapes_out.GetNumberOfValues(),"cells");
  outcset.Fill(shapes_out, nindices_out, conn_out);
  outds.AddCellSet(outcset);
}
