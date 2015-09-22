#include <cstdio>
#include <string>
#include "PyFRAdaptor.h"

#include "vtkCPVTKmPipeline.h"
#include <vtkCPDataDescription.h>
#include <vtkCPInputDataDescription.h>
#include <vtkCPProcessor.h>
#include <vtkDataObjectTypes.h>
#include <vtkNew.h>

#include "PyFRData.h"

namespace
{
  vtkCPProcessor* Processor = NULL;
}

void* CatalystInitialize(char* outputfile, char* datasetformat, void* p)
{
  PyFRData* data = PyFRData::New();
  data->Init(vtkDataObjectTypes::GetTypeIdFromClassName(datasetformat),p);

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
