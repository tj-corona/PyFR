
#include "vtkPyFRMapper.h"
#include "vtkPyFRContourMapper.h"

#include "vtkExecutive.h"
#include "vtkDemandDrivenPipeline.h"
#include "vtkObjectFactory.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkMath.h"
#include "vtkPolyData.h"
#include "vtkRenderWindow.h"
#include "vtkStreamingDemandDrivenPipeline.h"

#include <vtkPyFRContourData.h>

#include <vector>

vtkStandardNewMacro(vtkPyFRMapper);

class vtkPyFRMapperInternals
{
public:
  std::vector<vtkPyFRContourMapper*> Mappers;
};

//----------------------------------------------------------------------------
vtkPyFRMapper::vtkPyFRMapper()
{
  this->Internal = new vtkPyFRMapperInternals;
}

//----------------------------------------------------------------------------
vtkPyFRMapper::~vtkPyFRMapper()
{
  for(unsigned int i=0;i<this->Internal->Mappers.size();i++)
    {
    this->Internal->Mappers[i]->UnRegister(this);
    }
  this->Internal->Mappers.clear();

  delete this->Internal;
}

// Specify the type of data this mapper can handle.
//----------------------------------------------------------------------------
int vtkPyFRMapper::FillInputPortInformation(
  int vtkNotUsed(port), vtkInformation* info)
{
  info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkPyFRContourData");
  return 1;
}

// When the structure is out-of-date, recreate it by
// creating a mapper for each input data.
//----------------------------------------------------------------------------
void vtkPyFRMapper::BuildMappers()
{
  for(unsigned int i=0;i<this->Internal->Mappers.size();i++)
    {
    this->Internal->Mappers[i]->UnRegister(this);
    }
  this->Internal->Mappers.clear();

  //Get the dataset from the input
  vtkInformation* inInfo = this->GetExecutive()->GetInputInformation(0,0);
  vtkPyFRContourData *input = vtkPyFRContourData::SafeDownCast(
    inInfo->Get(vtkDataObject::DATA_OBJECT()));

  int numContours = input->GetNumberOfContours();
  for (int i = 0; i < numContours; ++i)
    {
    if (input->HasData(i))
      {
      vtkPyFRContourData* newcontour = vtkPyFRContourData::New();
      newcontour->ShallowCopy(input);

      vtkPyFRContourMapper *cmapper = vtkPyFRContourMapper::New();
      cmapper->Register(this); //increments the ref count on cmapper
      cmapper->SetActiveContour(i); //tell the mapper which contour it is supposed to render
      cmapper->SetInputData(newcontour);
      this->Internal->Mappers.push_back(cmapper);

      newcontour->FastDelete();
      cmapper->FastDelete();
      }
    }

  this->InternalMappersBuildTime.Modified();

}

//----------------------------------------------------------------------------
void vtkPyFRMapper::Render(vtkRenderer *ren, vtkActor *a)
{
  vtkDemandDrivenPipeline * executive =
  vtkDemandDrivenPipeline::SafeDownCast(this->GetExecutive());

  if(executive->GetPipelineMTime() >
      this->InternalMappersBuildTime.GetMTime())
    {
    this->BuildMappers();
    }

  this->TimeToDraw = 0;
  //Call Render() on each of the PolyDataMappers
  for(unsigned int i=0;i<this->Internal->Mappers.size();i++)
    {
    if ( this->ClippingPlanes !=
         this->Internal->Mappers[i]->GetClippingPlanes() )
      {
      this->Internal->Mappers[i]->SetClippingPlanes( this->ClippingPlanes );
      }

    // this->Internal->Mappers[i]->SetLookupTable(
    //   this->GetLookupTable());
    // this->Internal->Mappers[i]->SetScalarVisibility(
    //   this->GetScalarVisibility());
    // this->Internal->Mappers[i]->SetUseLookupTableScalarRange(
    //   this->GetUseLookupTableScalarRange());
    // this->Internal->Mappers[i]->SetScalarRange(
    //   this->GetScalarRange());
    // this->Internal->Mappers[i]->SetImmediateModeRendering(
    //   this->GetImmediateModeRendering());
    // this->Internal->Mappers[i]->SetColorMode(this->GetColorMode());
    // this->Internal->Mappers[i]->SetInterpolateScalarsBeforeMapping(
    //   this->GetInterpolateScalarsBeforeMapping());

    // this->Internal->Mappers[i]->SetScalarMode(this->GetScalarMode());
    // if ( this->ScalarMode == VTK_SCALAR_MODE_USE_POINT_FIELD_DATA ||
    //      this->ScalarMode == VTK_SCALAR_MODE_USE_CELL_FIELD_DATA )
    //   {
    //   if ( this->ArrayAccessMode == VTK_GET_ARRAY_BY_ID )
    //     {
    //     this->Internal->Mappers[i]->ColorByArrayComponent(
    //       this->ArrayId,ArrayComponent);
    //     }
    //   else
    //     {
    //     this->Internal->Mappers[i]->ColorByArrayComponent(
    //       this->ArrayName,ArrayComponent);
    //     }
    //   }

    this->Internal->Mappers[i]->Render(ren,a);
    this->TimeToDraw += this->Internal->Mappers[i]->GetTimeToDraw();
    }
}


//Looks at each DataSet and finds the union of all the bounds
//----------------------------------------------------------------------------
void vtkPyFRMapper::ComputeBounds()
{
  vtkMath::UninitializeBounds(this->Bounds);

  vtkInformation* inInfo = this->GetExecutive()->GetInputInformation(0,0);
  vtkPyFRContourData *input = vtkPyFRContourData::SafeDownCast(
    inInfo->Get(vtkDataObject::DATA_OBJECT()));

  input->GetBounds( this->Bounds );
  this->BoundsMTime.Modified();
}

//----------------------------------------------------------------------------
double *vtkPyFRMapper::GetBounds()
{
  if ( ! this->GetExecutive()->GetInputData(0, 0) )
    {
    vtkMath::UninitializeBounds(this->Bounds);
    return this->Bounds;
    }
  else
    {
    this->Update();
    //only compute bounds when the input data has changed

    vtkDemandDrivenPipeline * executive =
    vtkDemandDrivenPipeline::SafeDownCast(this->GetExecutive());

    if(executive->GetPipelineMTime() >
        this->InternalMappersBuildTime.GetMTime())
      {
      this->ComputeBounds();
      }

    return this->Bounds;
    }
}

//----------------------------------------------------------------------------
void vtkPyFRMapper::ReleaseGraphicsResources( vtkWindow *win )
{
  for(unsigned int i=0;i<this->Internal->Mappers.size();i++)
    {
    this->Internal->Mappers[i]->ReleaseGraphicsResources( win );
    }
}

//----------------------------------------------------------------------------
void vtkPyFRMapper::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}
