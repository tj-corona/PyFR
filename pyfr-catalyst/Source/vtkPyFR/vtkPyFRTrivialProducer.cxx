#include "vtkPyFRTrivialProducer.h"

#include <vtkDataObject.h>
#include <vtkDataObjectTypes.h>
#include <vtkInformation.h>
#include <vtkInformationVector.h>
#include <vtkObjectFactory.h>

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkPyFRTrivialProducer);

//----------------------------------------------------------------------------
vtkPyFRTrivialProducer::vtkPyFRTrivialProducer() : Preset(0)
{
}

//----------------------------------------------------------------------------
vtkPyFRTrivialProducer::~vtkPyFRTrivialProducer()
{
}
