#include "vtkPyFRPipeline.h"

#include <sstream>

#include <vtkCPDataDescription.h>
#include <vtkCPInputDataDescription.h>
#include <vtkObjectFactory.h>
#include <vtkSmartPointer.h>

#include "vtkPyFRData.h"
#include "vtkPyFRContourData.h"
#include "vtkPyFRDataContourFilter.h"
#include "vtkXMLPyFRDataWriter.h"
#include "vtkXMLPyFRContourDataWriter.h"

vtkStandardNewMacro(vtkPyFRPipeline);

//----------------------------------------------------------------------------
vtkPyFRPipeline::vtkPyFRPipeline()
{
}

//----------------------------------------------------------------------------
vtkPyFRPipeline::~vtkPyFRPipeline()
{
}

//----------------------------------------------------------------------------
void vtkPyFRPipeline::Initialize(char* fileName)
{
  this->fileName = std::string(fileName);
}

//----------------------------------------------------------------------------
int vtkPyFRPipeline::RequestDataDescription(
  vtkCPDataDescription* dataDescription)
{
  if(!dataDescription)
    {
    vtkWarningMacro("dataDescription is NULL.");
    return 0;
    }

  if(this->fileName.empty())
    {
    vtkWarningMacro("No output file name given to output results to.");
    return 0;
    }

  dataDescription->GetInputDescriptionByName("input")->AllFieldsOn();
  dataDescription->GetInputDescriptionByName("input")->GenerateMeshOn();
  return 1;
}

//----------------------------------------------------------------------------
int vtkPyFRPipeline::CoProcess(vtkCPDataDescription* dataDescription)
{
  if(!dataDescription)
    {
    vtkWarningMacro("DataDescription is NULL");
    return 0;
    }
  vtkPyFRData* pyfrData =
    vtkPyFRData::SafeDownCast(dataDescription->
                              GetInputDescriptionByName("input")->GetGrid());
  if(pyfrData == NULL)
    {
    vtkWarningMacro("DataDescription is missing input PyFR data.");
    return 0;
    }
  if(this->RequestDataDescription(dataDescription) == 0)
    {
    return 1;
    }

  {
  std::stringstream s;
  s << fileName.substr(0,fileName.find_last_of("."));
  s << "_" << std::fixed << std::setprecision(3) << dataDescription->GetTime();
  s << "_input";

  vtkSmartPointer<vtkXMLPyFRDataWriter> writer =
    vtkSmartPointer<vtkXMLPyFRDataWriter>::New();

  writer->SetFileName(s.str().c_str());
  writer->SetInputData(pyfrData);
  writer->SetDataModeToBinary();
  writer->Write();
  }

  vtkSmartPointer<vtkPyFRDataContourFilter> isocontour =
    vtkSmartPointer<vtkPyFRDataContourFilter>::New();

  isocontour->SetContourField(0);
  isocontour->SetContourValue(1.0045);
  isocontour->SetInputData(pyfrData);

  {
  std::stringstream s;
  s << fileName.substr(0,fileName.find_last_of("."));
  s << "_" << std::fixed << std::setprecision(3) << dataDescription->GetTime();

  vtkSmartPointer<vtkXMLPyFRContourDataWriter> writer =
    vtkSmartPointer<vtkXMLPyFRContourDataWriter>::New();

  writer->SetFileName(s.str().c_str());
  writer->SetInputConnection(isocontour->GetOutputPort());
  writer->SetDataModeToBinary();
  writer->Write();
  }
  return 1;
}

//----------------------------------------------------------------------------
void vtkPyFRPipeline::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "fileName: " << this->fileName << "\n";
}
