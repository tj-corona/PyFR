#include "vtkCPVTKmPipeline.h"

#include <cmath>
#include <string>
#include <sstream>

#include "PyFRData.h"

#include <vtkCellArray.h>
#include <vtkCommunicator.h>
#include <vtkCompleteArrays.h>
#include <vtkCPDataDescription.h>
#include <vtkCPInputDataDescription.h>
#include <vtkDataArray.h>
#include <vtkDoubleArray.h>
#include <vtkFloatArray.h>
#include <vtkIdTypeArray.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkPointData.h>
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

#include <vtkm/worklet/IsosurfaceUniformGrid.h>

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

  namespace vtkmc = vtkm::cont;
  typedef vtkmc::ArrayHandleVTK<vtkIdType> IdArrayHandleVTK;
  typedef vtkmc::ArrayHandleVTK<double> DoubleArrayHandleVTK;
  typedef vtkmc::ArrayHandleVTK<vtkm::Vec<double,3> > Double3ArrayHandleVTK;
  typedef vtkmc::ArrayHandleCast<vtkm::Id,IdArrayHandleVTK > IdArrayHandleCast;
  typedef vtkmc::cuda::ArrayHandleCuda<double>::type CudaDoubleArrayHandle;

  Double3ArrayHandleVTK verts_out;
  Double3ArrayHandleVTK normals_out;
  DoubleArrayHandleVTK scalars_out;

  vtkmc::Field rho = dataSet.GetField("rho");
  CudaDoubleArrayHandle solution =
    rho.GetData().CastToArrayHandle(CudaDoubleArrayHandle::ValueType(),
                                    CudaDoubleArrayHandle::StorageTag());

  double isosurfaceValue = 1.;

  //   {
  //   DoubleArrayHandleVTK tmp;
  //   vtkm::cont::DeviceAdapterAlgorithm<VTKM_DEFAULT_DEVICE_ADAPTER_TAG>().
  //     Copy(solution,tmp);

  //   double mean = 0.;
  //   for (size_t i=0;i<tmp.GetNumberOfValues();i++)
  //     {
  //     std::cout<<i<<": "<<tmp.GetPortalConstControl().Get(i)<<std::endl;
  // mean += tmp.GetPortalConstControl().Get(i);
  //     }
  //   mean/=tmp.GetNumberOfValues();
  //   isosurfaceValue = mean;
  //   }

  // std::cout<<"Isosurface contour value = "<<isosurfaceValue<<std::endl;


  vtkm::Id3 dims;
  for (int i=0;i<3;i++)
    dims[i] = pyfrData->GetCellDimension()[i];
  vtkm::worklet::IsosurfaceFilterUniformGrid<double, VTKM_DEFAULT_DEVICE_ADAPTER_TAG>* isosurfaceFilter = new vtkm::worklet::IsosurfaceFilterUniformGrid<double,VTKM_DEFAULT_DEVICE_ADAPTER_TAG>(dataSet);

  isosurfaceFilter->Run(isosurfaceValue,
                        solution,
                        verts_out,
                        normals_out,
                        scalars_out);

  bool printData = false;

  if (printData)
    {
    Double3ArrayHandleVTK::PortalConstControl vertsPortal =
      verts_out.GetPortalConstControl();

    std::cout<<"# of vertex values: "<<verts_out.GetNumberOfValues()<<std::endl;
    std::cout<<"Verts: [ ";
    for (size_t i=0;i<verts_out.GetNumberOfValues();i++)
      std::cout<<vertsPortal.Get(i)<<" ";
    std::cout<<"]"<<std::endl;
    std::cout<<""<<std::endl;
    }

  if (printData)
    {
    Double3ArrayHandleVTK::PortalConstControl normalsPortal =
      normals_out.GetPortalConstControl();

    std::cout<<"# of vertex values: "<<normals_out.GetNumberOfValues()<<std::endl;
    std::cout<<"Normals: [ ";
    for (size_t i=0;i<normals_out.GetNumberOfValues();i++)
      std::cout<<normalsPortal.Get(i)<<" ";
    std::cout<<"]"<<std::endl;
    std::cout<<""<<std::endl;
    }

  if (printData)
    {
    DoubleArrayHandleVTK::PortalConstControl scalarsPortal =
      scalars_out.GetPortalConstControl();

    std::cout<<"# of scalars values: "<<scalars_out.GetNumberOfValues()<<std::endl;
    std::cout<<"Scalars: [ ";
    for (size_t i=0;i<scalars_out.GetNumberOfValues();i++)
      std::cout<<scalarsPortal.Get(i)<<" ";
    std::cout<<"]"<<std::endl;
    std::cout<<""<<std::endl;
    }

  vtkSmartPointer<vtkDoubleArray> pointData =
    vtkSmartPointer<vtkDoubleArray>::New();

  vtkIdType nVerts = verts_out.GetNumberOfValues();
  double* vertsArray = reinterpret_cast<double*>(verts_out.Storage().StealArray());
  pointData->SetArray(vertsArray, nVerts,
                      0, // give VTK control of the data
                      0);// delete using "free"
  pointData->SetNumberOfComponents(3);

  vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
  points->SetData(pointData);

  vtkSmartPointer<vtkDoubleArray> rhoData =
    vtkSmartPointer<vtkDoubleArray>::New();
  vtkIdType nRho = scalars_out.GetNumberOfValues();
  double* rhoArray = scalars_out.Storage().StealArray();
  rhoData->SetArray(rhoArray, nRho,
                      0, // give VTK control of the data
                      0);// delete using "free"
  rhoData->SetNumberOfComponents(1);

  vtkSmartPointer<vtkCellArray> polys =
        vtkSmartPointer<vtkCellArray>::New();
      vtkIdType indices[3];
      for (vtkIdType i=0;i<nVerts;i+=3)
        {
        for (vtkIdType j=0;j<3;j++)
          indices[j] = i+j;
        polys->InsertNextCell(3,indices);
        }

      // Create a polydata object and add the points to it.
      vtkSmartPointer<vtkPolyData> polydata = vtkSmartPointer<vtkPolyData>::New();
      polydata->SetPoints(points);
      polydata->SetPolys(polys);
      polydata->GetPointData()->AddArray(rhoData);

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

  writer->Write();











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
