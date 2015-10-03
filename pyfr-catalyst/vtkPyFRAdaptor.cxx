#include <cstdio>
#include <string>
#include "vtkPyFRAdaptor.h"

#include <vtkCPDataDescription.h>
#include <vtkCPInputDataDescription.h>
#include <vtkCPProcessor.h>
#include <vtkInstantiator.h>
#include <vtkNew.h>

#include "PyFRData.h"

#include "vtkPyFRPipeline.h"
#include "vtkPyFRData.h"
#include "vtkPyFRContourData.h"

namespace
{
  vtkCPProcessor* Processor = NULL;
}

void* CatalystInitialize(char* outputfile, void* p)
{
  vtkInstantiator::RegisterInstantiator("vtkPyFRData",
                                        &New_vtkPyFRData);
  vtkInstantiator::RegisterInstantiator("vtkPyFRContourData",
                                        &New_vtkPyFRContourData);

  vtkPyFRData* data = vtkPyFRData::New();
  data->GetData()->Init(p);

  if(Processor == NULL)
    {
    Processor = vtkCPProcessor::New();
    Processor->Initialize();
    }
  vtkNew<vtkPyFRPipeline> pipeline;
  pipeline->Initialize(outputfile);
  Processor->AddPipeline(pipeline.GetPointer());

  return data;
}

void CatalystFinalize(void* p)
{
  vtkPyFRData* data = static_cast<vtkPyFRData*>(p);
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
  vtkPyFRData* data = static_cast<vtkPyFRData*>(p);
  data->GetData()->Update();
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
