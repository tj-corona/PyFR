#ifndef VTKPYFRPIPELINE_H
#define VTKPYFRPIPELINE_H

#include <vtkCPPipeline.h>
#include <string>

class vtkCPDataDescription;

class vtkPyFRPipeline : public vtkCPPipeline
{
public:
  static vtkPyFRPipeline* New();
  vtkTypeMacro(vtkPyFRPipeline,vtkCPPipeline)
  virtual void PrintSelf(ostream& os, vtkIndent indent);

  virtual void Initialize(char* fileName);

  virtual int RequestDataDescription(vtkCPDataDescription* dataDescription);

  virtual int CoProcess(vtkCPDataDescription* dataDescription);

protected:
  vtkPyFRPipeline();
  virtual ~vtkPyFRPipeline();

private:
  vtkPyFRPipeline(const vtkPyFRPipeline&); // Not implemented
  void operator=(const vtkPyFRPipeline&); // Not implemented

  std::string fileName;
};
#endif
