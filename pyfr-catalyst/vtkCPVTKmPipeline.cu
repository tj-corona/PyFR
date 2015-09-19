#include "vtkCPVTKmPipeline.h"

#include <string>
#include <sstream>

#include "PyFRData.h"

#include <vtkCommunicator.h>
#include <vtkCompleteArrays.h>
#include <vtkCPDataDescription.h>
#include <vtkCPInputDataDescription.h>
#include <vtkDataArray.h>
#include <vtkFloatArray.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkSmartPointer.h>
#include <vtkUnstructuredGrid.h>
#include <vtkXMLPolyDataWriter.h>

#include <vtkm/cont/ArrayHandleCast.h>
#include <vtkm/cont/DeviceAdapter.h>
#include <vtkm/cont/DeviceAdapterAlgorithm.h>
#include <vtkm/cont/cuda/ArrayHandleCuda.h>
#include <vtkm/cont/cuda/internal/DeviceAdapterTagCuda.h>

#include "ArrayHandleVTK.h"

// #include <vtkm/worklet/DispatcherMapField.h>
// #include "TransferData.h"
// #include <vtkm/worklet/WorkletMapField.h>

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

  
  vtkm::cont::Field xyz = dataSet.GetField("xyz");
  typedef vtkm::cont::ArrayHandle<float> FloatArrayHandle;
  FloatArrayHandle verts =
    xyz.GetData().CastToArrayHandle(FloatArrayHandle::ValueType(),
                                    FloatArrayHandle::StorageTag());

  // vtkm::cont::ArrayHandle<float>::PortalConstControl vertsPortal =
  //   verts.GetPortalConstControl();
  
  // std::cout<<"Verts: [ ";
  // for (size_t i=0;i<verts.GetNumberOfValues();i++)
  //   std::cout<<vertsPortal.Get(i)<<" ";
  // std::cout<<"]"<<std::endl;

  vtkm::cont::CellSetExplicit<>& cset =
    dataSet.GetCellSet(0).CastTo<vtkm::cont::CellSetExplicit<> >();
  typedef vtkm::cont::ArrayHandle<vtkm::Id> IdArrayHandle;
  IdArrayHandle shapes = cset.GetShapesArray(
    vtkm::TopologyElementTagPoint(),vtkm::TopologyElementTagCell());
  IdArrayHandle nindices = cset.GetNumIndicesArray(
    vtkm::TopologyElementTagPoint(),vtkm::TopologyElementTagCell());
  IdArrayHandle conn = cset.GetConnectivityArray(
    vtkm::TopologyElementTagPoint(),vtkm::TopologyElementTagCell());
  typedef vtkm::cont::ArrayHandleVTK<vtkm::Id> IdArrayHandleVTK;
  typedef vtkm::cont::ArrayHandleVTK<float> FloatArrayHandleVTK;
  FloatArrayHandleVTK verts_out;
  IdArrayHandleVTK shapes_out;
  IdArrayHandleVTK nindices_out;
  IdArrayHandleVTK conn_out;

  // grab the most recently generated field
  vtkm::Id fieldId = dataSet.GetNumberOfFields() - 1;
  vtkm::cont::Field rho = dataSet.GetField(fieldId);
  typedef vtkm::cont::cuda::ArrayHandleCuda<float>::type CudaFloatArrayHandle;
  CudaFloatArrayHandle solution =
    rho.GetData().CastToArrayHandle(CudaFloatArrayHandle::ValueType(),
                                    CudaFloatArrayHandle::StorageTag());
  vtkm::cont::ArrayHandle<float> solution_out;

  // vtkm::worklet::TransferData<VTKM_DEFAULT_DEVICE_ADAPTER_TAG>().run(
  //   shapes, nindices, conn, shapes_out, nindices_out, conn_out);

  vtkm::cont::DeviceAdapterAlgorithm<VTKM_DEFAULT_DEVICE_ADAPTER_TAG>().
    Copy(verts,verts_out);
  vtkm::cont::DeviceAdapterAlgorithm<VTKM_DEFAULT_DEVICE_ADAPTER_TAG>().
    Copy(shapes,shapes_out);
  vtkm::cont::DeviceAdapterAlgorithm<VTKM_DEFAULT_DEVICE_ADAPTER_TAG>().
    Copy(nindices,nindices_out);
  vtkm::cont::DeviceAdapterAlgorithm<VTKM_DEFAULT_DEVICE_ADAPTER_TAG>().
    Copy(conn,conn_out);
  vtkm::cont::DeviceAdapterAlgorithm<VTKM_DEFAULT_DEVICE_ADAPTER_TAG>().
    Copy(solution,solution_out);

  vtkm::cont::DataSet outDataSet;
  for(size_t i=0; i < (size_t)dataSet.GetNumberOfCoordinateSystems(); i++)
    {
    outDataSet.AddCoordinateSystem(dataSet.GetCoordinateSystem(i));
    }

  // std::cout << shapes.GetNumberOfValues() << " input elements, "
  //           << shapes_out.GetNumberOfValues() << " output elements.\n";
  //
  // vtkm::cont::ArrayHandle<float>::PortalConstControl solutionPortal =
  //   solution_out.GetPortalConstControl();
  //
  // std::cout<<"Solution: [ ";
  // for (size_t i=0;i<solution_out.GetNumberOfValues();i++)
  //   std::cout<<solutionPortal.Get(i)<<" ";
  // std::cout<<"]"<<std::endl;

  vtkSmartPointer<vtkFloatArray> pointData =
    vtkSmartPointer<vtkFloatArray>::New();
  vtkIdType nVerts = verts_out.GetNumberOfValues();
  std::cout<<"there are "<<nVerts<<" values"<<std::endl;
  pointData->SetArray(verts_out.Storage().StealArray(), nVerts,
                      0, // give VTK control of the data
                      0);// delete using "free"
  pointData->SetNumberOfComponents(3);
  vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
  points->SetData(pointData);

  // Create a polydata object and add the points to it.
  vtkSmartPointer<vtkPolyData> polydata = vtkSmartPointer<vtkPolyData>::New();
  polydata->SetPoints(points);
 
  // Write the file
  vtkSmartPointer<vtkXMLPolyDataWriter> writer =
    vtkSmartPointer<vtkXMLPolyDataWriter>::New();
  std::stringstream s;
  s << fileName.substr(0,fileName.find_last_of("."));
  s << "_" << dataDescription->GetTimeStep();
  s << fileName.substr(fileName.find_last_of("."), std::string::npos);
  writer->SetFileName(s.str().c_str());
#if VTK_MAJOR_VERSION <= 5
  writer->SetInput(polydata);
#else
  writer->SetInputData(polydata);
#endif
 
  // Optional - set the mode. The default is binary.
  //writer->SetDataModeToBinary();
  //writer->SetDataModeToAscii();
 
  // writer->Write();
 










  vtkSmartPointer<vtkUnstructuredGrid> grid =
    vtkSmartPointer<vtkUnstructuredGrid>::New();











  
  // vtkUnstructuredGrid* grid = vtkUnstructuredGrid::SafeDownCast(
  //   dataDescription->GetInputDescriptionByName("input")->GetGrid());
  // if(grid == NULL)
  //   {
  //   vtkWarningMacro("DataDescription is missing input unstructured grid.");
  //   return 0;
  //   }
  // if(this->RequestDataDescription(dataDescription) == 0)
  //   {
  //   return 1;
  //   }

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
