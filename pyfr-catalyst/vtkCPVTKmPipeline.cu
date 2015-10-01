#include "vtkCPVTKmPipeline.h"

#include <cmath>
#include <iomanip>
#include <string>
#include <sstream>

#include "PyFRData.h"
#include "PyFRContourData.h"

#include <vtkCellArray.h>
#include <vtkCellType.h>
#include <vtkCommunicator.h>
#include <vtkCompleteArrays.h>
#include <vtkCPDataDescription.h>
#include <vtkCPInputDataDescription.h>
#include <vtkDataArray.h>
#include <vtkDoubleArray.h>
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

#include "vtkPyFRDataContourFilter.h"
#include "vtkXMLPyFRDataWriter.h"
#include "vtkXMLPyFRContourDataWriter.h"

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

  {
  std::stringstream s;
  s << fileName.substr(0,fileName.find_last_of("."));
  s << "_" << std::fixed << std::setprecision(3) << dataDescription->GetTime();
  s << "_input.vtu";

  vtkSmartPointer<vtkXMLPyFRDataWriter> writer =
    vtkSmartPointer<vtkXMLPyFRDataWriter>::New();

  writer->SetFileName(s.str().c_str());
  writer->SetInputData(pyfrData);
  writer->SetDataModeToBinary();
  writer->Write();
  }

  vtkSmartPointer<vtkPyFRDataContourFilter> isocontour =
    vtkSmartPointer<vtkPyFRDataContourFilter>::New();

  isocontour->SetContourFieldToDensity();
  isocontour->SetContourValue(1.0045);
  isocontour->SetInputData(pyfrData);

  {
  std::stringstream s;
  s << fileName.substr(0,fileName.find_last_of("."));
  s << "_" << std::fixed << std::setprecision(3) << dataDescription->GetTime();
  s << fileName.substr(fileName.find_last_of("."), std::string::npos);

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
void vtkCPVTKmPipeline::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "fileName: " << this->fileName << "\n";
}
