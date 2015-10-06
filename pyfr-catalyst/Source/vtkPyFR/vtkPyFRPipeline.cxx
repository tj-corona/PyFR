#include "vtkPyFRPipeline.h"

#include <sstream>

#include <vtkCPDataDescription.h>
#include <vtkCPInputDataDescription.h>
#include <vtkObjectFactory.h>
#include <vtkPVTrivialProducer.h>
#include <vtkSmartPointer.h>
#include <vtkSMIntVectorProperty.h>
#include <vtkSMDoubleVectorProperty.h>
#include <vtkSMInputProperty.h>
#include <vtkSMOutputPort.h>
#include <vtkSMPluginManager.h>
#include <vtkSMProxyManager.h>
#include <vtkSMSourceProxy.h>
#include <vtkSMSessionProxyManager.h>
#include <vtkSMStringVectorProperty.h>
#include <vtkSMWriterProxy.h>

#include "vtkPyFRData.h"
#include "vtkPyFRContourData.h"
#include "vtkPyFRContourDataConverter.h"
#include "vtkPyFRDataContourFilter.h"
#include "vtkXMLPyFRDataWriter.h"
#include "vtkXMLPyFRContourDataWriter.h"

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)

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
  this->FileName = std::string(fileName);

  vtkSMProxyManager::GetProxyManager()->GetPluginManager()->
    LoadLocalPlugin(TOSTRING(PyFRPlugin));
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

  if(this->FileName.empty())
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

  vtkSMProxyManager* proxyManager = vtkSMProxyManager::GetProxyManager();
  vtkSMSessionProxyManager* sessionProxyManager =
    proxyManager->GetActiveSessionProxyManager();

  // Create a vtkPVTrivialProducer and set its output
  // to be the input data.
  vtkSmartPointer<vtkSMSourceProxy> producer;
  producer.TakeReference(
    vtkSMSourceProxy::SafeDownCast(
      sessionProxyManager->NewProxy("sources", "PVTrivialProducer")));
  producer->UpdateVTKObjects();
  vtkObjectBase* clientSideObject = producer->GetClientSideObject();
  vtkPVTrivialProducer* realProducer =
    vtkPVTrivialProducer::SafeDownCast(clientSideObject);
  realProducer->SetOutput(pyfrData);

  // {
  // Create a convertor to convert the pyfr data into a vtkUnstructuredGrid
  // vtkSmartPointer<vtkSMSourceProxy> converter;
  // converter.TakeReference(
  //   vtkSMSourceProxy::SafeDownCast(sessionProxyManager->NewProxy("filters", "PyFRDataConverter")));
  // vtkSMInputProperty* converterInputConnection =
  //   vtkSMInputProperty::SafeDownCast(converter->GetProperty("Input"));

  // producer->UpdateVTKObjects();
  // converterInputConnection->SetInputConnection(0, producer, 0);
  // converter->UpdatePropertyInformation();
  // converter->UpdateVTKObjects();

  // // // Create an unstructured grid writer, set the filename and then update the
  // // pipeline.
  // vtkSmartPointer<vtkSMWriterProxy> writer;
  // writer.TakeReference(
  //   vtkSMWriterProxy::SafeDownCast(sessionProxyManager->NewProxy("writers", "XMLUnstructuredGridWriter")));
  // vtkSMInputProperty* writerInputConnection =
  //   vtkSMInputProperty::SafeDownCast(writer->GetProperty("Input"));
  // writerInputConnection->SetInputConnection(0, converter, 0);
  // vtkSMStringVectorProperty* fileName =
  //   vtkSMStringVectorProperty::SafeDownCast(writer->GetProperty("FileName"));

  // std::ostringstream o;
  // o << this->FileName.substr(0,this->FileName.find_last_of("."));
  // o << "_" << std::fixed << std::setprecision(3) << dataDescription->GetTime();
  // o << ".vtu";

  // fileName->SetElement(0, o.str().c_str());
  // writer->UpdatePropertyInformation();
  // writer->UpdateVTKObjects();
  // writer->UpdatePipeline();
  // }

  vtkSmartPointer<vtkSMSourceProxy> contour;
  contour.TakeReference(
    vtkSMSourceProxy::SafeDownCast(sessionProxyManager->NewProxy("filters", "PyFRDataContourFilter")));
  vtkSMInputProperty* contourInputConnection =
    vtkSMInputProperty::SafeDownCast(contour->GetProperty("Input"));

  vtkSMIntVectorProperty* contourField =
    vtkSMIntVectorProperty::SafeDownCast(contour->GetProperty("ContourField"));
  contourField->SetElements1(0);

  vtkSMDoubleVectorProperty* contourValue =
    vtkSMDoubleVectorProperty::SafeDownCast(contour->GetProperty("ContourValue"));
  contourValue->SetElements1(1.0045);

  producer->UpdateVTKObjects();
  contourInputConnection->SetInputConnection(0, producer, 0);
  contour->UpdatePropertyInformation();
  contour->UpdateVTKObjects();

  // Create a convertor to convert the pyfr contour data into polydata
  vtkSmartPointer<vtkSMSourceProxy> converter;
  converter.TakeReference(
    vtkSMSourceProxy::SafeDownCast(sessionProxyManager->NewProxy("filters", "PyFRContourDataConverter")));
  vtkSMInputProperty* converterInputConnection =
    vtkSMInputProperty::SafeDownCast(converter->GetProperty("Input"));

  producer->UpdateVTKObjects();
  converterInputConnection->SetInputConnection(0, contour, 0);
  converter->UpdatePropertyInformation();
  converter->UpdateVTKObjects();

  // Finally, create the polydata writer, set the
  // filename and then update the pipeline.
  vtkSmartPointer<vtkSMWriterProxy> writer;
  writer.TakeReference(
    vtkSMWriterProxy::SafeDownCast(sessionProxyManager->NewProxy("writers", "XMLPolyDataWriter")));
  vtkSMInputProperty* writerInputConnection =
    vtkSMInputProperty::SafeDownCast(writer->GetProperty("Input"));
  writerInputConnection->SetInputConnection(0, converter, 0);
  vtkSMStringVectorProperty* fileName =
    vtkSMStringVectorProperty::SafeDownCast(writer->GetProperty("FileName"));

  std::ostringstream o;
  o << this->FileName.substr(0,this->FileName.find_last_of("."));
  o << "_" << std::fixed << std::setprecision(3) << dataDescription->GetTime();
  o << ".vtp";

  fileName->SetElement(0, o.str().c_str());
  writer->UpdatePropertyInformation();
  writer->UpdateVTKObjects();
  writer->UpdatePipeline();

/*
  {
  std::stringstream s;
  s << this->FileName.substr(0,this->FileName.find_last_of("."));
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
  s << this->FileName.substr(0,this->FileName.find_last_of("."));
  s << "_" << std::fixed << std::setprecision(3) << dataDescription->GetTime();

  vtkSmartPointer<vtkXMLPyFRContourDataWriter> writer =
    vtkSmartPointer<vtkXMLPyFRContourDataWriter>::New();

  writer->SetFileName(s.str().c_str());
  writer->SetInputConnection(isocontour->GetOutputPort());
  writer->SetDataModeToBinary();
  writer->Write();
  }
*/
  return 1;
}

//----------------------------------------------------------------------------
void vtkPyFRPipeline::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "FileName: " << this->FileName << "\n";
}
