#include <cstdio>
#include <string>
#include "PyFRAdaptor.h"

#include "vtkCPVTKmPipeline.h"
#include <vtkCPDataDescription.h>
#include <vtkCPInputDataDescription.h>
#include <vtkCPProcessor.h>
#include <vtkNew.h>

#include "PyFRData.h"

namespace
{
  vtkCPProcessor* Processor = NULL;
}

void* CatalystInitialize(char* outputfile, void* p)
{
  printf("In CatalystInitialize\n");

  printf("Creating PyFRData object from data pointer...\n");
  PyFRData* data = PyFRData::New();
  data->Init(p);
  printf("Done!\n");

  if(Processor == NULL)
    {
    printf("Creating a new processor\n");
    Processor = vtkCPProcessor::New();
    printf("Initializing new processor...\n");
    Processor->Initialize();
    printf("Done!\n");
    }
  vtkNew<vtkCPVTKmPipeline> pipeline;
  pipeline->Initialize(outputfile);
  Processor->AddPipeline(pipeline.GetPointer());

  printf("Exiting CatalystInitialize()\n");
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
  printf("Calling data->Update()\n");
  PyFRData* data = static_cast<PyFRData*>(p);
  data->Update();
  printf("Done!\n");
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
