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
#include <vtkNew.h>
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
vtkPyFRCrinkleClipFilter::vtkPyFRCrinkleClipFilter() : LastExecuteTime(0)
{
  this->ClipFunction  = NULL;
  vtkNew<vtkPlane> defaultPlane;
  this->SetClipFunction(defaultPlane.GetPointer());
}

//----------------------------------------------------------------------------
vtkPyFRCrinkleClipFilter::~vtkPyFRCrinkleClipFilter()
{
  this->SetClipFunction(NULL);
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
    {
    if (plane->GetMTime() > this->LastExecuteTime)
      {
      this->LastExecuteTime = this->GetMTime();
      filter(input->GetData(),output->GetData(),plane);
      }
    }
  vtkSphere *sphere = vtkSphere::SafeDownCast(this->GetClipFunction());
  if (sphere)
    {
    if (sphere->GetMTime() > this->LastExecuteTime)
      {
      this->LastExecuteTime = this->GetMTime();
      filter(input->GetData(),output->GetData(),sphere);
      }
    }

  return 1;
}
//----------------------------------------------------------------------------

int vtkPyFRCrinkleClipFilter::FillInputPortInformation(
  int vtkNotUsed(port), vtkInformation* info)
{
  info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkPyFRData");
  return 1;
}
//----------------------------------------------------------------------------

// Overload standard modified time function. If the implicit clip function is
// modified, then this object is modified as well.
unsigned long vtkPyFRCrinkleClipFilter::GetMTime()
{
  unsigned long mTime=this->Superclass::GetMTime();
  unsigned long time;

  if (this->ClipFunction)
    {
    time = this->ClipFunction->GetMTime();
    mTime = ( time > mTime ? time : mTime );
    }

  return mTime;
}
//-----------------------------------------------------------------------------

void vtkPyFRCrinkleClipFilter::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}
