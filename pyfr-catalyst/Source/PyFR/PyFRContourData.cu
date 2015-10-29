#include "PyFRContourData.h"

#include "Bounds.h"

#include <vtkm/Math.h>
#include <vtkm/Pair.h>
#include <vtkm/Types.h>
#include <vtkm/VectorAnalysis.h>

#include <vtkm/cont/cuda/DeviceAdapterCuda.h>
#include <vtkm/cont/DeviceAdapter.h>
#include <vtkm/cont/ArrayHandleTransform.h>

//----------------------------------------------------------------------------
void PyFRContourData::SetNumberOfContours(unsigned nContours)
{
  // NB: Cannot call resize to increase the lengths of vectors of array
  // handles (or classes containing them)! You will end up with a vector of
  // smart pointers to the same array instance. A specialization of
  // std::allocator<> for array handles should be created.
  for (unsigned i=this->Contours.size();i<nContours;i++)
    this->Contours.push_back(PyFRContour());
  this->Contours.resize(nContours);
}

//----------------------------------------------------------------------------
unsigned PyFRContourData::GetContourSize(int contour) const
{
  return this->GetContour(contour).GetVertices().GetNumberOfValues();
}

//----------------------------------------------------------------------------
void PyFRContourData::ComputeContourBounds(int contour,FPType* bounds) const
{
  typedef ::vtkm::cont::DeviceAdapterTagCuda CudaTag;
  typedef vtkm::cont::DeviceAdapterAlgorithm<CudaTag> Algorithm;
  typedef vtkm::Vec<vtkm::Float64, 3> ResultType;
  typedef vtkm::Pair<ResultType, ResultType> MinMaxPairType;
  typedef PyFRContour::Vec3ArrayHandle ArrayHandleType;

  MinMaxPairType initialValue =
    make_Pair(ResultType(vtkm::Infinity64()),
              ResultType(vtkm::NegativeInfinity64()));

  vtkm::cont::ArrayHandleTransform<MinMaxPairType, ArrayHandleType,
    internal::InputToOutputTypeTransform<3> > input(this->GetContour(contour)
                                                    .GetVertices());

  MinMaxPairType result = Algorithm::Reduce(input, initialValue,
                                            internal::MinMax<3>());

  for (unsigned i=0;i<3;i++)
    {
    bounds[2*i] = result.first[i];
    bounds[2*i+1] = result.second[i];
    }
}

//----------------------------------------------------------------------------
void PyFRContourData::ComputeBounds(FPType* bounds) const
{
  for (unsigned i=0;i<3;i++)
    {
    bounds[2*i] = std::numeric_limits<FPType>::max();
    bounds[2*i+1] = std::numeric_limits<FPType>::min();
    }
  for (unsigned i=0;i<this->GetNumberOfContours();i++)
    {
    FPType b[6];
    this->ComputeContourBounds(i,b);
    for (unsigned j=0;j<3;j++)
      {
      int jj = 2*j;
      bounds[jj] = (bounds[jj] < b[jj] ? bounds[jj] : b[jj]);
      jj++;
      bounds[jj] = (bounds[jj] > b[jj] ? bounds[jj] : b[jj]);
      }
    }
}