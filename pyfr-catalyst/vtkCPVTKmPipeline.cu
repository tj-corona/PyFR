#include "vtkCPVTKmPipeline.h"

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

  // currently, we have a cell set for each cell type
  for (size_t cellType=0;cellType<dataSet.GetNumberOfCellSets();cellType++)
    {
      std::stringstream name; name << "xyz_" << cellType;
      vtkm::cont::Field xyz = dataSet.GetField(name.str());
      typedef vtkm::cont::ArrayHandle<double> DoubleArrayHandle;
      DoubleArrayHandle verts =
        xyz.GetData().CastToArrayHandle(DoubleArrayHandle::ValueType(),
                                        DoubleArrayHandle::StorageTag());

      name.clear();
      name.str("");
      name << "cells_" << cellType;
      vtkm::cont::CellSetExplicit<>& cset =
        dataSet.GetCellSet(name.str()).CastTo<vtkm::cont::CellSetExplicit<> >();
      typedef vtkm::cont::ArrayHandle<vtkm::Id> IdArrayHandle;
      IdArrayHandle shapes = cset.GetShapesArray(
        vtkm::TopologyElementTagPoint(),vtkm::TopologyElementTagCell());
      IdArrayHandle nindices = cset.GetNumIndicesArray(
        vtkm::TopologyElementTagPoint(),vtkm::TopologyElementTagCell());
      IdArrayHandle conn = cset.GetConnectivityArray(
        vtkm::TopologyElementTagPoint(),vtkm::TopologyElementTagCell());
      typedef vtkm::cont::ArrayHandleVTK<vtkm::Id> IdArrayHandleVTK;
      typedef vtkm::cont::ArrayHandleVTK<double> FloatArrayHandleVTK;
      FloatArrayHandleVTK verts_out;
      vtkm::cont::ArrayHandleVTK<vtkIdType> shapes_out;
      vtkm::cont::ArrayHandleCast<vtkm::Id,vtkm::cont::ArrayHandleVTK<vtkIdType> > shapes_out_cast(shapes_out);

      vtkm::cont::ArrayHandleVTK<vtkIdType> nindices_out;
      vtkm::cont::ArrayHandleCast<vtkm::Id,vtkm::cont::ArrayHandleVTK<vtkIdType> > nindices_out_cast(nindices_out);

      vtkm::cont::ArrayHandleVTK<vtkIdType> conn_out;
      vtkm::cont::ArrayHandleCast<vtkm::Id,vtkm::cont::ArrayHandleVTK<vtkIdType> > conn_out_cast(conn_out);

      name.clear();
      name.str("");
      name << "rho_" << cellType;
      vtkm::cont::Field rho = dataSet.GetField(name.str());
      typedef vtkm::cont::cuda::ArrayHandleCuda<double>::type CudaDoubleArrayHandle;
      CudaDoubleArrayHandle solution =
        rho.GetData().CastToArrayHandle(CudaDoubleArrayHandle::ValueType(),
                                        CudaDoubleArrayHandle::StorageTag());
      vtkm::cont::ArrayHandle<double> solution_out;

      // vtkm::worklet::TransferData<VTKM_DEFAULT_DEVICE_ADAPTER_TAG>().run(
      //   shapes, nindices, conn, shapes_out, nindices_out, conn_out);

      vtkm::cont::DeviceAdapterAlgorithm<VTKM_DEFAULT_DEVICE_ADAPTER_TAG>().
        Copy(verts,verts_out);
      vtkm::cont::DeviceAdapterAlgorithm<VTKM_DEFAULT_DEVICE_ADAPTER_TAG>().
        Copy(shapes,shapes_out_cast);
      vtkm::cont::DeviceAdapterAlgorithm<VTKM_DEFAULT_DEVICE_ADAPTER_TAG>().
        Copy(nindices,nindices_out_cast);
      vtkm::cont::DeviceAdapterAlgorithm<VTKM_DEFAULT_DEVICE_ADAPTER_TAG>().
        Copy(conn,conn_out_cast);
      vtkm::cont::DeviceAdapterAlgorithm<VTKM_DEFAULT_DEVICE_ADAPTER_TAG>().
        Copy(solution,solution_out);

      vtkm::cont::DataSet outDataSet;
      for(size_t i=0; i < (size_t)dataSet.GetNumberOfCoordinateSystems(); i++)
        {
        outDataSet.AddCoordinateSystem(dataSet.GetCoordinateSystem(i));
        }


  // std::cout << shapes.GetNumberOfValues() << " input elements, "
  //           << shapes_out.GetNumberOfValues() << " output elements.\n";

      {
      vtkm::cont::ArrayHandle<double>::PortalConstControl vertsPortal =
        verts_out.GetPortalConstControl();

      std::cout<<"# of vertex values: "<<verts_out.GetNumberOfValues()<<std::endl;
      std::cout<<"Verts: [ ";
      for (size_t i=0;i<verts_out.GetNumberOfValues();i++)
        std::cout<<vertsPortal.Get(i)<<" ";
      std::cout<<"]"<<std::endl;
      std::cout<<""<<std::endl;
      }

      {
      vtkm::cont::ArrayHandle<vtkIdType>::PortalConstControl shapesPortal =
        shapes_out.GetPortalConstControl();

      std::cout<<"# of shapes values: "<<shapes_out.GetNumberOfValues()<<std::endl;
      std::cout<<"shapes: [ ";
      for (size_t i=0;i<shapes_out.GetNumberOfValues();i++)
        std::cout<<shapesPortal.Get(i)<<" ";
      std::cout<<"]"<<std::endl;
      std::cout<<""<<std::endl;
      }

      {
      vtkm::cont::ArrayHandle<vtkIdType>::PortalConstControl nindicesPortal =
        nindices_out.GetPortalConstControl();

      std::cout<<"# of nindices values: "<<nindices_out.GetNumberOfValues()<<std::endl;
      std::cout<<"nindices: [ ";
      for (size_t i=0;i<nindices_out.GetNumberOfValues();i++)
        std::cout<<nindicesPortal.Get(i)<<" ";
      std::cout<<"]"<<std::endl;
      std::cout<<""<<std::endl;
      }

      {
      vtkm::cont::ArrayHandle<vtkIdType>::PortalConstControl connPortal =
        conn_out.GetPortalConstControl();

      std::cout<<"# of conn values: "<<conn_out.GetNumberOfValues()<<std::endl;
      std::cout<<"conn: [ ";
      for (size_t i=0;i<conn_out.GetNumberOfValues();i++)
        std::cout<<connPortal.Get(i)<<" ";
      std::cout<<"]"<<std::endl;
      std::cout<<""<<std::endl;
      }

      {
      vtkm::cont::ArrayHandle<double>::PortalConstControl solutionPortal =
        solution_out.GetPortalConstControl();

      std::cout<<"# of solution values: "<<solution_out.GetNumberOfValues()<<std::endl;
      std::cout<<"Solution: [ ";
      for (size_t i=0;i<solution_out.GetNumberOfValues();i++)
        std::cout<<solutionPortal.Get(i)<<" ";
      std::cout<<"]"<<std::endl;
      std::cout<<""<<std::endl;
      }

      vtkSmartPointer<vtkDoubleArray> pointData =
        vtkSmartPointer<vtkDoubleArray>::New();

      vtkIdType nVerts = verts_out.GetNumberOfValues();
      double* vertsArray = verts_out.Storage().StealArray();
      pointData->SetArray(vertsArray, nVerts,
                          0, // give VTK control of the data
                          0);// delete using "free"
      pointData->SetNumberOfComponents(3);

      vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
      points->SetData(pointData);

      vtkSmartPointer<vtkIdTypeArray> cellData =
        vtkSmartPointer<vtkIdTypeArray>::New();
      vtkIdType nConn = conn_out.GetNumberOfValues();
      vtkIdType* connArray = conn_out.Storage().StealArray();
      cellData->SetArray(connArray,nConn,
                         0, // give VTK control of the data
                         0);

      vtkSmartPointer<vtkCellArray> polys =
        vtkSmartPointer<vtkCellArray>::New();
      polys->SetCells(shapes_out.GetNumberOfValues(),cellData);

      // Create a polydata object and add the points to it.
      vtkSmartPointer<vtkPolyData> polydata = vtkSmartPointer<vtkPolyData>::New();
      polydata->SetPoints(points);
      // polydata->SetPolys(polys);

      // Write the file
      vtkSmartPointer<vtkXMLPolyDataWriter> writer =
        vtkSmartPointer<vtkXMLPolyDataWriter>::New();
      std::stringstream s;
      s << fileName.substr(0,fileName.find_last_of("."));
      s << "_" << cellType << "_" << dataDescription->GetTimeStep();
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

      // vtkSmartPointer<vtkUnstructuredGrid> grid =
      //   vtkSmartPointer<vtkUnstructuredGrid>::New();
    }











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
