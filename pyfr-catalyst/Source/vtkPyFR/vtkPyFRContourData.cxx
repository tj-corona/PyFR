#include "vtkPyFRContourData.h"

#include <vtkDataObjectTypes.h>
#include <vtkObjectFactory.h>

#include "PyFRContourData.h"

vtkStandardNewMacro(vtkPyFRContourData);

//----------------------------------------------------------------------------
vtkPyFRContourData::vtkPyFRContourData()
{
  this->data = new PyFRContourData();
}

//----------------------------------------------------------------------------
vtkPyFRContourData::~vtkPyFRContourData()
{
  delete this->data;
}

//----------------------------------------------------------------------------
void vtkPyFRContourData::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}

//----------------------------------------------------------------------------
vtkObject* New_vtkPyFRContourData() { return vtkPyFRContourData::New(); }
