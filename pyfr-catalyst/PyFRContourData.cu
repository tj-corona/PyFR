#include "PyFRContourData.h"

#include <vtkDataObjectTypes.h>
#include <vtkObjectFactory.h>

//------------------------------------------------------------------------------
vtkStandardNewMacro(PyFRContourData);

//------------------------------------------------------------------------------
PyFRContourData::PyFRContourData()
{
}

//------------------------------------------------------------------------------
PyFRContourData::~PyFRContourData()
{
}

//------------------------------------------------------------------------------
vtkObject* NewPyFRContourData()
 {
   return PyFRContourData::New();
 }