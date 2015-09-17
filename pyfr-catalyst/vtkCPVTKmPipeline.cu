#include "vtkCPVTKmPipeline.h"

#include <sstream>

#include <cuda.h>

#include "PyFRData.h"

#include <vtkCommunicator.h>
#include <vtkCompleteArrays.h>
#include <vtkCPDataDescription.h>
#include <vtkCPInputDataDescription.h>
#include <vtkDataArray.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkPointData.h>

#include <vtkm/cont/ArrayHandle.h>
#include <vtkm/cont/DeviceAdapter.h>
#include <vtkm/cont/Field.h>
#include <vtkm/cont/DataSet.h>
#include <vtkm/cont/internal/DeviceAdapterTag.h>
#include <vtkm/cont/cuda/ArrayHandleCuda.h>
#include <vtkm/cont/cuda/internal/DeviceAdapterTagCuda.h>
#include <vtkm/worklet/DispatcherMapField.h>
#include <vtkm/worklet/ExternalFaces.h>
#include <vtkm/worklet/WorkletMapField.h>

vtkStandardNewMacro(vtkCPVTKmPipeline);

//----------------------------------------------------------------------------
vtkCPVTKmPipeline::vtkCPVTKmPipeline()
{
}

//----------------------------------------------------------------------------
vtkCPVTKmPipeline::~vtkCPVTKmPipeline()
{
}

//----------------------------------------------------------------------------
void vtkCPVTKmPipeline::Initialize(char* fileName)
{
  this->fileName = std::string(fileName);
}

//----------------------------------------------------------------------------
int vtkCPVTKmPipeline::RequestDataDescription(
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
int vtkCPVTKmPipeline::CoProcess(vtkCPDataDescription* dataDescription)
{
  if(!dataDescription)
    {
    vtkWarningMacro("DataDescription is NULL");
    return 0;
    }
  PyFRData* pyfrData =
    PyFRData::SafeDownCast(dataDescription->
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

  vtkm::cont::DataSet& dataSet = pyfrData->GetDataSet();

  vtkm::cont::CellSetExplicit<>& cset =
    dataSet.GetCellSet(0).CastTo<vtkm::cont::CellSetExplicit<> >();
  vtkm::cont::ArrayHandle<vtkm::Id> shapes = cset.GetShapesArray(
    vtkm::TopologyElementTagPoint(),vtkm::TopologyElementTagCell());
  vtkm::cont::ArrayHandle<vtkm::Id> nindices = cset.GetNumIndicesArray(
    vtkm::TopologyElementTagPoint(),vtkm::TopologyElementTagCell());
  vtkm::cont::ArrayHandle<vtkm::Id> conn = cset.GetConnectivityArray(
    vtkm::TopologyElementTagPoint(),vtkm::TopologyElementTagCell());
  vtkm::cont::ArrayHandle<vtkm::Id> shapes_out;
  vtkm::cont::ArrayHandle<vtkm::Id> nindices_out;
  vtkm::cont::ArrayHandle<vtkm::Id> conn_out;

  vtkm::worklet::ExternalFaces<VTKM_DEFAULT_DEVICE_ADAPTER_TAG>().run(
    shapes, nindices, conn, shapes_out, nindices_out, conn_out
  );
  vtkm::cont::DataSet outDataSet;
  for(size_t i=0; i < (size_t)dataSet.GetNumberOfCoordinateSystems(); ++i)
    {
    outDataSet.AddCoordinateSystem(dataSet.GetCoordinateSystem(i));
    }

  std::cout << shapes.GetNumberOfValues() << " input elements, "
            << shapes_out.GetNumberOfValues() << " output elements.\n";

/*
  vm::CellSetExplicit<> outcset("cells", shapes_out.GetNumberOfValues());
  outcset.Fill(shapes_out, nindices_out, conn_out);
  outds.AddCellSet(outcset);

  vtkUnstructuredGrid* grid = vtkUnstructuredGrid::SafeDownCast(
    dataDescription->GetInputDescriptionByName("input")->GetGrid());
  if(grid == NULL)
    {
    vtkWarningMacro("DataDescription is missing input unstructured grid.");
    return 0;
    }
  if(this->RequestDataDescription(dataDescription) == 0)
    {
    return 1;
    }
*/

/*
  vtkNew<vtkPVTrivialProducer> producer;
  producer->SetOutput(grid);

  vtkNew<vtkPVArrayCalculator> calculator;
  calculator->SetInputConnection(producer->GetOutputPort());
  calculator->SetAttributeMode(1);
  calculator->SetResultArrayName("velocity magnitude");
  calculator->SetFunction("mag(velocity)");

  // update now so that we can get the global data bounds of
  // the velocity magnitude for thresholding
  calculator->Update();
  double range[2];
  vtkUnstructuredGrid::SafeDownCast(calculator->GetOutput())->GetPointData()
    ->GetArray("velocity magnitude")->GetRange(range, 0);
  double globalRange[2];
  vtkMultiProcessController::GetGlobalController()->AllReduce(
    range+1, globalRange+1, 1, vtkCommunicator::MAX_OP);

  vtkNew<vtkThreshold> threshold;
  threshold->SetInputConnection(calculator->GetOutputPort());
  threshold->SetInputArrayToProcess(
    0, 0, 0, "vtkDataObject::FIELD_ASSOCIATION_POINTS", "velocity magnitude");
  threshold->ThresholdBetween(0.9*globalRange[1], globalRange[1]);

  // If process 0 doesn't have any points or cells, the writer may
  // have problems in parallel so we use completeArrays to fill in
  // the missing information.
  vtkNew<vtkCompleteArrays> completeArrays;
  completeArrays->SetInputConnection(threshold->GetOutputPort());

  vtkNew<vtkXMLPUnstructuredGridWriter> writer;
  writer->SetInputConnection(completeArrays->GetOutputPort());
  std::ostringstream o;
  o << dataDescription->GetTimeStep();
  std::string name = this->fileName + o.str() + ".pvtu";
  writer->SetfileName(name.c_str());
  writer->Update();
*/
  return 1;
}

//----------------------------------------------------------------------------
void vtkCPVTKmPipeline::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "fileName: " << this->fileName << "\n";
}
