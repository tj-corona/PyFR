#include <cstdio>
#include <iostream>
#include <string>
#include "vtkPyFRAdaptor.h"

#include <vtkCPDataDescription.h>
#include <vtkCPInputDataDescription.h>
#include <vtkCPProcessor.h>
#include <vtkNew.h>

#include "PyFRData.h"

#include "vtkPyFRPipeline.h"
#include "vtkPyFRData.h"
#include "vtkPyFRContourData.h"

namespace
{
  vtkCPProcessor* Processor = NULL;
}

//----------------------------------------------------------------------------
void* CatalystInitialize(char* hostName, int port, char* outputfile, void* p)
{
  vtkPyFRData* data = vtkPyFRData::New();
  data->GetData()->Init(p);

  if(Processor == NULL)
    {
    Processor = vtkCPProcessor::New();
    Processor->Initialize();
    }
  vtkNew<vtkCPDataDescription> dataDescription;
  dataDescription->AddInput("input");
  dataDescription->SetTimeData(0, 0);
  dataDescription->GetInputDescriptionByName("input")->SetGrid(data);

  vtkNew<vtkPyFRPipeline> pipeline;
  pipeline->Initialize(hostName,port,outputfile,dataDescription.GetPointer());
  Processor->AddPipeline(pipeline.GetPointer());

  return data;
}

//----------------------------------------------------------------------------
void CatalystFinalize(void* p)
{
  vtkPyFRData* data = static_cast<vtkPyFRData*>(p);
  vtkPyFRPipeline* pipeline =
    vtkPyFRPipeline::SafeDownCast(Processor->GetPipeline(0));
  vtkPyFRContourData* contourData = pipeline->GetOutputData();
  contourData->ReleaseResources();

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

//----------------------------------------------------------------------------
void CatalystCoProcess(double time,unsigned int timeStep, void* p,bool lastTimeStep)
{
  vtkPyFRData* data = static_cast<vtkPyFRData*>(p);
  data->GetData()->Update();
  vtkNew<vtkCPDataDescription> dataDescription;
  dataDescription->AddInput("input");
  dataDescription->SetTimeData(time, timeStep);
  if(lastTimeStep == true)
    {
    // assume that we want to all the pipelines to execute if it
    // is the last time step.
    dataDescription->ForceOutputOn();
    }
  if(Processor->RequestDataDescription(dataDescription.GetPointer()) != 0)
    {
    dataDescription->GetInputDescriptionByName("input")->SetGrid(data);
    Processor->CoProcess(dataDescription.GetPointer());
    }
}
