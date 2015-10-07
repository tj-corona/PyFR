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
  // Yes, not deleting this->data results in a memory leak. This class is
  // instantiated exactly once, though, and only destructs with the destruction
  // of the Catalyst pipeline (i.e. at the end of the simulation). When we try
  // to delete this->data, we end up with a cuda runtime segfault in
  // thrust::system::cuda::detail::trivial_copy_detail::checked_cudaMemcpy. I'm
  // not sure if this bug is due to me, vtk-m, thrust, or simultaneously using
  // CUDA APIs from python and C++, but for the purposes of having a working
  // demonstration, I am taking the coward's way out.

  this->ReleaseResources();
}

//----------------------------------------------------------------------------
void vtkPyFRContourData::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
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
vtkObject* New_vtkPyFRContourData() { return vtkPyFRContourData::New(); }
