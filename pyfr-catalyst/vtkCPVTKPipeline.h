#ifndef VTKCPVTKPIPELINE_H
#define VTKCPVTKPIPELINE_H

#include <vtkCPPipeline.h>
#include <string>

class vtkCPDataDescription;
class vtkCPPythonHelper;

class vtkCPVTKPipeline : public vtkCPPipeline
{
public:
  static vtkCPVTKPipeline* New();
  vtkTypeMacro(vtkCPVTKPipeline,vtkCPPipeline);
  virtual void PrintSelf(ostream& os, vtkIndent indent);

  virtual void Initialize(char* fileName);

  virtual int RequestDataDescription(vtkCPDataDescription* dataDescription);

  virtual int CoProcess(vtkCPDataDescription* dataDescription);

protected:
  vtkCPVTKPipeline();
  virtual ~vtkCPVTKPipeline();

private:
  vtkCPVTKPipeline(const vtkCPVTKPipeline&); // Not implemented
  void operator=(const vtkCPVTKPipeline&); // Not implemented

  std::string FileName;
};
#endif
