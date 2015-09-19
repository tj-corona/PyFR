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
#ifndef vtk_m_ArrayHandleVTK_h
#define vtk_m_ArrayHandleVTK_h

#include <vtkm/cont/ArrayHandle.h>
#include <vtkm/cont/Storage.h>

namespace vtkm {
namespace cont {

template<typename T, typename StorageTag_ = VTKM_DEFAULT_STORAGE_TAG>
class ArrayHandleVTK : public ArrayHandle<T,StorageTag_>
{
public:
  typedef vtkm::cont::internal::Storage<T,StorageTag_> StorageType;

  StorageType& Storage()
  {
    return this->Internals->ControlArray;
  }
};

} //namespace cont
} //namespace vtkm

#endif //vtk_m_ArrayHandleVTK_h
