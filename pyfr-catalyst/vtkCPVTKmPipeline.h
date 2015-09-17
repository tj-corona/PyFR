#ifndef VTKCPVTKMPIPELINE_H
#define VTKCPVTKMPIPELINE_H

#include <vtkCPPipeline.h>
#include <string>

class vtkCPDataDescription;

class vtkCPVTKmPipeline : public vtkCPPipeline
{
public:
  static vtkCPVTKmPipeline* New();
  vtkTypeMacro(vtkCPVTKmPipeline,vtkCPPipeline)
  virtual void PrintSelf(ostream& os, vtkIndent indent);

  virtual void Initialize(char* fileName);

  virtual int RequestDataDescription(vtkCPDataDescription* dataDescription);

  virtual int CoProcess(vtkCPDataDescription* dataDescription);

protected:
  vtkCPVTKmPipeline();
  virtual ~vtkCPVTKmPipeline();

private:
  vtkCPVTKmPipeline(const vtkCPVTKmPipeline&); // Not implemented
  void operator=(const vtkCPVTKmPipeline&); // Not implemented

  std::string fileName;
};
#endif
