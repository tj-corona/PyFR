#include "vtkPyFRContourData.h"

#include <vtkDataObjectTypes.h>
#include <vtkObjectFactory.h>

#include "PyFRContourData.h"

vtkStandardNewMacro(vtkPyFRContourData);

//----------------------------------------------------------------------------
vtkPyFRContourData::vtkPyFRContourData()
{
  this->data = new PyFRContourData();
  for (unsigned i=0;i<6;i++)
    {
    this->Bounds[i] = 0.;
    }
  this->ColorRange[0] = 0.;
  this->ColorRange[1] = 1.;
}

//----------------------------------------------------------------------------
vtkPyFRContourData::~vtkPyFRContourData()
{
  this->ReleaseResources();
}

//----------------------------------------------------------------------------
void vtkPyFRContourData::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}

//----------------------------------------------------------------------------
int vtkPyFRContourData::GetNumberOfContours() const
{
  if (this->data)
    {
    return static_cast<int>(this->data->GetNumberOfContours());
    }
  return 0;
}


//----------------------------------------------------------------------------
void vtkPyFRContourData::ReleaseResources()
{
  if (this->data)
    {
    delete this->data;
    this->data = NULL;
    }
}

//----------------------------------------------------------------------------
void vtkPyFRContourData::GetBounds(double* bounds)
{
  if (this->GetMTime() > this->BoundsUpdateTime)
    {
    this->BoundsUpdateTime = this->GetMTime();
    FPType b[6];
    this->data->ComputeBounds(b);
    for (unsigned i=0;i<6;i++)
      {
      this->Bounds[i] = b[i];
      }
    }
  for (unsigned i=0;i<6;i++)
    {
    bounds[i] = this->Bounds[i];
    }
}

//----------------------------------------------------------------------------
bool vtkPyFRContourData::HasData() const
{
  for (unsigned i=0;i<this->data->GetNumberOfContours();i++)
    {
    if (this->HasData(i))
      return true;
    }
  return false;
}

//----------------------------------------------------------------------------
bool vtkPyFRContourData::HasData(int i) const
{
  return this->data->GetContourSize(i) > 0;
}

//----------------------------------------------------------------------------
void vtkPyFRContourData::SetColorRange(double* range)
{
  if (this->ColorRange[0] != range[0] || this->ColorRange[1] != range[1])
    {
    this->ColorRange[0] = range[0];
    this->ColorRange[1] = range[1];
    this->data->SetColorRange(this->ColorRange[0],this->ColorRange[1]);
    this->Modified();
    }
}

//----------------------------------------------------------------------------
vtkObject* New_vtkPyFRContourData() { return vtkPyFRContourData::New(); }
