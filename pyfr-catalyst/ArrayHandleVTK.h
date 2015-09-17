//=============================================================================
//
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//
//  Copyright 2015 Sandia Corporation.
//  Copyright 2015 UT-Battelle, LLC.
//  Copyright 2015 Los Alamos National Security.
//
//  Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
//  the U.S. Government retains certain rights in this software.
//  Under the terms of Contract DE-AC52-06NA25396 with Los Alamos National
//  Laboratory (LANL), the U.S. Government retains certain rights in
//  this software.
//
//=============================================================================
#ifndef vtk_m_cont_cuda_internal_ArrayHandleVTK_h
#define vtk_m_cont_cuda_internal_ArrayHandleVTK_h

#include <vtkm/cont/ArrayHandle.h>
#include <vtkm/cont/ErrorControlBadType.h>
#include <vtkm/cont/Storage.h>

namespace vtkm {
namespace cont {
namespace internal {

struct StorageTagVTK;

template<typename T>
class ArrayPortalVTK
{
public:
  typedef T ValueType;

  VTKM_CONT_EXPORT
  ArrayPortalVTK()
    : Data(NULL), NumberOfValues(0)
  {  }

  VTKM_CONT_EXPORT
  ArrayPortalVTK(ValueType* d, vtkm::Id numberOfValues)
    : Data(d), NumberOfValues(numberOfValues)
  {  }

  VTKM_CONT_EXPORT
  vtkm::Id GetNumberOfValues() const
  {
    return NumberOfValues;
  }

  VTKM_CONT_EXPORT
  ValueType Get(vtkm::Id index) const
  {
    throw vtkm::cont::ErrorControlBadType(
      "ArrayHandleVTK only provides access to the raw data pointer.");
  }

  VTKM_CONT_EXPORT
  ValueType* GetRawPointer() const
  {
    return Data;
  }
private:
  ValueType* Data;
  vtkm::Id NumberOfValues;
};

template<typename T, class StorageTag>
class Storage;

template<typename T>
class Storage<T, StorageTagVTK >
{
public:
  typedef T ValueType;
  typedef ArrayPortalVTK<T> PortalType;
  typedef ArrayPortalVTK<T> PortalConstType;

  VTKM_CONT_EXPORT
  Storage():
    Data(NULL), NumberOfValues(0)
  {
  }
  VTKM_CONT_EXPORT
  Storage(ValueType* d, vtkm::Id numberOfValues):
    Data(d), NumberOfValues(numberOfValues)
  {
  }

  VTKM_CONT_EXPORT
  PortalType GetPortal()
  {
    return PortalType(this->Data,this->NumberOfValues);
  }

  VTKM_CONT_EXPORT
  PortalConstType GetPortalConst() const
  {
    return PortalConstType(this->Data,this->NumberOfValues);
  }

  VTKM_CONT_EXPORT
  vtkm::Id GetNumberOfValues() const
  {
    return this->NumberOfValues;
  }

  VTKM_CONT_EXPORT
  void Allocate(vtkm::Id size)
  {
    cudaMalloc((void**)&Data, size*sizeof(ValueType));
    NumberOfValues = size;
  }

  VTKM_CONT_EXPORT
  void Shrink(vtkm::Id numberOfValues)
  {
    VTKM_ASSERT_CONT(numberOfValues <= static_cast<vtkm::Id>(this->GetNumberOfValues()));

    this->NumberOfValues = numberOfValues;
    if (numberOfValues == 0)
      this->ReleaseResources();
  }

  VTKM_CONT_EXPORT
  void ReleaseResources()
  {
    cudaFree((void**)&Data);
    NumberOfValues = 0;
  }

private:
  ValueType* Data;
  vtkm::Id NumberOfValues;
};

} //namespace internal
} //namespace cont
} //namespace vtkm

namespace vtkm {
namespace cont {

template<typename T>
class ArrayHandleVTK
  : public vtkm::cont::ArrayHandle<T,internal::StorageTagVTK>
{
public:
  typedef T ValueType;
  typedef internal::StorageTagVTK StorageTag;
  typedef internal::Storage<ValueType, StorageTag> StorageType;


  typedef vtkm::cont::ArrayHandle<ValueType, StorageTag> Superclass;

  ArrayHandleVTK()
    : Superclass() {  }

  ArrayHandleVTK(ValueType* d, vtkm::Id numberOfValues)
    : Superclass(StorageType(d,numberOfValues)) {  }
};

} //namespace cont
} //namespace vtkm

#endif //vtk_m_cont_cuda_internal_ArrayHandleVTK_h
