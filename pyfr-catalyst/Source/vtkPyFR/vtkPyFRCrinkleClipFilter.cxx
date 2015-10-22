#include "vtkPyFRCrinkleClipFilter.h"

#include <vtkDataObject.h>
#include <vtkDataObjectTypes.h>
#include "vtkImplicitFunction.h"
#include <vtkInformation.h>
#include <vtkInformationVector.h>
#include <vtkInstantiator.h>
#include <vtkObjectFactory.h>
#include <vtkPlane.h>
#include <vtkSphere.h>

#include "PyFRCrinkleClipFilter.h"

#include "vtkPyFRData.h"

//----------------------------------------------------------------------------
int vtkPyFRCrinkleClipFilter::RegisterPyFRDataType()
{
  vtkInstantiator::RegisterInstantiator("vtkPyFRData",
                                        &New_vtkPyFRData);
  return 1;
}

int vtkPyFRCrinkleClipFilter::PyFRDataTypeRegistered =
  vtkPyFRCrinkleClipFilter::RegisterPyFRDataType();

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkPyFRCrinkleClipFilter);
vtkCxxSetObjectMacro(vtkPyFRCrinkleClipFilter,ClipFunction,vtkImplicitFunction);

//----------------------------------------------------------------------------
vtkPyFRCrinkleClipFilter::vtkPyFRCrinkleClipFilter()
{
  this->Plane = vtkPlane::New();
  this->ClipFunction  = this->Plane;
}

//----------------------------------------------------------------------------
vtkPyFRCrinkleClipFilter::~vtkPyFRCrinkleClipFilter()
{
  this->Plane->Delete();
}

//----------------------------------------------------------------------------
void vtkPyFRCrinkleClipFilter::SetInputData(vtkDataObject* input)
{
  this->SetInputData(0, input);
}

//----------------------------------------------------------------------------
void vtkPyFRCrinkleClipFilter::SetInputData(int index, vtkDataObject* input)
{
  this->SetInputDataInternal(index, input);
}

//----------------------------------------------------------------------------
int vtkPyFRCrinkleClipFilter::RequestData(
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
  vtkPyFRData *output = vtkPyFRData::SafeDownCast(
    outInfo->Get(vtkDataObject::DATA_OBJECT()));

  PyFRCrinkleClipFilter filter;

  vtkPlane *plane = vtkPlane::SafeDownCast(this->GetClipFunction());
  if (plane)
    filter(input->GetData(),output->GetData(),plane);
  vtkSphere *sphere = vtkSphere::SafeDownCast(this->GetClipFunction());
  if (sphere)
    filter(input->GetData(),output->GetData(),sphere);

  return 1;
}
//----------------------------------------------------------------------------

void vtkPyFRCrinkleClipFilter::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}
