//============================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//
//  Copyright 2014 Sandia Corporation.
//  Copyright 2014 UT-Battelle, LLC.
//  Copyright 2014 Los Alamos National Security.
//
//  Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
//  the U.S. Government retains certain rights in this software.
//
//  Under the terms of Contract DE-AC52-06NA25396 with Los Alamos National
//  Laboratory (LANL), the U.S. Government retains certain rights in
//  this software.
//============================================================================

#ifndef vtk_m_worklet_IsosurfaceHexahedra_h
#define vtk_m_worklet_IsosurfaceHexahedra_h

#include <stdlib.h>
#include <stdio.h>

#include <vtkm/cont/DeviceAdapter.h>
#include <vtkm/cont/ArrayHandle.h>
#include <vtkm/cont/ArrayHandleCounting.h>
#include <vtkm/cont/ArrayHandlePermutation.h>
#include <vtkm/cont/CellSetExplicit.h>
#include <vtkm/cont/DynamicArrayHandle.h>
#include <vtkm/worklet/DispatcherMapField.h>
#include <vtkm/worklet/WorkletMapField.h>
#include <vtkm/Pair.h>

#include <vtkm/cont/CellSetPermutation.h>
#include <vtkm/cont/DataSet.h>
#include <vtkm/cont/Field.h>
#include <vtkm/worklet/DispatcherMapTopology.h>
#include <vtkm/worklet/WorkletMapTopology.h>
#include <vtkm/VectorAnalysis.h>

#include <vtkm/worklet/MarchingCubesDataTables.h>

namespace vtkm {
namespace worklet {

/// \brief Compute the isosurface for a uniform grid data set
template <typename FieldType, typename DeviceAdapter>
class IsosurfaceFilterHexahedra
{
protected:
  vtkm::cont::DataSet DataSet;
  vtkm::cont::ArrayHandle<FieldType> InterpolationWeight;
  vtkm::cont::ArrayHandle<vtkm::Id> InterpolationLowId;
  vtkm::cont::ArrayHandle<vtkm::Id> InterpolationHighId;
  vtkm::Id NumOutputCells;

public:

  class ClassifyCell : public vtkm::worklet::WorkletMapTopologyPointToCell
  {
  public:
    typedef void ControlSignature(FieldInFrom<Scalar> scalars,
                                  TopologyIn topology,
                                  FieldOut<IdType> numVertices);
    typedef void ExecutionSignature(_1, FromCount, _3);
    typedef _2 InputDomain;

    typedef vtkm::cont::ArrayHandle<vtkm::Id> IdArrayHandle;
    typedef typename IdArrayHandle::ExecutionTypes<DeviceAdapter>::PortalConst IdPortalType;
    IdPortalType VertexTable;
    FieldType Isovalue;

    VTKM_CONT_EXPORT
    ClassifyCell(IdPortalType vertexTable, FieldType isovalue) :
      VertexTable(vertexTable),
      Isovalue(isovalue)
    {
    }

    template<typename ScalarsVecType>
    VTKM_EXEC_EXPORT
    void operator()(const ScalarsVecType &scalars,
                    const vtkm::Id count,
                    vtkm::Id& numVertices) const
    {
      const vtkm::Id mask[] = { 1, 2, 4, 8, 16, 32, 64, 128 };

      vtkm::Id caseId = 0;
      for (vtkm::IdComponent i = 0; i < count; ++i)
        caseId += (static_cast<FieldType>(scalars[i]) > this->Isovalue)*mask[i];
      numVertices = this->VertexTable.Get(caseId) / 3;
    }
  };

  /// \brief Compute isosurface vertices and scalars
  class IsoSurfaceGenerate : public vtkm::worklet::WorkletMapTopologyPointToCell
  {
  public:
    typedef void ControlSignature(FieldInFrom<Scalar> scalars,
                                  FieldInFrom<Vec3> coordinates,
                                  FieldInTo<IdType> inputLowerBounds,
                                  TopologyIn topology);
    typedef void ExecutionSignature(WorkIndex, FromCount, _1, _2, _3, FromIndices);
    typedef _4 InputDomain;

    const FieldType Isovalue;

    typedef typename vtkm::cont::ArrayHandle<FieldType>::template ExecutionTypes<DeviceAdapter>::Portal ScalarPortalType;
    ScalarPortalType InterpolationWeight;

    typedef typename vtkm::cont::ArrayHandle<vtkm::Id> IdArrayHandle;
    typedef typename IdArrayHandle::ExecutionTypes<DeviceAdapter>::PortalConst IdPortalConstType;
    IdPortalConstType TriTable;
    typedef typename IdArrayHandle::ExecutionTypes<DeviceAdapter>::Portal IdPortalType;
    IdPortalType InterpolationLowId;
    IdPortalType InterpolationHighId;

    typedef typename vtkm::cont::ArrayHandle<vtkm::Vec<FieldType, 3> >::template ExecutionTypes<DeviceAdapter>::Portal VectorPortalType;
    VectorPortalType Vertices;
    VectorPortalType Normals;

    template<typename V>
    VTKM_CONT_EXPORT
    IsoSurfaceGenerate(const FieldType ivalue,
                       IdPortalConstType triTablePortal,
                       ScalarPortalType interpolationWeight,
                       IdPortalType interpolationLowId,
                       IdPortalType interpolationHighId,
                       const V &vertices,
                       const V &normals) :
      Isovalue(ivalue),
      InterpolationWeight(interpolationWeight),
      TriTable(triTablePortal),
      InterpolationLowId(interpolationLowId),
      InterpolationHighId(interpolationHighId),
      Vertices(vertices),
      Normals(normals)
    {
    }

    template<typename ScalarsVecType, typename VectorsVecType, typename IdVecType>
    VTKM_EXEC_EXPORT
    void operator()(const vtkm::Id outputCellId,
                    const vtkm::Id count,
                    const ScalarsVecType &scalars,
                    const VectorsVecType &pointCoords,
                    const vtkm::Id inputLowerBounds,
                    const IdVecType &pointIds) const
    {
      // Get data for this cell
      const int verticesForEdge[] = { 0, 1, 1, 2, 3, 2, 0, 3,
                                      4, 5, 5, 6, 7, 6, 4, 7,
                                      0, 4, 1, 5, 2, 6, 3, 7 };

      // Compute the Marching Cubes case number for this cell
      unsigned int cubeindex = 0;
      const vtkm::Id mask[] = { 1, 2, 4, 8, 16, 32, 64, 128 };
      for (vtkm::IdComponent i = 0; i < count; ++i)
        cubeindex += (static_cast<FieldType>(scalars[i]) > this->Isovalue)*mask[i];

      // Interpolate for vertex positions and associated scalar values
      const vtkm::Id inputIteration = (outputCellId - inputLowerBounds);
      const vtkm::Id outputVertId = outputCellId * 3;
      const vtkm::Id cellOffset = (static_cast<vtkm::Id>(cubeindex*16) +
                                   (inputIteration * 3));

      for (int v = 0; v < 3; v++)
      {
        const vtkm::Id edge = this->TriTable.Get(cellOffset + v);
        const int v0   = verticesForEdge[2*edge];
        const int v1   = verticesForEdge[2*edge + 1];
        const FieldType t  = (this->Isovalue - scalars[v0]) / (scalars[v1] - scalars[v0]);
        this->Vertices.Set(outputVertId + v,
                           vtkm::Lerp(pointCoords[v0], pointCoords[v1], t));
        this->InterpolationWeight.Set(outputVertId + v, t);
        this->InterpolationLowId.Set(outputVertId + v, pointIds[v0]);
        this->InterpolationHighId.Set(outputVertId + v, pointIds[v1]);
      }
      vtkm::Vec<FieldType, 3> vertex0 = this->Vertices.Get(outputVertId + 0);
      vtkm::Vec<FieldType, 3> vertex1 = this->Vertices.Get(outputVertId + 1);
      vtkm::Vec<FieldType, 3> vertex2 = this->Vertices.Get(outputVertId + 2);

      vtkm::Vec<FieldType, 3> curNorm = vtkm::Cross(vertex1-vertex0,
                                                    vertex2-vertex0);
      vtkm::Normalize(curNorm);
      this->Normals.Set(outputVertId + 0, curNorm);
      this->Normals.Set(outputVertId + 1, curNorm);
      this->Normals.Set(outputVertId + 2, curNorm);
    }
  };

  template <typename Field>
  class ApplyToField : public vtkm::worklet::WorkletMapField
  {
  public:
    typedef void ControlSignature(FieldIn<Scalar> interpolationLow,
                                  FieldIn<Scalar> interpolationHigh,
                                  FieldIn<Scalar> interpolationWeight,
                                  FieldOut<Scalar> interpolatedOutput);
    typedef void ExecutionSignature(_1, _2, _3, _4);
    typedef _1 InputDomain;

    VTKM_CONT_EXPORT
    ApplyToField()
    {
    }

    VTKM_EXEC_EXPORT
    void operator()(const Field &low,
                    const Field &high,
                    const FieldType &weight,
                    Field &result) const
    {
      result = vtkm::Lerp(low, high, weight);
    }
  };

  IsosurfaceFilterHexahedra(const vtkm::cont::DataSet &dataSet) :
    DataSet(dataSet),
    NumOutputCells(0)
  {
  }

  template<typename CoordinateType>
  void Run(const FieldType &isovalue,
           const vtkm::cont::DynamicArrayHandle& isoField,
           vtkm::cont::ArrayHandle< vtkm::Vec<CoordinateType,3> >& verticesArray,
           vtkm::cont::ArrayHandle< vtkm::Vec<CoordinateType,3> >& normalsArray)
  {
    //todo this needs to change so that we don't presume the storage type
    vtkm::cont::ArrayHandle<FieldType> field;
    field = isoField.CastToArrayHandle(FieldType(), VTKM_DEFAULT_STORAGE_TAG());
    this->Run(isovalue, field, verticesArray, normalsArray);
  }

  template<typename StorageTag, typename CoordinateType>
  void Run(const FieldType &isovalue,
           const vtkm::cont::ArrayHandle<FieldType,StorageTag>& isoField,
           vtkm::cont::ArrayHandle< vtkm::Vec<CoordinateType,3> >& verticesArray,
           vtkm::cont::ArrayHandle< vtkm::Vec<CoordinateType,3> >& normalsArray)
  {
    typedef typename vtkm::cont::DeviceAdapterAlgorithm<DeviceAdapter> DeviceAlgorithms;

    // Set up the Marching Cubes case tables
    vtkm::cont::ArrayHandle<vtkm::Id> vertexTableArray =
        vtkm::cont::make_ArrayHandle(vtkm::worklet::internal::numVerticesTable,
                                     256);
    vtkm::cont::ArrayHandle<vtkm::Id> triangleTableArray =
        vtkm::cont::make_ArrayHandle(vtkm::worklet::internal::triTable,
                                     256*16);

    // Call the ClassifyCell functor to compute the Marching Cubes case numbers
    // for each cell, and the number of vertices to be generated
    ClassifyCell classifyCell(vertexTableArray.PrepareForInput(DeviceAdapter()),
                              isovalue);

    typedef typename vtkm::worklet::DispatcherMapTopology<
                                      ClassifyCell,
                                      DeviceAdapter> ClassifyCellDispatcher;
    ClassifyCellDispatcher classifyCellDispatcher(classifyCell);

    vtkm::cont::ArrayHandle<vtkm::Id> numOutputTrisPerCell;
    classifyCellDispatcher.Invoke(isoField,
                                  this->DataSet.GetCellSet(0),
                                  numOutputTrisPerCell);

    // Compute the number of valid input cells and those ids
    this->NumOutputCells =
      DeviceAlgorithms::ScanInclusive(numOutputTrisPerCell,
                                      numOutputTrisPerCell);

    // Terminate if no cells have triangles left
    if (this->NumOutputCells == 0) return;

    typedef vtkm::cont::ArrayHandle<vtkm::Id> IdHandleType;

    IdHandleType validCellIndicesArray,inputCellIterationNumber;
    vtkm::cont::ArrayHandleCounting<vtkm::Id> validCellCountImplicitArray(0, 1, this->NumOutputCells);
    DeviceAlgorithms::UpperBounds(numOutputTrisPerCell,
                                  validCellCountImplicitArray,
                                  validCellIndicesArray);
    numOutputTrisPerCell.ReleaseResources();

    // Compute for each output triangle what iteration of the input cell generates it
    DeviceAlgorithms::LowerBounds(validCellIndicesArray,
                                  validCellIndicesArray,
                                  inputCellIterationNumber);

    // Generate a single triangle per cell
    const vtkm::Id numTotalVertices = this->NumOutputCells * 3;

    IsoSurfaceGenerate isosurface(
      isovalue,
      triangleTableArray.PrepareForInput(DeviceAdapter()),
      InterpolationWeight.PrepareForOutput(numTotalVertices, DeviceAdapter()),
      InterpolationLowId.PrepareForOutput(numTotalVertices, DeviceAdapter()),
      InterpolationHighId.PrepareForOutput(numTotalVertices, DeviceAdapter()),
      verticesArray.PrepareForOutput(numTotalVertices, DeviceAdapter()),
      normalsArray.PrepareForOutput(numTotalVertices, DeviceAdapter())
    );

    vtkm::cont::CellSetExplicit<> cellSet = this->DataSet.GetCellSet(0)
      .template CastTo<vtkm::cont::CellSetExplicit<> >();

    vtkm::cont::CellSetPermutation<vtkm::cont::ArrayHandle<vtkm::Id>,
      vtkm::cont::CellSetExplicit<> > cellPermutation(validCellIndicesArray,
                                                      cellSet);

    typedef typename vtkm::worklet::DispatcherMapTopology< IsoSurfaceGenerate,
      DeviceAdapter> IsoSurfaceDispatcher;
    IsoSurfaceDispatcher isosurfaceDispatcher(isosurface);
    isosurfaceDispatcher.Invoke(isoField,
                                this->DataSet.GetCoordinateSystem(0).GetData(),
                                inputCellIterationNumber,
                                cellPermutation);
  }

  template<typename Field, typename StorageTag>
  void MapFieldOntoIsosurface(const vtkm::cont::ArrayHandle<Field,
                              StorageTag>& fieldIn,
                              vtkm::cont::ArrayHandle< Field >& fieldOut)
  {
    if (this->NumOutputCells == 0)
      {
      fieldOut.Shrink(0);
      return;
      }

    typedef vtkm::cont::ArrayHandle<vtkm::Id> IdHandleType;
    typedef vtkm::cont::ArrayHandle<Field,StorageTag> FieldHandleType;
    typedef vtkm::cont::ArrayHandlePermutation<IdHandleType,
      FieldHandleType> FieldPermutationHandleType;

    FieldPermutationHandleType low(InterpolationLowId,fieldIn);
    FieldPermutationHandleType high(InterpolationHighId,fieldIn);

    ApplyToField<Field> applyToField;

    typedef typename vtkm::worklet::DispatcherMapField<
      ApplyToField<Field>,
      DeviceAdapter> ApplyToFieldDispatcher;
    ApplyToFieldDispatcher applyToFieldDispatcher(applyToField);

    applyToFieldDispatcher.Invoke(low,
                                  high,
                                  InterpolationWeight,
                                  fieldOut);
  }
};

}
} // namespace vtkm::worklet

#endif // vtk_m_worklet_IsosurfaceHexahedra_h
