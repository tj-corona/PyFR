#include <cmath>
#include <iomanip>
#include <string>
#include <sstream>

#include "PyFRDataWriter.h"

#include <vtkPolyData.h>
#include <vtkSmartPointer.h>
#include <vtkUnstructuredGrid.h>
#include <vtkXMLPolyDataWriter.h>
#include <vtkXMLUnstructuredGridWriter.h>

#include "PyFRDataConverter.h"
#include "PyFRData.h"
#include "PyFRContourData.h"

//----------------------------------------------------------------------------
PyFRDataWriter::PyFRDataWriter() : IsBinary(true),
                                   FileName("output")
{
}

//----------------------------------------------------------------------------
PyFRDataWriter::~PyFRDataWriter()
{
}

//----------------------------------------------------------------------------
void PyFRDataWriter::operator ()(PyFRData* pyfrData) const
{
  PyFRDataConverter convert;
  vtkUnstructuredGrid* grid = vtkUnstructuredGrid::New();
  convert(pyfrData,grid);

  // Write the file
  vtkSmartPointer<vtkXMLUnstructuredGridWriter> writer =
    vtkSmartPointer<vtkXMLUnstructuredGridWriter>::New();
  std::stringstream s; s << FileName << ".vtu";
  writer->SetFileName(s.str().c_str());
  writer->SetInputData(grid);

  if (this->IsBinary)
    writer->SetDataModeToBinary();
  else
    writer->SetDataModeToAscii();

  writer->Write();
  grid->Delete();
}

//----------------------------------------------------------------------------
void PyFRDataWriter::operator ()(PyFRContourData* pyfrContourData) const
{
  PyFRDataConverter convert;
  vtkPolyData* polydata = vtkPolyData::New();
  convert(pyfrContourData,polydata);

  // Write the file
  vtkSmartPointer<vtkXMLPolyDataWriter> writer =
    vtkSmartPointer<vtkXMLPolyDataWriter>::New();
  std::stringstream s; s << FileName << ".vtp";
  writer->SetFileName(s.str().c_str());
  writer->SetInputData(polydata);

  if (this->IsBinary)
    writer->SetDataModeToBinary();
  else
    writer->SetDataModeToAscii();

  writer->Write();
  polydata->Delete();
}