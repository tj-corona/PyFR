#include "PyFRContourFilter.h"

#include <vtkm/cont/cuda/DeviceAdapterCuda.h>

#include "IsosurfaceHexahedra.h"
#include "PyFRData.h"
#include "PyFRContourData.h"

//----------------------------------------------------------------------------
PyFRContourFilter::PyFRContourFilter() : ContourValue(0.),
                                         ContourField("density")
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
  PyFRContourData::Vec3ArrayHandle vertices = output->GetVertices();
  PyFRContourData::Vec3ArrayHandle normals = output->GetNormals();

  IsosurfaceFilter isosurfaceFilter;

  DataVec dataVec;
  dataVec.push_back(this->ContourValue);
  Vec3HandleVec verticesVec;
  verticesVec.push_back(vertices);
  Vec3HandleVec normalsVec;
  normalsVec.push_back(normals);

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
    PyFRContourData::ScalarDataArrayHandle scalars_out =
      output->GetScalarData(fields[i]);

    vtkm::cont::Field projectedField = dataSet.GetField(fields[i]);

    PyFRData::ScalarDataArrayHandle projectedArray = projectedField.GetData()
      .CastToArrayHandle(PyFRData::ScalarDataArrayHandle::ValueType(),
                         PyFRData::ScalarDataArrayHandle::StorageTag());

    ScalarDataHandleVec scalarDataHandleVec;
    scalarDataHandleVec.push_back(scalars_out);
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
