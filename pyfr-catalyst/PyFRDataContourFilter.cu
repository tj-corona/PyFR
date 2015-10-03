#include "PyFRDataContourFilter.h"

#include <vtkm/cont/cuda/DeviceAdapterCuda.h>
#include <vtkm/worklet/IsosurfaceHexahedra.h>

#include "PyFRData.h"
#include "PyFRContourData.h"

//----------------------------------------------------------------------------
PyFRDataContourFilter::PyFRDataContourFilter() : ContourValue(0.),
                                                 ContourField("density")
{
}

//----------------------------------------------------------------------------
PyFRDataContourFilter::~PyFRDataContourFilter()
{
}

//----------------------------------------------------------------------------
void PyFRDataContourFilter::operator()(PyFRData* input,
                                       PyFRContourData* output) const
{
  const vtkm::cont::DataSet& dataSet = input->GetDataSet();

  typedef ::vtkm::cont::DeviceAdapterTagCuda CudaTag;
  typedef vtkm::worklet::IsosurfaceFilterHexahedra<FPType,CudaTag>
    IsosurfaceFilter;

  PyFRContourData::Vec3ArrayHandle& verts_out = output->Vertices;
  PyFRContourData::Vec3ArrayHandle& normals_out = output->Normals;
  PyFRContourData::ScalarDataArrayHandle& scalars_out = output->Density;

  vtkm::cont::Field scalars = dataSet.GetField(this->ContourField);
  PyFRData::ScalarDataArrayHandle scalarsArray = scalars.GetData()
    .CastToArrayHandle(PyFRData::ScalarDataArrayHandle::ValueType(),
                       PyFRData::ScalarDataArrayHandle::StorageTag());

  IsosurfaceFilter* isosurfaceFilter = new IsosurfaceFilter(dataSet);

  isosurfaceFilter->Run(this->ContourValue,
                        scalarsArray,
                        verts_out,
                        normals_out);

  isosurfaceFilter->MapFieldOntoIsosurface(scalarsArray,
                                           scalars_out);
}
//----------------------------------------------------------------------------

void PyFRDataContourFilter::SetContourField(int i)
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
