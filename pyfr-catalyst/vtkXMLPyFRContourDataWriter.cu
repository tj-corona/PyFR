#include <cmath>
#include <iomanip>
#include <string>
#include <sstream>

#include "vtkXMLPyFRContourDataWriter.h"

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

#include "ArrayHandleExposed.h"
#include "PyFRData.h"

vtkStandardNewMacro(vtkXMLPyFRContourDataWriter);

//----------------------------------------------------------------------------
vtkXMLPyFRContourDataWriter::vtkXMLPyFRContourDataWriter() : IsBinary(true),
                                       FileName("output.vtu")
{
}

//----------------------------------------------------------------------------
vtkXMLPyFRContourDataWriter::~vtkXMLPyFRContourDataWriter()
{
}

//----------------------------------------------------------------------------
void vtkXMLPyFRContourDataWriter::SetInputData(vtkDataObject* input)
{
  this->SetInputData(0, input);
}

//----------------------------------------------------------------------------
void vtkXMLPyFRContourDataWriter::SetInputData(int index, vtkDataObject* input)
{
  this->SetInputDataInternal(index, input);
}

//----------------------------------------------------------------------------
int vtkXMLPyFRContourDataWriter::Write()
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
void vtkXMLPyFRContourDataWriter::WriteData()
{
  const PyFRContourData* pyfrContourData =
    PyFRContourData::SafeDownCast(this->GetExecutive()->GetInputData(0, 0));
  if(!pyfrContourData)
    throw std::runtime_error("PyFRContourData input required.");

  PyFRContourData* contourData = const_cast<PyFRContourData*>(pyfrContourData);

  PyFRContourData::Double3ArrayHandle& verts_out = contourData->Vertices;

  vtkSmartPointer<vtkDoubleArray> pointData =
    vtkSmartPointer<vtkDoubleArray>::New();

  vtkIdType nVerts = verts_out.GetNumberOfValues();
  double* vertsArray = reinterpret_cast<double*>(verts_out.Storage().StealArray());
  pointData->SetArray(vertsArray, nVerts*3,
                      0, // give VTK control of the data
                      0);// delete using "free"
  pointData->SetNumberOfComponents(3);

  vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
  points->SetData(pointData);

  PyFRContourData::Double3ArrayHandle& normals_out = contourData->Normals;

  vtkSmartPointer<vtkDoubleArray> normalsData =
    vtkSmartPointer<vtkDoubleArray>::New();

  vtkIdType nNormals = normals_out.GetNumberOfValues();
  double* normalsArray = reinterpret_cast<double*>(normals_out.Storage().StealArray());
  normalsData->SetArray(normalsArray, nNormals*3,
                        0, // give VTK control of the data
                        0);// delete using "free"
  normalsData->SetNumberOfComponents(3);

  PyFRContourData::DoubleArrayHandle& scalars_out = contourData->Density;

  vtkSmartPointer<vtkDoubleArray> solutionData =
    vtkSmartPointer<vtkDoubleArray>::New();
  vtkIdType nSolution = scalars_out.GetNumberOfValues();
  double* solutionArray = scalars_out.Storage().StealArray();
  solutionData->SetArray(solutionArray, nSolution,
                         0, // give VTK control of the data
                         0);// delete using "free"
  solutionData->SetNumberOfComponents(1);
  solutionData->SetName("output");

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
  writer->SetFileName(FileName.c_str());
  writer->SetInputData(polydata);

  if (this->IsBinary)
    writer->SetDataModeToBinary();
  else
    writer->SetDataModeToAscii();

  writer->Write();

}

//----------------------------------------------------------------------------
int vtkXMLPyFRContourDataWriter::RequestData(
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

void vtkXMLPyFRContourDataWriter::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "FileName: " << this->FileName << "\n";
}
