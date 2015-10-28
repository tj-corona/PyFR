#include "vtkPyFRPipeline.h"

#include <sstream>

#include <vtkCollection.h>
#include <vtkCPDataDescription.h>
#include <vtkCPInputDataDescription.h>
#include <vtkLiveInsituLink.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkPlane.h>
#include <vtkPVArrayInformation.h>
#include <vtkPVLiveRenderView.h>
#include <vtkPVTrivialProducer.h>
#include <vtkSmartPointer.h>
#include <vtkSMIntVectorProperty.h>
#include <vtkSMDoubleVectorProperty.h>
#include <vtkSMInputProperty.h>
#include <vtkSMOutputPort.h>
#include <vtkSMParaViewPipelineControllerWithRendering.h>
#include <vtkSMPluginManager.h>
#include <vtkSMPropertyHelper.h>
#include <vtkSMProxyManager.h>
#include <vtkSMPVRepresentationProxy.h>
#include <vtkSMRepresentationProxy.h>
#include <vtkSMSourceProxy.h>
#include <vtkSMSessionProxyManager.h>
#include <vtkSMStringVectorProperty.h>
#include <vtkSMTransferFunctionManager.h>
#include <vtkSMTransferFunctionProxy.h>
#include <vtkSMViewProxy.h>
#include <vtkSMWriterProxy.h>
#include <vtkSMProxyListDomain.h>
#include "vtkPyFRData.h"
#include "vtkPyFRCrinkleClipFilter.h"
#include "vtkPyFRContourData.h"
#include "vtkPyFRContourDataConverter.h"
#include "vtkPyFRContourFilter.h"
#include "vtkPyFRParallelSliceFilter.h"
#include "vtkXMLPyFRDataWriter.h"
#include "vtkXMLPyFRContourDataWriter.h"
#include "vtkPVPlugin.h"

#include "PyFRData.h"

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)

#ifdef SINGLE
PV_PLUGIN_IMPORT_INIT(pyfr_plugin_fp32)
#else
PV_PLUGIN_IMPORT_INIT(pyfr_plugin_fp64)
#endif

vtkStandardNewMacro(vtkPyFRPipeline);

//----------------------------------------------------------------------------
vtkPyFRPipeline::vtkPyFRPipeline() : InsituLink(NULL)
{
}

//----------------------------------------------------------------------------
vtkPyFRPipeline::~vtkPyFRPipeline()
{
  if (this->InsituLink)
    this->InsituLink->Delete();
}

//----------------------------------------------------------------------------
void vtkPyFRPipeline::Initialize(char* hostName, int port, char* fileName,
                                 vtkCPDataDescription* dataDescription)
{
  this->FileName = std::string(fileName);

  vtkSMProxyManager* proxyManager = vtkSMProxyManager::GetProxyManager();

  // Load PyFR plugin
#ifdef SINGLE
PV_PLUGIN_IMPORT(pyfr_plugin_fp32)
#else
PV_PLUGIN_IMPORT(pyfr_plugin_fp64)
#endif

  // Grab the active session proxy manager
  vtkSMSessionProxyManager* sessionProxyManager =
    proxyManager->GetActiveSessionProxyManager();

  // Create the vtkLiveInsituLink (the "link" to the visualization processes).
  this->InsituLink = vtkLiveInsituLink::New();

  // Tell vtkLiveInsituLink what host/port must it connect to for the
  // visualization process.
  this->InsituLink->SetHostname(hostName);
  this->InsituLink->SetInsituPort(port);

  // Grab the data object from the data description
  vtkPyFRData* pyfrData =
    vtkPyFRData::SafeDownCast(dataDescription->
                              GetInputDescriptionByName("input")->GetGrid());

  // If these flags are set to true, the data will be written to vtk files on
  // the server side, but the pipeline cannot be cannected to a client.
  bool preFilterWrite = false;
  bool postFilterWrite = false;

  // Construct a pipeline controller to register my elements
  vtkNew<vtkSMParaViewPipelineControllerWithRendering> controller;

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
  controller->InitializeProxy(producer);
  controller->RegisterPipelineProxy(producer,"Source");

  if (preFilterWrite)
    {
    // Create a convertor to convert the pyfr data into a vtkUnstructuredGrid
    vtkSmartPointer<vtkSMSourceProxy> pyfrDataConverter;
    pyfrDataConverter.TakeReference(
      vtkSMSourceProxy::SafeDownCast(sessionProxyManager->
                                     NewProxy("filters", "PyFRDataConverter")));
    vtkSMInputProperty* pyfrDataConverterInputConnection =
      vtkSMInputProperty::SafeDownCast(pyfrDataConverter->GetProperty("Input"));

    producer->UpdateVTKObjects();
    pyfrDataConverterInputConnection->SetInputConnection(0, producer, 0);
    pyfrDataConverter->UpdatePropertyInformation();
    pyfrDataConverter->UpdateVTKObjects();
    controller->InitializeProxy(pyfrDataConverter);
    controller->RegisterPipelineProxy(pyfrDataConverter,"convertPyFRData");

    // Create an unstructured grid writer, set the filename and then update the
    // pipeline.
    vtkSmartPointer<vtkSMWriterProxy> unstructuredGridWriter;
    unstructuredGridWriter.TakeReference(
      vtkSMWriterProxy::SafeDownCast(sessionProxyManager->
                                     NewProxy("writers",
                                              "XMLUnstructuredGridWriter")));
    vtkSMInputProperty* unstructuredGridWriterInputConnection =
      vtkSMInputProperty::SafeDownCast(unstructuredGridWriter->
                                       GetProperty("Input"));
    unstructuredGridWriterInputConnection->SetInputConnection(0,
                                                              pyfrDataConverter,
                                                              0);
    vtkSMStringVectorProperty* unstructuredGridFileName =
      vtkSMStringVectorProperty::SafeDownCast(unstructuredGridWriter->
                                              GetProperty("FileName"));

      {
      std::ostringstream o;
      o << this->FileName.substr(0,this->FileName.find_last_of("."));
      o << "_" <<std::fixed << std::setprecision(3)<<dataDescription->GetTime();
      o << ".vtu";
      unstructuredGridFileName->SetElement(0, o.str().c_str());
      }

      unstructuredGridWriter->UpdatePropertyInformation();
      unstructuredGridWriter->UpdateVTKObjects();
      unstructuredGridWriter->UpdatePipeline();
      controller->InitializeProxy(unstructuredGridWriter);
      controller->RegisterPipelineProxy(unstructuredGridWriter,
                                        "UnstructuredGridWriter");
    }

  // Add the clip filter
  vtkSmartPointer<vtkSMSourceProxy> clip;
  clip.TakeReference(
    vtkSMSourceProxy::SafeDownCast(sessionProxyManager->
                                   NewProxy("filters",
                                            "PyFRCrinkleClipFilter")));
  controller->PreInitializeProxy(clip);
  vtkSMPropertyHelper(clip, "Input").Set(producer, 0);
  clip->UpdateVTKObjects();
  controller->PostInitializeProxy(clip);
  controller->RegisterPipelineProxy(clip,"Clip");

  // Add the slice filter
  vtkSmartPointer<vtkSMSourceProxy> slice;
  slice.TakeReference(
    vtkSMSourceProxy::SafeDownCast(sessionProxyManager->
                                   NewProxy("filters",
                                            "PyFRParallelSliceFilter")));
  controller->PreInitializeProxy(slice);
  vtkSMPropertyHelper(slice, "Input").Set(producer, 0);
  vtkSMPropertyHelper(slice,"ColorField").Set(1);
  slice->UpdateVTKObjects();
  controller->PostInitializeProxy(slice);
  controller->RegisterPipelineProxy(slice,"Slice");

  // Create a converter to convert the pyfr slice data into polydata
  vtkSmartPointer<vtkSMSourceProxy> pyfrSliceDataConverter;
  pyfrSliceDataConverter.TakeReference(
    vtkSMSourceProxy::SafeDownCast(sessionProxyManager->
                                   NewProxy("filters",
                                            "PyFRContourDataConverter")));
  vtkSMInputProperty* pyfrSliceDataConverterInputConnection =
    vtkSMInputProperty::SafeDownCast(pyfrSliceDataConverter->
                                     GetProperty("Input"));

  producer->UpdateVTKObjects();
  pyfrSliceDataConverterInputConnection->SetInputConnection(0, slice, 0);
  pyfrSliceDataConverter->UpdatePropertyInformation();
  pyfrSliceDataConverter->UpdateVTKObjects();
  controller->InitializeProxy(pyfrSliceDataConverter);
  controller->RegisterPipelineProxy(pyfrSliceDataConverter,
                                    "ConvertSlicesToPolyData");

  // Add the contour filter
  vtkSmartPointer<vtkSMSourceProxy> contour;
  contour.TakeReference(
    vtkSMSourceProxy::SafeDownCast(sessionProxyManager->
                                   NewProxy("filters",
                                            "PyFRContourFilter")));
  vtkSMInputProperty* contourInputConnection =
    vtkSMInputProperty::SafeDownCast(contour->GetProperty("Input"));

  vtkSMPropertyHelper(contour, "Input").Set(clip, 0);
  vtkSMPropertyHelper(contour,"ContourField").Set(0);
  vtkSMPropertyHelper(contour,"ColorField").Set(0);
  // vtkSMPropertyHelper(contour,"ContourValues").Set(0,.65);
  // vtkSMPropertyHelper(contour,"ContourValues").Set(1,.7);
  vtkSMPropertyHelper(contour,"ContourValues").Set(0,1.0025);
  vtkSMPropertyHelper(contour,"ContourValues").Set(1,1.0045);

  contour->UpdateVTKObjects();
  controller->InitializeProxy(contour);
  controller->RegisterPipelineProxy(contour,"Contour");

  vtkObjectBase* clientSideContourBase = contour->GetClientSideObject();
  vtkPyFRContourFilter* realContour =
    vtkPyFRContourFilter::SafeDownCast(clientSideContourBase);
  this->OutputData = vtkPyFRContourData::SafeDownCast(realContour->GetOutput());

  // Create a converter to convert the pyfr contour data into polydata
  vtkSmartPointer<vtkSMSourceProxy> pyfrContourDataConverter;
  pyfrContourDataConverter.TakeReference(
    vtkSMSourceProxy::SafeDownCast(sessionProxyManager->
                                   NewProxy("filters",
                                            "PyFRContourDataConverter")));
  vtkSMInputProperty* pyfrContourDataConverterInputConnection =
    vtkSMInputProperty::SafeDownCast(pyfrContourDataConverter->
                                     GetProperty("Input"));

  producer->UpdateVTKObjects();
  pyfrContourDataConverterInputConnection->SetInputConnection(0, contour, 0);
  pyfrContourDataConverter->UpdatePropertyInformation();
  pyfrContourDataConverter->UpdateVTKObjects();
  controller->InitializeProxy(pyfrContourDataConverter);
  controller->RegisterPipelineProxy(pyfrContourDataConverter,
                                    "ConvertContoursToPolyData");

  // Create a view
  vtkSmartPointer<vtkSMViewProxy> polydataViewer;
  polydataViewer.TakeReference(
    vtkSMViewProxy::SafeDownCast(sessionProxyManager->
                                 NewProxy("views","RenderView")));
  controller->InitializeProxy(polydataViewer);
  controller->RegisterViewProxy(polydataViewer);

  // Show the results.
  vtkSMProxy* sliceRepresentationBase =
    controller->Show(vtkSMSourceProxy::SafeDownCast(pyfrSliceDataConverter),0,
                     vtkSMViewProxy::SafeDownCast(polydataViewer));
  this->SliceRepresentation =
    vtkSMPVRepresentationProxy::SafeDownCast(sliceRepresentationBase);
  this->SliceRepresentation->SetScalarColoring(
    PyFRData::FieldName(vtkSMPropertyHelper(slice,
                                            "ColorField").GetAsInt()).c_str(),
    0);
  this->SliceRepresentation->RescaleTransferFunctionToDataRange();

  vtkSMProxy* contourRepresentationBase = controller->
    Show(vtkSMSourceProxy::SafeDownCast(pyfrContourDataConverter), 0,
         vtkSMViewProxy::SafeDownCast(polydataViewer));
  this->ContourRepresentation =
    vtkSMPVRepresentationProxy::SafeDownCast(contourRepresentationBase);
  this->ContourRepresentation->SetScalarColoring(
    PyFRData::FieldName(vtkSMPropertyHelper(contour,
                                            "ColorField").GetAsInt()).c_str(),
    0);
  this->ContourRepresentation->RescaleTransferFunctionToDataRange();

  vtkNew<vtkSMTransferFunctionManager> transferFunctionManager;
  vtkSMTransferFunctionProxy::ApplyPreset(
    transferFunctionManager->GetColorTransferFunction(
      PyFRData::FieldName(
        vtkSMPropertyHelper(contour,"ColorField").GetAsInt()).c_str(),
      vtkSMProxyManager::GetProxyManager()->GetActiveSessionProxyManager()),
    "Cool to Warm", true);
  vtkSMTransferFunctionProxy::ApplyPreset(
    transferFunctionManager->GetColorTransferFunction(
      PyFRData::FieldName(
        vtkSMPropertyHelper(slice,"ColorField").GetAsInt()).c_str(),
      vtkSMProxyManager::GetProxyManager()->GetActiveSessionProxyManager()),
    "Black-Body Radiation", true);

  if (postFilterWrite)
    {
    // Create the polydata writer, set the filename and then update the pipeline
    vtkSmartPointer<vtkSMWriterProxy> polydataWriter;
    polydataWriter.TakeReference(
      vtkSMWriterProxy::SafeDownCast(sessionProxyManager->
                                     NewProxy("writers", "XMLPolyDataWriter")));
    vtkSMInputProperty* polydataWriterInputConnection =
      vtkSMInputProperty::SafeDownCast(polydataWriter->GetProperty("Input"));
    polydataWriterInputConnection->SetInputConnection(0,
                                                      pyfrContourDataConverter,
                                                      0);
    vtkSMStringVectorProperty* polydataFileName =
      vtkSMStringVectorProperty::SafeDownCast(polydataWriter->
                                              GetProperty("FileName"));

      {
      std::ostringstream o;
      o << this->FileName.substr(0,this->FileName.find_last_of("."));
      o << "_"<<std::fixed<<std::setprecision(3)<<dataDescription->GetTime();
      o << ".vtp";
      polydataFileName->SetElement(0, o.str().c_str());
      }

      polydataWriter->UpdatePropertyInformation();
      polydataWriter->UpdateVTKObjects();
      polydataWriter->UpdatePipeline();
      controller->InitializeProxy(polydataWriter);
      controller->RegisterPipelineProxy(polydataWriter,"polydataWriter");
    }

  // Initialize the "link"
  this->InsituLink->InsituInitialize(vtkSMProxyManager::GetProxyManager()->
                                     GetActiveSessionProxyManager());
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
  vtkSMSessionProxyManager* sessionProxyManager =
    vtkSMProxyManager::GetProxyManager()->GetActiveSessionProxyManager();

  // Grab the data object from the data description
  vtkPyFRData* pyfrData =
    vtkPyFRData::SafeDownCast(dataDescription->
                              GetInputDescriptionByName("input")->GetGrid());

  // Use it to update the source
  vtkSMSourceProxy* source =
    vtkSMSourceProxy::SafeDownCast(sessionProxyManager->GetProxy("Source"));
  vtkObjectBase* clientSideObject = source->GetClientSideObject();
  vtkPVTrivialProducer* realProducer =
    vtkPVTrivialProducer::SafeDownCast(clientSideObject);
  realProducer->SetOutput(pyfrData,dataDescription->GetTime());

  vtkSMSourceProxy* unstructuredGridWriter =
    vtkSMSourceProxy::SafeDownCast(sessionProxyManager->
				   GetProxy("UnstructuredGridWriter"));
  if (unstructuredGridWriter)
    {
    vtkSMStringVectorProperty* unstructuredGridFileName =
      vtkSMStringVectorProperty::SafeDownCast(unstructuredGridWriter->
                                              GetProperty("FileName"));

      {
      std::ostringstream o;
      o << this->FileName.substr(0,this->FileName.find_last_of("."));
      o << "_"<<std::fixed<<std::setprecision(3)<<dataDescription->GetTime();
      o << ".vtu";
      unstructuredGridFileName->SetElement(0, o.str().c_str());
      }
      unstructuredGridWriter->UpdatePropertyInformation();
      unstructuredGridWriter->UpdateVTKObjects();
      unstructuredGridWriter->UpdatePipeline();
    }
  vtkSMSourceProxy* polydataWriter =
    vtkSMSourceProxy::SafeDownCast(sessionProxyManager->
				   GetProxy("polydataWriter"));
  if (polydataWriter)
    {
    vtkSMStringVectorProperty* polydataFileName =
      vtkSMStringVectorProperty::SafeDownCast(polydataWriter->
                                              GetProperty("FileName"));

      {
      std::ostringstream o;
      o << this->FileName.substr(0,this->FileName.find_last_of("."));
      o << "_"<<std::fixed<<std::setprecision(3)<<dataDescription->GetTime();
      o << ".vtp";
      polydataFileName->SetElement(0, o.str().c_str());
      }
      polydataWriter->UpdatePropertyInformation();
      polydataWriter->UpdateVTKObjects();
      polydataWriter->UpdatePipeline();
    }

  // stay in the loop while the simulation is paused
  while (true)
    {
    this->InsituLink->InsituUpdate(dataDescription->GetTime(),
                                   dataDescription->GetTimeStep());

    vtkSMSourceProxy* contour =
      vtkSMSourceProxy::SafeDownCast(sessionProxyManager->
                                     GetProxy("Contour"));
    this->ContourRepresentation->SetScalarColoring(
      PyFRData::FieldName(vtkSMPropertyHelper(contour,
                                              "ColorField").GetAsInt()).c_str(),
      0);
    this->ContourRepresentation->RescaleTransferFunctionToDataRange();

    vtkSMSourceProxy* slice =
      vtkSMSourceProxy::SafeDownCast(sessionProxyManager->
                                     GetProxy("Slice"));
    this->SliceRepresentation->SetScalarColoring(
      PyFRData::FieldName(vtkSMPropertyHelper(slice,
                                              "ColorField").GetAsInt()).c_str(),
      0);
    this->SliceRepresentation->RescaleTransferFunctionToDataRange();

    vtkNew<vtkCollection> views;
    sessionProxyManager->GetProxies("views",views.GetPointer());
    for (int i=0;i<views->GetNumberOfItems();i++)
      {
      vtkSMViewProxy* viewProxy =
        vtkSMViewProxy::SafeDownCast(views->GetItemAsObject(i));
      vtkSMPropertyHelper(viewProxy,"ViewTime").Set(dataDescription->GetTime());

      vtkNew<vtkSMTransferFunctionManager> transferFunctionManager;
      vtkSMTransferFunctionProxy::ApplyPreset(
        transferFunctionManager->GetColorTransferFunction(
          PyFRData::FieldName(
            vtkSMPropertyHelper(slice,"ColorField").GetAsInt()).c_str(),
          vtkSMProxyManager::GetProxyManager()->GetActiveSessionProxyManager()),
        "Cool to Warm", true);
      vtkSMTransferFunctionProxy::ApplyPreset(
        transferFunctionManager->GetColorTransferFunction(
          PyFRData::FieldName(
            vtkSMPropertyHelper(contour,"ColorField").GetAsInt()).c_str(),
          vtkSMProxyManager::GetProxyManager()->GetActiveSessionProxyManager()),
        "Black-Body Radiation", true);

      enum UpdateScalarBarsMode
      {
        HIDE_UNUSED_SCALAR_BARS = 0x01,
        SHOW_USED_SCALAR_BARS = 0x02
      };

      transferFunctionManager->UpdateScalarBars(viewProxy,
                                                HIDE_UNUSED_SCALAR_BARS |
                                                SHOW_USED_SCALAR_BARS);
      this->ContourRepresentation->SetScalarBarVisibility(viewProxy,true);
      this->SliceRepresentation->SetScalarBarVisibility(viewProxy,true);
      viewProxy->UpdateVTKObjects();
      viewProxy->Update();
      }

    this->InsituLink->InsituPostProcess(dataDescription->GetTime(),
                                        dataDescription->GetTimeStep());
    if (this->InsituLink->GetSimulationPaused())
      {
      if (this->InsituLink->WaitForLiveChange())
        {
        break;
        }
      }
    else
      {
      break;
      }
    }

  return 1;
}

//----------------------------------------------------------------------------
void vtkPyFRPipeline::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "FileName: " << this->FileName << "\n";
}
