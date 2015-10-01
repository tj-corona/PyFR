#include <cstdio>
#include <string>
#include "PyFRAdaptor.h"

#include "vtkCPVTKmPipeline.h"
#include <vtkCPDataDescription.h>
#include <vtkCPInputDataDescription.h>
#include <vtkCPProcessor.h>
#include <vtkInstantiator.h>
#include <vtkNew.h>

#include "PyFRData.h"
#include "PyFRContourData.h"

namespace
{
  vtkCPProcessor* Processor = NULL;
}

void* CatalystInitialize(char* outputfile, void* p)
{
  vtkInstantiator::RegisterInstantiator("PyFRData",&NewPyFRData);
  vtkInstantiator::RegisterInstantiator("PyFRContourData",&NewPyFRContourData);

  PyFRData* data = PyFRData::New();
  data->Init(p);

  if(Processor == NULL)
    {
    Processor = vtkCPProcessor::New();
    Processor->Initialize();
    }
  vtkNew<vtkCPVTKmPipeline> pipeline;
  pipeline->Initialize(outputfile);
  Processor->AddPipeline(pipeline.GetPointer());

  return data;
}

void CatalystFinalize(void* p)
{
  PyFRData* data = static_cast<PyFRData*>(p);
  if(Processor)
    {
    Processor->Delete();
    Processor = NULL;
    }
  if (data)
    {
    data->Delete();
    }
}

void CatalystCoProcess(double time,unsigned int timeStep, void* p)
{
  PyFRData* data = static_cast<PyFRData*>(p);
  data->Update();
  vtkNew<vtkCPDataDescription> dataDescription;
  dataDescription->AddInput("input");
  dataDescription->SetTimeData(time, timeStep);
  // if(lastTimeStep == true)
  //   {
  //   // assume that we want to all the pipelines to execute if it
  //   // is the last time step.
  //   dataDescription->ForceOutputOn();
  //   }
  if(Processor->RequestDataDescription(dataDescription.GetPointer()) != 0)
    {
    dataDescription->GetInputDescriptionByName("input")->SetGrid(data);
    Processor->CoProcess(dataDescription.GetPointer());
    }
}
