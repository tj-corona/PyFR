#include "vtkPyFRDataContourFilter.h"

#include <vtkDataObject.h>
#include <vtkDataObjectTypes.h>
#include <vtkInformation.h>
#include <vtkInformationVector.h>
#include <vtkObjectFactory.h>

#include "PyFRDataContourFilter.h"

#include "vtkPyFRData.h"
#include "vtkPyFRContourData.h"

vtkStandardNewMacro(vtkPyFRDataContourFilter);

//----------------------------------------------------------------------------
vtkPyFRDataContourFilter::vtkPyFRDataContourFilter() : ContourValue(0.),
                                                       ContourField(0)
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
  vtkPyFRData *input = vtkPyFRData::SafeDownCast(
    inInfo->Get(vtkDataObject::DATA_OBJECT()));
  vtkPyFRContourData *output = vtkPyFRContourData::SafeDownCast(
    outInfo->Get(vtkDataObject::DATA_OBJECT()));

  PyFRDataContourFilter filter;
  filter.SetContourValue(this->ContourValue);
  filter.SetContourField(this->ContourField);
  filter(input->GetData(),output->GetData());

  return 1;
}
//----------------------------------------------------------------------------

int vtkPyFRDataContourFilter::FillInputPortInformation(
  int vtkNotUsed(port), vtkInformation* info)
{
  info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkPyFRData");
  return 1;
}
//----------------------------------------------------------------------------

void vtkPyFRDataContourFilter::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "ContourField: " << this->ContourField << "\n";
  os << indent << "ContourValue: " << this->ContourValue << "\n";
}
