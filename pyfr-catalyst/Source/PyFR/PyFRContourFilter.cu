#include "PyFRContourFilter.h"

#include <vtkm/cont/cuda/DeviceAdapterCuda.h>

#include "IsosurfaceHexahedra.h"
#include "PyFRData.h"
#include "PyFRContourData.h"

//----------------------------------------------------------------------------
PyFRContourFilter::PyFRContourFilter() : ContourField("density")
{
}

//----------------------------------------------------------------------------
PyFRContourFilter::~PyFRContourFilter()
{
}

//----------------------------------------------------------------------------
void PyFRContourFilter::operator()(PyFRData* input,
                                   PyFRContourData* output) const
{
  const vtkm::cont::DataSet& dataSet = input->GetDataSet();

  typedef ::vtkm::cont::DeviceAdapterTagCuda CudaTag;

  typedef vtkm::worklet::IsosurfaceFilterHexahedra<FPType,CudaTag>
  IsosurfaceFilter;

  typedef std::vector<vtkm::cont::ArrayHandle<vtkm::Vec<FPType,3> > >
    Vec3HandleVec;
  typedef std::vector<vtkm::cont::ArrayHandle<FPType> > ScalarDataHandleVec;
  typedef std::vector<FPType> DataVec;

  vtkm::cont::Field contourField = dataSet.GetField(this->ContourField);
  PyFRData::ScalarDataArrayHandle contourArray = contourField.GetData()
    .CastToArrayHandle(PyFRData::ScalarDataArrayHandle::ValueType(),
                       PyFRData::ScalarDataArrayHandle::StorageTag());

  DataVec dataVec;
  Vec3HandleVec verticesVec;
  Vec3HandleVec normalsVec;
  output->SetNumberOfContours(this->ContourValues.size());
  for (unsigned i=0;i<output->GetNumberOfContours();i++)
    {
    dataVec.push_back(this->ContourValues[i]);
    verticesVec.push_back(output->GetContour(i).GetVertices());
    normalsVec.push_back(output->GetContour(i).GetNormals());
    }

  IsosurfaceFilter isosurfaceFilter;
  isosurfaceFilter.Run(dataVec,
                       dataSet.GetCellSet().CastTo(PyFRData::CellSet()),
                       dataSet.GetCoordinateSystem(),
                       contourArray,
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

//----------------------------------------------------------------------------
void PyFRContourFilter::SetContourField(int i)
{
  switch (i)
    {
    case 0:
      this->SetContourFieldToDensity();
      break;
    case 1:
      this->SetContourFieldToPressure();
      break;
    case 2:
      this->SetContourFieldToVelocity_u();
      break;
    case 3:
      this->SetContourFieldToVelocity_v();
      break;
    case 4:
      this->SetContourFieldToVelocity_w();
      break;
    }
}
