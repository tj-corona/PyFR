#include "vtkPyFRDataContourFilter.h"

#include <vtkDataObject.h>
#include <vtkDataObjectTypes.h>
#include <vtkInformation.h>
#include <vtkInformationVector.h>
#include <vtkObjectFactory.h>

#include <vtkm/worklet/IsosurfaceHexahedra.h>

#include "PyFRData.h"
#include "PyFRContourData.h"

vtkStandardNewMacro(vtkPyFRDataContourFilter);

//----------------------------------------------------------------------------
vtkPyFRDataContourFilter::vtkPyFRDataContourFilter() : ContourValue(0.),
                                           ContourField("density")
{
}

//----------------------------------------------------------------------------
vtkPyFRDataContourFilter::~vtkPyFRDataContourFilter()
{
}

//----------------------------------------------------------------------------
int vtkPyFRDataContourFilter::RequestData(
  vtkInformation*,
  vtkInformationVector** inputVector,
  vtkInformationVector* outputVector)
{
  // get the info objects
  vtkInformation *inInfo = inputVector[0]->GetInformationObject(0);
  vtkInformation *outInfo = outputVector->GetInformationObject(0);

  // get the input and output
  PyFRData *input = PyFRData::SafeDownCast(
    inInfo->Get(vtkDataObject::DATA_OBJECT()));
  PyFRContourData *output = PyFRContourData::SafeDownCast(
    outInfo->Get(vtkDataObject::DATA_OBJECT()));

  const vtkm::cont::DataSet& dataSet = input->GetDataSet();

  typedef vtkm::worklet::IsosurfaceFilterHexahedra<double,
    VTKM_DEFAULT_DEVICE_ADAPTER_TAG> IsosurfaceFilter;

  PyFRContourData::Double3ArrayHandle& verts_out = output->Vertices;
  PyFRContourData::Double3ArrayHandle& normals_out = output->Normals;
  PyFRContourData::DoubleArrayHandle& scalars_out = output->Density;

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

  return 1;
}
//----------------------------------------------------------------------------

int vtkPyFRDataContourFilter::FillInputPortInformation(
  int vtkNotUsed(port), vtkInformation* info)
{
  info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "PyFRData");
  return 1;
}
//----------------------------------------------------------------------------

void vtkPyFRDataContourFilter::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "ContourField: " << this->ContourField << "\n";
  os << indent << "ContourValue: " << this->ContourValue << "\n";
}
