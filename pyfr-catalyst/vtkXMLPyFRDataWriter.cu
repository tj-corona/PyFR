#include <cmath>
#include <iomanip>
#include <string>
#include <sstream>

#include "vtkXMLPyFRDataWriter.h"

#include <vtkCellArray.h>
#include <vtkCellType.h>
#include <vtkCommand.h>
#include <vtkCommunicator.h>
#include <vtkCompleteArrays.h>
#include <vtkCPDataDescription.h>
#include <vtkCPInputDataDescription.h>
#include <vtkDataArray.h>
#include <vtkDoubleArray.h>
#include "vtkErrorCode.h"
#include "vtkExecutive.h"
#include <vtkFloatArray.h>
#include <vtkHexahedron.h>
#include <vtkIdTypeArray.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkPointData.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkSmartPointer.h>
#include <vtkUnstructuredGrid.h>
#include <vtkXMLPolyDataWriter.h>
#include <vtkXMLUnstructuredGridWriter.h>

#include <vtkm/cont/ArrayHandleCast.h>
#include <vtkm/cont/DeviceAdapter.h>
#include <vtkm/cont/DeviceAdapterAlgorithm.h>
#include <vtkm/cont/DeviceAdapterSerial.h>
#include <vtkm/cont/DynamicArrayHandle.h>
#include <vtkm/cont/cuda/ArrayHandleCuda.h>
#include <vtkm/cont/cuda/internal/DeviceAdapterTagCuda.h>

#include "ArrayChoice.h"
#include "ArrayHandleExposed.h"
#include "PyFRData.h"

vtkStandardNewMacro(vtkXMLPyFRDataWriter);

//----------------------------------------------------------------------------
vtkXMLPyFRDataWriter::vtkXMLPyFRDataWriter() : IsBinary(true),
                                       FileName("output.vtu")
{
}

//----------------------------------------------------------------------------
vtkXMLPyFRDataWriter::~vtkXMLPyFRDataWriter()
{
}

//----------------------------------------------------------------------------
void vtkXMLPyFRDataWriter::SetInputData(vtkDataObject* input)
{
  this->SetInputData(0, input);
}

//----------------------------------------------------------------------------
void vtkXMLPyFRDataWriter::SetInputData(int index, vtkDataObject* input)
{
  this->SetInputDataInternal(index, input);
}

//----------------------------------------------------------------------------
int vtkXMLPyFRDataWriter::Write()
{
  // Make sure we have input.
  if (this->GetNumberOfInputConnections(0) < 1)
    {
    vtkErrorMacro("No input provided!");
    return 0;
    }

  // always write even if the data hasn't changed
  this->Modified();
  this->UpdateWholeExtent();

  return (this->GetErrorCode() == vtkErrorCode::NoError);
}

//----------------------------------------------------------------------------
void vtkXMLPyFRDataWriter::WriteData()
{
  const PyFRData* pyfrData = PyFRData::SafeDownCast(this->GetExecutive()->GetInputData(0, 0));
  if(!pyfrData)
    throw std::runtime_error("PyFRData input required.");

  const vtkm::cont::DataSet& dataSet = pyfrData->GetDataSet();

  namespace vtkmc = vtkm::cont;
  typedef vtkmc::ArrayHandleExposed<vtkIdType> IdArrayHandleExposed;
  typedef vtkmc::ArrayHandleExposed<FPType> ScalarDataArrayHandleExposed;
  typedef vtkmc::ArrayHandleExposed<vtkm::Vec<FPType,3> > Vec3ArrayHandleExposed;

  typedef vtkmc::ArrayHandle<vtkm::Vec<FPType,3> > Vec3ArrayHandle;
  Vec3ArrayHandleExposed vertices;
    {
    Vec3ArrayHandle tmp = dataSet.GetCoordinateSystem().GetData()
      .CastToArrayHandle(Vec3ArrayHandle::ValueType(),
                         Vec3ArrayHandle::StorageTag());
    vtkm::cont::DeviceAdapterAlgorithm<VTKM_DEFAULT_DEVICE_ADAPTER_TAG>().
      Copy(tmp,vertices);
    }

  vtkSmartPointer<ArrayChoice<FPType>::type> pointData =
    vtkSmartPointer<ArrayChoice<FPType>::type>::New();

  vtkIdType nVerts = vertices.GetNumberOfValues();
  FPType* vertsArray = reinterpret_cast<FPType*>(vertices.Storage().StealArray());
  pointData->SetArray(vertsArray, nVerts*3,
                      0, // give VTK control of the data
                      0);// delete using "free"
  pointData->SetNumberOfComponents(3);

  vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
  points->SetData(pointData);

  std::string fieldName[5] = {"density","velocity_u","velocity_v","velocity_w",
"pressure"};
  vtkSmartPointer<ArrayChoice<FPType>::type> solutionData[5];
  for (unsigned i=0;i<5;i++)
    {
    vtkmc::Field solution = dataSet.GetField(fieldName[i]);
    PyFRData::ScalarDataArrayHandle solutionArray = solution.GetData()
      .CastToArrayHandle(PyFRData::ScalarDataArrayHandle::ValueType(),
                         PyFRData::ScalarDataArrayHandle::StorageTag());
    ScalarDataArrayHandleExposed solutionArrayHost;
    vtkm::cont::DeviceAdapterAlgorithm<VTKM_DEFAULT_DEVICE_ADAPTER_TAG>().
      Copy(solutionArray, solutionArrayHost);

    solutionData[i] = vtkSmartPointer<ArrayChoice<FPType>::type>::New();
    vtkIdType nSolution = solutionArrayHost.GetNumberOfValues();
    FPType* solutionArr = solutionArrayHost.Storage().StealArray();
    solutionData[i]->SetArray(solutionArr, nSolution,
                           0, // give VTK control of the data
                           0);// delete using "free"
    solutionData[i]->SetNumberOfComponents(1);
    solutionData[i]->SetName(fieldName[i].c_str());
    }

  vtkm::cont::CellSetExplicit<> cellSet = dataSet.GetCellSet(0)
      .template CastTo<vtkm::cont::CellSetExplicit<> >();

    vtkm::cont::ArrayHandle<vtkm::Id> connectivity =
      cellSet.GetConnectivityArray(vtkm::TopologyElementTagPoint(),
                                   vtkm::TopologyElementTagCell());
    vtkm::cont::ArrayHandle<vtkm::Id>::PortalConstControl portal =
      connectivity.GetPortalConstControl();

    vtkSmartPointer<vtkUnstructuredGrid> grid =
        vtkSmartPointer<vtkUnstructuredGrid>::New();
  grid->Allocate(connectivity.GetNumberOfValues()/8);
  grid->SetPoints(points);
  for (unsigned i=0;i<5;i++)
    grid->GetPointData()->AddArray(solutionData[i]);
  vtkIdType indices[8];
  vtkIdType counter = 0;
  while (counter < connectivity.GetNumberOfValues())
    {
    vtkSmartPointer<vtkHexahedron> hex = vtkSmartPointer<vtkHexahedron>::New();
    for (vtkIdType j=0;j<8;j++)
      hex->GetPointIds()->SetId(j,portal.Get(counter++));
    grid->InsertNextCell(hex->GetCellType(),hex->GetPointIds());
    }

  // Write the file
  vtkSmartPointer<vtkXMLUnstructuredGridWriter> writer =
    vtkSmartPointer<vtkXMLUnstructuredGridWriter>::New();
  writer->SetFileName(FileName.c_str());
  writer->SetInputData(grid);

  if (this->IsBinary)
    writer->SetDataModeToBinary();
  else
    writer->SetDataModeToAscii();

  writer->Write();
}

//----------------------------------------------------------------------------
int vtkXMLPyFRDataWriter::RequestData(
  vtkInformation *,
  vtkInformationVector **,
  vtkInformationVector *)
{
  this->SetErrorCode(vtkErrorCode::NoError);

  vtkDataObject *input = this->GetInput();
  int idx;

  // make sure input is available
  if ( !input )
    {
    vtkErrorMacro(<< "No input!");
    return 0;
    }

  for (idx = 0; idx < this->GetNumberOfInputPorts(); ++idx)
    {
    if (this->GetInputExecutive(idx, 0) != NULL)
      {
      this->GetInputExecutive(idx, 0)->Update();
      }
    }

  this->InvokeEvent(vtkCommand::StartEvent,NULL);
  this->WriteData();
  this->InvokeEvent(vtkCommand::EndEvent,NULL);

  return 1;
}
//----------------------------------------------------------------------------

void vtkXMLPyFRDataWriter::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "FileName: " << this->FileName << "\n";
}
