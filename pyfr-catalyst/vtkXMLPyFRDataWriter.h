#ifndef VTKXMLPYFRDATAWRITER_H
#define VTKXMLPYFRDATAWRITER_H

#include <string>

#include "vtkPyFRDataAlgorithm.h"
#include "PyFRData.h"

//State that the default backend for this code is CUDA
//not serial
#define VTKM_DEVICE_ADAPTER VTKM_DEVICE_ADAPTER_CUDA
//Disable treading support in our array handle
//needed for nvcc to stop complaining.
#define BOOST_SP_DISABLE_THREADS

class vtkXMLPyFRDataWriter : public vtkPyFRDataAlgorithm
{
public:
  vtkTypeMacro(vtkXMLPyFRDataWriter,vtkPyFRDataAlgorithm)
  static vtkXMLPyFRDataWriter* New();
  virtual void PrintSelf(ostream& os, vtkIndent indent);

  void SetFileName(std::string fileName) { FileName = fileName; }
  std::string GetFileName() const { return FileName; }
  void SetInputData(vtkDataObject *);
  void SetInputData(int, vtkDataObject*);

  void SetDataModeToAscii() { IsBinary = false; }
  void SetDataModeToBinary() { IsBinary = true; }

  int RequestData(
    vtkInformation *,
    vtkInformationVector **,
    vtkInformationVector *);

  int Write();

protected:
  vtkXMLPyFRDataWriter();
  virtual ~vtkXMLPyFRDataWriter();

  void WriteData();

private:
  vtkXMLPyFRDataWriter(const vtkXMLPyFRDataWriter&); // Not implemented
  void operator=(const vtkXMLPyFRDataWriter&); // Not implemented

  bool IsBinary;
  std::string FileName;
};
#endif
