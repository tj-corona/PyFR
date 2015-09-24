#include "vtkCPVTKmPipeline.h"

#include <cmath>
#include <iomanip>
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

  std::string isosurfaceField = "density"; // density, velocity_u/v/w, pressure
  double isosurfaceValue = 1.0045;

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

  vtkmc::Field scalars = dataSet.GetField(isosurfaceField);
  CudaDoubleArrayHandle scalarsArray =
    scalars.GetData().CastToArrayHandle(CudaDoubleArrayHandle::ValueType(),
                                        CudaDoubleArrayHandle::StorageTag());

  // vtkm::worklet::IsosurfaceFilterUniformGrid<double, VTKM_DEFAULT_DEVICE_ADAPTER_TAG>* isosurfaceFilter = new vtkm::worklet::IsosurfaceFilterUniformGrid<double,VTKM_DEFAULT_DEVICE_ADAPTER_TAG>(dataSet);

  // isosurfaceFilter->Run(isosurfaceValue,
  //                       scalarsArray,
  //                       verts_out,
  //                       normals_out,
  //                       scalars_out);

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

  vtkSmartPointer<vtkDoubleArray> normalsData =
    vtkSmartPointer<vtkDoubleArray>::New();

  vtkIdType nNormals = normals_out.GetNumberOfValues();
  double* normalsArray = reinterpret_cast<double*>(normals_out.Storage().StealArray());
  normalsData->SetArray(normalsArray, nVerts,
                        0, // give VTK control of the data
                        0);// delete using "free"
  normalsData->SetNumberOfComponents(3);

  vtkSmartPointer<vtkDoubleArray> solutionData =
    vtkSmartPointer<vtkDoubleArray>::New();
  vtkIdType nSolution = scalars_out.GetNumberOfValues();
  double* solutionArray = scalars_out.Storage().StealArray();
  solutionData->SetArray(solutionArray, nSolution,
                         0, // give VTK control of the data
                         0);// delete using "free"
  solutionData->SetNumberOfComponents(1);
  solutionData->SetName(isosurfaceField.c_str());

  vtkSmartPointer<vtkCellArray> polys =
        vtkSmartPointer<vtkCellArray>::New();
  vtkIdType indices[3];
  for (vtkIdType i=0;i<points->GetNumberOfPoints();i+=3)
    {
    for (vtkIdType j=0;j<3;j++)
      indices[j] = i+j;
    polys->InsertNextCell(3,indices);
    }

  // Create a polydata object and add the points to it.
  vtkSmartPointer<vtkPolyData> polydata = vtkSmartPointer<vtkPolyData>::New();
  polydata->SetPoints(points);
  polydata->SetPolys(polys);
  polydata->GetPointData()->SetNormals(normalsData);
  polydata->GetPointData()->AddArray(solutionData);

  // Write the file
  vtkSmartPointer<vtkXMLPolyDataWriter> writer =
    vtkSmartPointer<vtkXMLPolyDataWriter>::New();
  std::stringstream s;
  s << fileName.substr(0,fileName.find_last_of("."));
  s << "_" << std::fixed << std::setprecision(3) << dataDescription->GetTime();
  s << fileName.substr(fileName.find_last_of("."), std::string::npos);
  writer->SetFileName(s.str().c_str());
#if VTK_MAJOR_VERSION <= 5
  writer->SetInput(polydata);
#else
  writer->SetInputData(polydata);
#endif

  // Optional - set the mode. The default is binary.
  //writer->SetDataModeToBinary();
  writer->SetDataModeToAscii();

  writer->Write();

  return 1;
}

//----------------------------------------------------------------------------
void vtkCPVTKmPipeline::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "fileName: " << this->fileName << "\n";
}
